#include "doctest.h"

#include <memory>
#include <string>
#include <vector>

#include "LegacyRecordSystem.h"
#include "OrganizationDirectory.h"
#include "RecordSystemAdapter.h"

namespace {
// Standard actor IDs from project scenarios
const int DR_SARAH_KHUMALO = 101;     // Pre-seeded Lab Researcher
const int STUDENT_INTERN   = 102;     // Pre-seeded Intern
const int LOAN_OFFICER     = 201;     // Scenario 2 Loan Officer
const int BRANCH_MANAGER   = 202;     // Scenario 2 Branch Manager
const int UNREGISTERED_ID  = 999;     // Non-existent actor

/** Helper to seed an adaptee with loan scenario records. */
std::shared_ptr<LegacyRecordSystem> createLoanSystemAdaptee() {
    auto legacy = std::make_shared<LegacyRecordSystem>();
    legacy->insertRecord(LOAN_OFFICER, {"ROLE_OFFICER", "LOAN_APPROVAL_TIER_1"});
    legacy->insertRecord(BRANCH_MANAGER, {"BRANCH_MANAGER", "LOAN_APPROVAL_TIER_1", 
                                          "LOAN_APPROVAL_TIER_2", "OVERRIDE_COLLATERAL"});
    return legacy;
}
}  // namespace

TEST_CASE("Adapter: polymorphic target interface compliance and dynamic binding") {
    auto legacy = std::make_shared<LegacyRecordSystem>();
    std::unique_ptr<OrganizationDirectory> directory(new RecordSystemAdapter(legacy));

    REQUIRE(directory != nullptr);
    CHECK(directory->userExists(DR_SARAH_KHUMALO));
    CHECK_FALSE(directory->userExists(UNREGISTERED_ID));
}

TEST_CASE("Adapter: user existence translation from legacy status strings") {
    auto legacy = std::make_shared<LegacyRecordSystem>();
    RecordSystemAdapter adapter(legacy);

    // Pre-seeded records in LegacyRecordSystem (STATUS:ACTIVE;ID:xxx)
    CHECK(adapter.userExists(DR_SARAH_KHUMALO));
    CHECK(adapter.userExists(STUDENT_INTERN));

    // Non-existent and invalid IDs (STATUS:NOT_FOUND)
    CHECK_FALSE(adapter.userExists(UNREGISTERED_ID));
    CHECK_FALSE(adapter.userExists(-1));
    CHECK_FALSE(adapter.userExists(0));
}

TEST_CASE("Adapter: parses comma-delimited permission tokens correctly") {
    auto legacy = std::make_shared<LegacyRecordSystem>();
    RecordSystemAdapter adapter(legacy);

    // ID 101 holds: {"ROLE_STAFF", "BIOSAFETY_LEVEL_2", "LAB_EQUIPMENT_RESERVATION"}
    CHECK(adapter.isAuthorized(DR_SARAH_KHUMALO, "ROLE_STAFF"));
    CHECK(adapter.isAuthorized(DR_SARAH_KHUMALO, "BIOSAFETY_LEVEL_2"));
    CHECK(adapter.isAuthorized(DR_SARAH_KHUMALO, "LAB_EQUIPMENT_RESERVATION"));

    // Lacks admin clearance
    CHECK_FALSE(adapter.isAuthorized(DR_SARAH_KHUMALO, "SYSTEM_ADMIN"));

    // ID 102 holds only {"ROLE_INTERN", "LAB_EQUIPMENT_RESERVATION"}
    CHECK(adapter.isAuthorized(STUDENT_INTERN, "LAB_EQUIPMENT_RESERVATION"));
    CHECK_FALSE(adapter.isAuthorized(STUDENT_INTERN, "BIOSAFETY_LEVEL_2"));
}

TEST_CASE("Adapter: dynamically added loan officer records are adapted correctly") {
    auto legacy = createLoanSystemAdaptee();
    RecordSystemAdapter adapter(legacy);

    // Officer 201 has TIER_1 approval
    CHECK(adapter.userExists(LOAN_OFFICER));
    CHECK(adapter.isAuthorized(LOAN_OFFICER, "LOAN_APPROVAL_TIER_1"));
    CHECK_FALSE(adapter.isAuthorized(LOAN_OFFICER, "LOAN_APPROVAL_TIER_2"));

    // Manager 202 has TIER_1, TIER_2, and OVERRIDE clearances
    CHECK(adapter.userExists(BRANCH_MANAGER));
    CHECK(adapter.isAuthorized(BRANCH_MANAGER, "LOAN_APPROVAL_TIER_1"));
    CHECK(adapter.isAuthorized(BRANCH_MANAGER, "LOAN_APPROVAL_TIER_2"));
    CHECK(adapter.isAuthorized(BRANCH_MANAGER, "OVERRIDE_COLLATERAL"));
}

TEST_CASE("Adapter: exact token matching prevents prefix and substring collisions") {
    auto legacy = std::make_shared<LegacyRecordSystem>();
    // Substring trap: "ROLE" is a prefix of "ROLE_STAFF"
    legacy->insertRecord(300, {"ROLE_SUPERVISOR", "LEVEL_2"});
    RecordSystemAdapter adapter(legacy);

    CHECK(adapter.isAuthorized(300, "ROLE_SUPERVISOR"));
    CHECK(adapter.isAuthorized(300, "LEVEL_2"));

    // Must not match partial substrings
    CHECK_FALSE(adapter.isAuthorized(300, "ROLE"));
    CHECK_FALSE(adapter.isAuthorized(300, "SUPERVISOR"));
    CHECK_FALSE(adapter.isAuthorized(300, "LEVEL"));
}

TEST_CASE("Adapter: empty roles list results in user existing with zero permissions") {
    auto legacy = std::make_shared<LegacyRecordSystem>();
    legacy->insertRecord(350, std::vector<std::string>{});
    RecordSystemAdapter adapter(legacy);

    CHECK(adapter.userExists(350));
    CHECK_FALSE(adapter.isAuthorized(350, "ROLE_STAFF"));
    CHECK_FALSE(adapter.isAuthorized(350, ""));
}

TEST_CASE("Adapter: non-existent user returns false for any clearance check") {
    auto legacy = std::make_shared<LegacyRecordSystem>();
    RecordSystemAdapter adapter(legacy);

    CHECK_FALSE(adapter.isAuthorized(UNREGISTERED_ID, "ROLE_STAFF"));
    CHECK_FALSE(adapter.isAuthorized(UNREGISTERED_ID, "BIOSAFETY_LEVEL_2"));
}

TEST_CASE("Adapter: null adaptee reference fails safely without crashing") {
    RecordSystemAdapter nullAdapter(nullptr);

    CHECK_FALSE(nullAdapter.userExists(DR_SARAH_KHUMALO));
    CHECK_FALSE(nullAdapter.isAuthorized(DR_SARAH_KHUMALO, "BIOSAFETY_LEVEL_2"));
}