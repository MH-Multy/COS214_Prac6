#include "doctest.h"

#include <memory>
#include <string>
#include <vector>

#include "ApproverObserver.h"
#include "AuditLogger.h"
#include "RequesterObserver.h"
#include "WorkflowEvent.h"
#include "WorkflowObserver.h"
#include "WorkflowSubject.h"

namespace {
// Domain participant IDs
const int DR_SARAH_KHUMALO = 101;  // Laboratory Pathologist
const int LAB_DIRECTOR     = 901;  // Head Researcher / Approver
const int BORROWER_ID      = 405;  // Retail Loan Applicant
const int RISK_OFFICER_ID  = 201;  // Credit Approver

/**
 * @brief Test Subject to isolate Observer event broadcasting without depending on other patterns.
 */
class MockWorkflowSubject : public WorkflowSubject {
private:
    int workflowId_;

public:
    explicit MockWorkflowSubject(int id) : workflowId_(id) {}

    int getWorkflowId() const { return workflowId_; }

    void triggerEvent(const std::string& type, int actorId, const std::string& message) {
        WorkflowEvent event(type, workflowId_, actorId, message);
        notifyObservers(event);
    }
};
}  // namespace

// ============================================================================
// 1. Subscription Management & Registry Safeguards
// ============================================================================

TEST_CASE("Observer: attach, detach, and registry tracking") {
    MockWorkflowSubject subject(500);
    auto auditLogger = std::make_shared<AuditLogger>();
    auto requester = std::make_shared<RequesterObserver>(DR_SARAH_KHUMALO);

    CHECK(subject.getObserverCount() == 0);

    subject.attach(auditLogger);
    subject.attach(requester);
    CHECK(subject.getObserverCount() == 2);

    // Prevent duplicate registration of the same observer
    subject.attach(auditLogger);
    CHECK(subject.getObserverCount() == 2);

    // Safe handling of null pointers
    subject.attach(nullptr);
    CHECK(subject.getObserverCount() == 2);

    // Detach subscriber
    subject.detach(requester);
    CHECK(subject.getObserverCount() == 1);

    // Detaching an unregistered or already-detached observer does not crash
    subject.detach(requester);
    CHECK(subject.getObserverCount() == 1);
}

// ============================================================================
// 2. Concrete Observer Functionality: AuditLogger
// ============================================================================

TEST_CASE("Observer: AuditLogger maintains complete chronological audit trail") {
    MockWorkflowSubject labSubject(501);
    auto auditLogger = std::make_shared<AuditLogger>();
    labSubject.attach(auditLogger);

    labSubject.triggerEvent("VALIDATED", -1, "Automated biosafety checks passed");
    labSubject.triggerEvent("APPROVED", LAB_DIRECTOR, "Class-II cabinet reservation confirmed");
    labSubject.triggerEvent("SUSPENDED", LAB_DIRECTOR, "Exhaust pressure sensor threshold exceeded");

    CHECK(auditLogger->getLogSize() == 3);
    const auto& log = auditLogger->getAuditLog();
    REQUIRE(log.size() == 3);

    // Verify events are preserved in order
    CHECK(log[0].find("VALIDATED") != std::string::npos);
    CHECK(log[1].find("APPROVED") != std::string::npos);
    CHECK(log[2].find("SUSPENDED") != std::string::npos);
    CHECK(log[2].find("Exhaust pressure") != std::string::npos);
}

// ============================================================================
// 3. Concrete Observer Functionality: RequesterObserver
// ============================================================================

TEST_CASE("Observer: RequesterObserver routes personal notifications into user inbox") {
    MockWorkflowSubject labSubject(501);
    auto drKhumaloInbox = std::make_shared<RequesterObserver>(DR_SARAH_KHUMALO);
    labSubject.attach(drKhumaloInbox);

    CHECK(drKhumaloInbox->getTargetRequesterId() == DR_SARAH_KHUMALO);
    CHECK(drKhumaloInbox->getUnreadCount() == 0);

    labSubject.triggerEvent("APPROVED", LAB_DIRECTOR, "Proposal accepted. Begin daily protocol log.");

    CHECK(drKhumaloInbox->getUnreadCount() == 1);
    REQUIRE(drKhumaloInbox->getInbox().size() == 1);

    const std::string& note = drKhumaloInbox->getInbox()[0];
    CHECK(note.find("Requester #101") != std::string::npos);
    CHECK(note.find("Workflow #501") != std::string::npos);
    CHECK(note.find("APPROVED") != std::string::npos);
}

// ============================================================================
// 4. Concrete Observer Functionality: ApproverObserver (Event Filtering)
// ============================================================================

TEST_CASE("Observer: ApproverObserver filters actionable events and discards routine transitions") {
    MockWorkflowSubject loanSubject(701);
    auto riskCommittee = std::make_shared<ApproverObserver>("RISK_COMMITTEE");
    loanSubject.attach(riskCommittee);

    CHECK(riskCommittee->getRole() == "RISK_COMMITTEE");
    CHECK(riskCommittee->getPendingCount() == 0);

    // 1. Actionable: VALIDATED -> Placed in review queue
    loanSubject.triggerEvent("VALIDATED", -1, "Preliminary credit scoring satisfied");
    CHECK(riskCommittee->getPendingCount() == 1);

    // 2. Routine operational steps: APPROVED & STARTED -> Ignored by manager queue
    loanSubject.triggerEvent("APPROVED", RISK_OFFICER_ID, "Facility granted at Tier 1");
    loanSubject.triggerEvent("STARTED", RISK_OFFICER_ID, "Funds disbursement in transit");
    CHECK(riskCommittee->getPendingCount() == 1);  // Unchanged

    // 3. Emergency / Actionable: SUSPENDED -> Placed in review queue
    loanSubject.triggerEvent("SUSPENDED", RISK_OFFICER_ID, "Collateral deed title discrepancy");
    CHECK(riskCommittee->getPendingCount() == 2);

    const auto& pending = riskCommittee->getPendingApprovals();
    REQUIRE(pending.size() == 2);
    CHECK(pending[0].find("RISK_COMMITTEE ACTION REQUIRED") != std::string::npos);
    CHECK(pending[1].find("Collateral deed title discrepancy") != std::string::npos);
}

// ============================================================================
// 5. Scenario Integration: Multi-Observer Broadcast & Dynamic Detach
// ============================================================================

TEST_CASE("Observer: Multi-observer broadcast and dynamic detachment isolation") {
    MockWorkflowSubject labSubject(502);

    auto auditLogger = std::make_shared<AuditLogger>();
    auto researcher = std::make_shared<RequesterObserver>(DR_SARAH_KHUMALO);
    auto approver = std::make_shared<ApproverObserver>("HEAD_RESEARCHER");

    labSubject.attach(auditLogger);
    labSubject.attach(researcher);
    labSubject.attach(approver);
    REQUIRE(labSubject.getObserverCount() == 3);

    // Event 1: Creation & Validation
    labSubject.triggerEvent("VALIDATED", -1, "Pathogen protocol validated");
    CHECK(auditLogger->getLogSize() == 1);
    CHECK(researcher->getUnreadCount() == 1);
    CHECK(approver->getPendingCount() == 1);

    // Dynamic Detach: Researcher opts out / unregisters active listener
    labSubject.detach(researcher);
    CHECK(labSubject.getObserverCount() == 2);

    // Event 2: Subsequent event after detach
    labSubject.triggerEvent("RESUMED", LAB_DIRECTOR, "Containment cleared by safety team");

    CHECK(auditLogger->getLogSize() == 2);
    CHECK(researcher->getUnreadCount() == 1);  // Detached subscriber receives no new alerts
    CHECK(approver->getPendingCount() == 1);   // RESUMED is not an actionable approver status
}