#include "doctest.h"

#include <memory>
#include "ApprovalPolicy.h"
#include "Participant.h"
#include "TestSystem.h"
#include "WorkflowDefinition.h"
#include "WorkflowInstance.h"

namespace {
WorkflowInstance accepted(std::shared_ptr<ApprovalPolicy> policy, int requester = 1) {
    WorkflowInstance wf(1, std::make_shared<const WorkflowDefinition>("Generic", 1), WorkflowRequest("Request", requester),
                        std::shared_ptr<CompositeWorkItem>(), policy);
    wf.validate();
    return wf;
}
Participant person(int id, const char* role) { return Participant(id, "P", std::vector<std::string>(1, role)); }
}  // namespace

TEST_CASE("Strategy: the timeout policy auto-approves once the time limit has passed") {
    CHECK(accepted(std::make_shared<TimeoutApprovalPolicy>("Manager", 0)).shouldAutoApprove());
}

TEST_CASE("Strategy: the timeout policy waits while the limit has not passed") {
    CHECK_FALSE(accepted(std::make_shared<TimeoutApprovalPolicy>("Manager", 3600)).shouldAutoApprove());
}

TEST_CASE("Strategy: only a request awaiting a decision can be auto-approved") {
    WorkflowInstance wf(1, std::make_shared<const WorkflowDefinition>("Generic", 1), WorkflowRequest("Request", 1),
                        std::shared_ptr<CompositeWorkItem>(), std::make_shared<TimeoutApprovalPolicy>("Manager", 0));
    CHECK_FALSE(wf.shouldAutoApprove());         // still Pending
}

TEST_CASE("Strategy: the strict policy never auto-approves") {
    CHECK_FALSE(accepted(std::make_shared<StrictApprovalPolicy>("Manager")).shouldAutoApprove());
}

TEST_CASE("Strategy: only holders of the approver role may approve") {
    WorkflowInstance wf = accepted(std::make_shared<TimeoutApprovalPolicy>("Manager", 60));
    CHECK(wf.canApprove(person(9, "Manager")));
    CHECK_FALSE(wf.canApprove(person(8, "Clerk")));
}

TEST_CASE("Strategy: the strict policy applies the four-eyes principle") {
    WorkflowInstance wf = accepted(std::make_shared<StrictApprovalPolicy>("Manager"), 9);
    CHECK_FALSE(wf.canApprove(person(9, "Manager")));   // the requester
    CHECK(wf.canApprove(person(10, "Manager")));
}

TEST_CASE("Strategy: each instance keeps its own policy, so industries run side by side") {
    TestSystem sys;
    CHECK(sys.facade.getWorkflow(sys.labRequest())->getApprovalPolicy()->describe().find("Auto-approves") != std::string::npos);
    CHECK(sys.facade.getWorkflow(sys.loanRequest())->getApprovalPolicy()->describe().find("four-eyes") != std::string::npos);
}
