#include "doctest.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "AuthorizationHandler.h"
#include "IdentityVerificationHandler.h"
#include "OrganizationDirectory.h"
#include "RequiredAttributesHandler.h"
#include "ThresholdHandler.h"
#include "ValidationHandler.h"
#include "ValidationResult.h"
#include "WorkflowDefinition.h"
#include "WorkflowInstance.h"
#include "WorkflowRequest.h"

namespace {
// Actor IDs matching project domain scenarios
const int DR_SARAH_KHUMALO = 101;  // Registered Researcher / Pathologist
const int STUDENT_INTERN   = 102;  // Intern without elevated biohazard clearance
const int LOAN_OFFICER     = 201;  // Tier-1 Loan Officer
const int BRANCH_MANAGER   = 202;  // High-Tier Approver
const int UNREGISTERED_ID  = 999;  // Rogue or non-existent actor

/**
 * @brief Self-contained mock directory to isolate Chain of Responsibility unit tests.
 */
class MockDirectory : public OrganizationDirectory {
private:
    std::map<int, std::vector<std::string>> permissions_;

public:
    void registerActor(int id, const std::vector<std::string>& clearances) {
        permissions_[id] = clearances;
    }

    bool userExists(int id) const override {
        return permissions_.find(id) != permissions_.end();
    }

    bool isAuthorized(int id, const std::string& activity) const override {
        auto it = permissions_.find(id);
        if (it == permissions_.end()) {
            return false;
        }
        for (const auto& perm : it->second) {
            if (perm == activity) {
                return true;
            }
        }
        return false;
    }
};

/** Setup helper for mock directory with baseline scenario actors. */
std::shared_ptr<MockDirectory> createTestDirectory() {
    auto dir = std::make_shared<MockDirectory>();
    dir->registerActor(DR_SARAH_KHUMALO, {"ROLE_STAFF", "BIOSAFETY_LEVEL_2", "LAB_EQUIPMENT_RESERVATION"});
    dir->registerActor(STUDENT_INTERN, {"ROLE_INTERN", "LAB_EQUIPMENT_RESERVATION"});
    dir->registerActor(LOAN_OFFICER, {"ROLE_OFFICER", "LOAN_APPROVAL_TIER_1"});
    dir->registerActor(BRANCH_MANAGER, {"BRANCH_MANAGER", "LOAN_APPROVAL_TIER_1", "LOAN_APPROVAL_TIER_2"});
    return dir;
}

/** Helper to instantiate a test workflow instance. */
WorkflowInstance createLabInstance(int id, int requesterId, 
                                  const std::string& testType, 
                                  const std::string& equipment, 
                                  const std::string& biohazardLevel) {
    auto def = std::make_shared<const WorkflowDefinition>("Laboratory Biosafety Protocol", 1);
    WorkflowRequest req("Viral Assay Request", requesterId);
    if (!testType.empty()) req.setAttribute("test_type", testType);
    if (!equipment.empty()) req.setAttribute("equipment_needed", equipment);
    if (!biohazardLevel.empty()) req.setAttribute("biohazard_level", biohazardLevel);
    return WorkflowInstance(id, def, req);
}
}  // namespace

// ============================================================================
// 1. Concrete Handler Unit Tests
// ============================================================================

TEST_CASE("CoR: RequiredAttributesHandler passes when all mandatory keys exist") {
    RequiredAttributesHandler handler({"test_type", "equipment_needed"});
    WorkflowInstance wf = createLabInstance(1, DR_SARAH_KHUMALO, "Viral Culture", "Biosafety Cabinet", "2.0");

    ValidationResult result = handler.handle(wf);

    CHECK(result.isValid());
    CHECK(result.getIssues().empty());
}

TEST_CASE("CoR: RequiredAttributesHandler fails when one or more keys are omitted") {
    RequiredAttributesHandler handler({"test_type", "equipment_needed", "funding_code"});
    // Notice "funding_code" is omitted
    WorkflowInstance wf = createLabInstance(2, DR_SARAH_KHUMALO, "Viral Culture", "Biosafety Cabinet", "");

    ValidationResult result = handler.handle(wf);

    CHECK_FALSE(result.isValid());
    REQUIRE(result.getIssues().size() == 1);
    CHECK(result.getIssues()[0].handlerName == "RequiredAttributesHandler");
    CHECK(result.getIssues()[0].message.find("funding_code") != std::string::npos);
}

TEST_CASE("CoR: IdentityVerificationHandler verifies registered actors and flags unregistered IDs") {
    auto dir = createTestDirectory();
    IdentityVerificationHandler handler(dir);

    WorkflowInstance validWf = createLabInstance(3, DR_SARAH_KHUMALO, "Assay", "Cabinet", "2.0");
    WorkflowInstance invalidWf = createLabInstance(4, UNREGISTERED_ID, "Assay", "Cabinet", "2.0");

    CHECK(handler.handle(validWf).isValid());
    
    ValidationResult failResult = handler.handle(invalidWf);
    CHECK_FALSE(failResult.isValid());
    REQUIRE(failResult.getIssues().size() == 1);
    CHECK(failResult.getIssues()[0].handlerName == "IdentityVerificationHandler");
}

TEST_CASE("CoR: AuthorizationHandler validates clearance permissions per actor role") {
    auto dir = createTestDirectory();
    AuthorizationHandler authHandler(dir, "BIOSAFETY_LEVEL_2");

    WorkflowInstance doctorWf = createLabInstance(5, DR_SARAH_KHUMALO, "Assay", "Cabinet", "2.0");
    WorkflowInstance internWf = createLabInstance(6, STUDENT_INTERN, "Assay", "Cabinet", "2.0");

    CHECK(authHandler.handle(doctorWf).isValid());

    ValidationResult internResult = authHandler.handle(internWf);
    CHECK_FALSE(internResult.isValid());
    REQUIRE(internResult.getIssues().size() == 1);
    CHECK(internResult.getIssues()[0].handlerName == "AuthorizationHandler[BIOSAFETY_LEVEL_2]");
}

TEST_CASE("CoR: ThresholdHandler accepts values below maximum ceiling and flags excess") {
    ThresholdHandler handler("biohazard_level", 3.0);

    WorkflowInstance safeWf = createLabInstance(7, DR_SARAH_KHUMALO, "Assay", "Cabinet", "2.5");
    WorkflowInstance dangerousWf = createLabInstance(8, DR_SARAH_KHUMALO, "Assay", "Cabinet", "4.5");

    CHECK(handler.handle(safeWf).isValid());

    ValidationResult dangerResult = handler.handle(dangerousWf);
    CHECK_FALSE(dangerResult.isValid());
    REQUIRE(dangerResult.getIssues().size() == 1);
    CHECK(dangerResult.getIssues()[0].handlerName == "ThresholdHandler[biohazard_level]");
}

TEST_CASE("CoR: ThresholdHandler selectively steps aside when monitored attribute is not present") {
    ThresholdHandler handler("biohazard_level", 3.0);
    // Simulation / dry-lab test: no "biohazard_level" attribute
    WorkflowInstance dryLabWf = createLabInstance(9, DR_SARAH_KHUMALO, "Data Analysis", "Workstation PC", "");

    ValidationResult result = handler.handle(dryLabWf);

    CHECK(result.isValid());
    CHECK(result.getIssues().empty());
}

// ============================================================================
// 2. Dynamic Chaining & Traversal Policies (FAIL_FAST vs AUDIT_ACCUMULATE)
// ============================================================================

TEST_CASE("CoR: fluent setNext and recursive add build identical linear chains") {
    auto dir = createTestDirectory();
    
    // Chain A via fluent setNext
    auto headA = std::make_shared<RequiredAttributesHandler>(std::vector<std::string>{"test_type"});
    headA->setNext(std::make_shared<IdentityVerificationHandler>(dir))
         ->setNext(std::make_shared<ThresholdHandler>("biohazard_level", 3.0));

    // Chain B via recursive add
    auto headB = std::make_shared<RequiredAttributesHandler>(std::vector<std::string>{"test_type"});
    headB->add(std::make_shared<IdentityVerificationHandler>(dir));
    headB->add(std::make_shared<ThresholdHandler>("biohazard_level", 3.0));

    WorkflowInstance validWf = createLabInstance(10, DR_SARAH_KHUMALO, "Viral Assay", "Incubator", "2.0");

    CHECK(headA->handle(validWf).isValid());
    CHECK(headB->handle(validWf).isValid());
}

TEST_CASE("CoR: FAIL_FAST halts evaluation at the first broken rule") {
    auto dir = createTestDirectory();
    auto chain = std::make_shared<RequiredAttributesHandler>(std::vector<std::string>{"test_type"});
    chain->setNext(std::make_shared<IdentityVerificationHandler>(dir))
         ->setNext(std::make_shared<AuthorizationHandler>(dir, "BIOSAFETY_LEVEL_2"))
         ->setNext(std::make_shared<ThresholdHandler>("biohazard_level", 3.0));

    // Breaks 2 rules: Unregistered Actor ID AND Exceeds biohazard threshold
    WorkflowInstance rogueWf = createLabInstance(11, UNREGISTERED_ID, "Viral Assay", "Cabinet", "5.0");

    ValidationResult fastResult = chain->handle(rogueWf, ChainExecutionMode::FAIL_FAST);

    CHECK_FALSE(fastResult.isValid());
    // Since IdentityVerification is 2nd in line, FAIL_FAST stops there and does not evaluate subsequent links
    REQUIRE(fastResult.getIssues().size() == 1);
    CHECK(fastResult.getIssues()[0].handlerName == "IdentityVerificationHandler");
}

TEST_CASE("CoR: AUDIT_ACCUMULATE traverses the entire chain and compiles every violation") {
    auto dir = createTestDirectory();
    auto chain = std::make_shared<RequiredAttributesHandler>(std::vector<std::string>{"test_type"});
    chain->setNext(std::make_shared<IdentityVerificationHandler>(dir))
         ->setNext(std::make_shared<AuthorizationHandler>(dir, "BIOSAFETY_LEVEL_2"))
         ->setNext(std::make_shared<ThresholdHandler>("biohazard_level", 3.0));

    // Breaks 3 rules: Unregistered actor, Unauthorized activity clearance, AND Exceeds biohazard threshold
    WorkflowInstance rogueWf = createLabInstance(12, UNREGISTERED_ID, "Viral Assay", "Cabinet", "5.0");

    ValidationResult auditResult = chain->handle(rogueWf, ChainExecutionMode::AUDIT_ACCUMULATE);

    CHECK_FALSE(auditResult.isValid());
    // Collects Identity failure, Authorization failure, and Threshold failure
    REQUIRE(auditResult.getIssues().size() == 3);
    CHECK(auditResult.getIssues()[0].handlerName == "IdentityVerificationHandler");
    CHECK(auditResult.getIssues()[1].handlerName == "AuthorizationHandler[BIOSAFETY_LEVEL_2]");
    CHECK(auditResult.getIssues()[2].handlerName == "ThresholdHandler[biohazard_level]");
}
// ============================================================================
// 3. Scenario Integration: Loan Application Workflow
// ============================================================================

TEST_CASE("CoR: Loan Application scenario validates credit boundaries and clearances") {
    auto dir = createTestDirectory();

    auto loanChain = std::make_shared<RequiredAttributesHandler>(
        std::vector<std::string>{"loan_amount", "collateral_verified"}
    );
    loanChain->setNext(std::make_shared<IdentityVerificationHandler>(dir))
             ->setNext(std::make_shared<AuthorizationHandler>(dir, "LOAN_APPROVAL_TIER_1"))
             ->setNext(std::make_shared<ThresholdHandler>("loan_amount", 250000.0));

    auto def = std::make_shared<const WorkflowDefinition>("Retail Credit Protocol", 1);

    // Scenario A: Standard compliant loan application processed by Loan Officer (ID 201)
    WorkflowRequest validReq("Home Improvement Loan", LOAN_OFFICER);
    validReq.setAttribute("loan_amount", "150000.0");
    validReq.setAttribute("collateral_verified", "true");
    WorkflowInstance validLoan(20, def, validReq);

    ValidationResult validRes = loanChain->handle(validLoan, ChainExecutionMode::FAIL_FAST);
    CHECK(validRes.isValid());

    // Scenario B: Loan amount exceeds Tier-1 threshold limit (R300,000 > R250,000)
    WorkflowRequest excessReq("Commercial Micro Loan", LOAN_OFFICER);
    excessReq.setAttribute("loan_amount", "300000.0");
    excessReq.setAttribute("collateral_verified", "true");
    WorkflowInstance excessLoan(21, def, excessReq);

    ValidationResult excessRes = loanChain->handle(excessLoan, ChainExecutionMode::FAIL_FAST);
    CHECK_FALSE(excessRes.isValid());
    REQUIRE(excessRes.getIssues().size() == 1);
    CHECK(excessRes.getIssues()[0].handlerName == "ThresholdHandler[loan_amount]");
}