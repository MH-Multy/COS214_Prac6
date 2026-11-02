// End-to-end scenarios required by the specification: for each industry, a
// completion path and exception paths, run through the same generic engine.
#include "doctest.h"

#include "TestSystem.h"
#include "WorkflowInstance.h"

// ------------------------------------------------------------------ laboratory testing
TEST_CASE("Lab: a test request runs from submission to completion") {
    TestSystem sys;
    int id = sys.labRequest();
    REQUIRE(sys.facade.getStatus(id) == "Accepted");
    REQUIRE(sys.facade.approveRequest(id, people::NAIDOO));

    CHECK(sys.facade.completeWorkItem(id, labTask::RESERVE, people::LERATO));
    CHECK(sys.facade.completeWorkItem(id, labTask::PREPARE, people::SARAH));
    CHECK(sys.facade.updateReport(id, people::SARAH, "Samples prepared"));
    CHECK(sys.facade.completeWorkItem(id, labTask::RUN, people::SARAH));
    CHECK(sys.facade.completeWorkItem(id, labTask::RECORD, people::SARAH));
    CHECK(sys.facade.completeWorkItem(id, labTask::REVIEW, people::THABO));

    CHECK(sys.facade.getStatus(id) == "Completed");
    CHECK(sys.facade.getWorkflow(id)->allWorkComplete());
    CHECK(sys.hasEvent(id, "CREATED"));
    CHECK(sys.hasEvent(id, "APPROVED"));
    CHECK(sys.hasEvent(id, "COMPLETED"));
}

TEST_CASE("Lab: steps in a sequential stage cannot be skipped") {
    TestSystem sys;
    int id = sys.approvedLab();
    CHECK_FALSE(sys.facade.completeWorkItem(id, labTask::RUN, people::SARAH));      // preparation not done
    sys.facade.completeWorkItem(id, labTask::RESERVE, people::LERATO);
    sys.facade.completeWorkItem(id, labTask::PREPARE, people::SARAH);
    CHECK_FALSE(sys.facade.completeWorkItem(id, labTask::RECORD, people::SARAH));   // must run the test first
}

TEST_CASE("Lab exception: an intern who is not cleared is rejected") {
    TestSystem sys;
    CHECK(sys.facade.getStatus(sys.labRequest(people::THABO)) == "Rejected");
}

TEST_CASE("Lab exception: a request over the risk limit is rejected") {
    TestSystem sys;
    CHECK(sys.facade.getStatus(sys.labRequest(people::SARAH, "4")) == "Rejected");
    CHECK(sys.facade.getStatus(sys.labRequest(people::SARAH, "3")) == "Accepted");   // the limit itself is allowed
}

TEST_CASE("Lab exception: a request with missing details is rejected") {
    TestSystem sys;
    WorkflowRequest r("Incomplete", people::SARAH);
    r.setAttribute("testType", "Blood panel");
    CHECK(sys.facade.getStatus(sys.facade.submitRequest("Lab Test Request", r)) == "Rejected");
}

TEST_CASE("Lab exception: the head researcher denies the request") {
    TestSystem sys;
    int id = sys.labRequest();
    CHECK(sys.facade.rejectRequest(id, people::NAIDOO, "Equipment booked for maintenance"));
    CHECK(sys.facade.getStatus(id) == "Denied");
    CHECK_FALSE(sys.facade.approveRequest(id, people::NAIDOO));
}

TEST_CASE("Lab exception: a suspended test cannot continue until resumed") {
    TestSystem sys;
    int id = sys.approvedLab();
    REQUIRE(sys.facade.suspendRequest(id, people::NAIDOO));
    CHECK_FALSE(sys.facade.completeWorkItem(id, labTask::RESERVE, people::LERATO));
    CHECK_FALSE(sys.facade.updateReport(id, people::SARAH, "log"));
    REQUIRE(sys.facade.resumeRequest(id, people::NAIDOO));
    CHECK(sys.facade.completeWorkItem(id, labTask::RESERVE, people::LERATO));
}

TEST_CASE("Lab: an unanswered request is approved automatically after the timeout") {
    TestSystem sys(0);
    int id = sys.labRequest();
    CHECK(sys.facade.getStatus(id) == "Approved");
    CHECK_FALSE(sys.facade.getAvailableWork(id).empty());
}

// ------------------------------------------------------------------ loan applications
TEST_CASE("Loan: an application runs from submission to payout and sign-off") {
    TestSystem sys;
    int id = sys.loanRequest();
    REQUIRE(sys.facade.approveRequest(id, people::PIETER));
    CHECK(sys.facade.completeWorkItem(id, loanTask::CREDIT, people::AYESHA));
    CHECK(sys.facade.completeWorkItem(id, loanTask::AFFORD, people::AYESHA));
    CHECK(sys.facade.updateReport(id, people::AYESHA, "Affordability ratio 0.31: acceptable"));
    CHECK(sys.facade.completeWorkItem(id, loanTask::OFFER, people::AYESHA));
    CHECK(sys.facade.completeWorkItem(id, loanTask::ACCEPT, people::NOMVULA));
    CHECK(sys.facade.completeWorkItem(id, loanTask::DISBURSE, people::PIETER));
    CHECK(sys.facade.getStatus(id) == "Approved");     // the compliance sign-off is still outstanding
    CHECK(sys.facade.completeWorkItem(id, loanTask::SIGNOFF, people::PIETER));
    CHECK(sys.facade.getStatus(id) == "Completed");
}

TEST_CASE("Loan exception: an amount over the limit is rejected") {
    TestSystem sys;
    CHECK(sys.facade.getStatus(sys.loanRequest(people::NOMVULA, "500000")) == "Rejected");
}

TEST_CASE("Loan exception: an ineligible customer is rejected") {
    TestSystem sys;
    CHECK(sys.facade.getStatus(sys.loanRequest(people::JOHAN)) == "Rejected");
}

TEST_CASE("Loan exception: nobody approves their own application") {
    TestSystem sys;
    int id = sys.loanRequest(people::PIETER, "50000");
    REQUIRE(sys.facade.getStatus(id) == "Accepted");
    CHECK_FALSE(sys.facade.canApprove(id, people::PIETER));
    CHECK_FALSE(sys.facade.approveRequest(id, people::PIETER));
    CHECK(sys.facade.getStatus(id) == "Accepted");
}

TEST_CASE("Loan exception: an analyst cannot approve, and silence never approves") {
    TestSystem sys(0);
    int id = sys.loanRequest();
    CHECK(sys.facade.getStatus(id) == "Accepted");     // strict policy ignores the zero timeout
    CHECK_FALSE(sys.facade.approveRequest(id, people::AYESHA));
    CHECK(sys.facade.processTimeouts() == 0);
}

TEST_CASE("Loan exception: the credit manager denies an application") {
    TestSystem sys;
    int id = sys.loanRequest();
    CHECK(sys.facade.rejectRequest(id, people::PIETER, "Insufficient credit history"));
    CHECK(sys.facade.getStatus(id) == "Denied");
}

// ------------------------------------------------------------------ one engine
TEST_CASE("Both industries run on the same engine at the same time without interfering") {
    TestSystem sys;
    int lab = sys.approvedLab();
    int loan = sys.approvedLoan();
    sys.facade.suspendRequest(lab, people::NAIDOO);
    CHECK(sys.facade.getStatus(lab) == "Suspended");
    CHECK(sys.facade.getStatus(loan) == "Approved");
    CHECK(sys.facade.completeWorkItem(loan, loanTask::CREDIT, people::AYESHA));
    CHECK_FALSE(sys.facade.suspendRequest(loan, people::NAIDOO));   // the lab approver has no say over loans
}
