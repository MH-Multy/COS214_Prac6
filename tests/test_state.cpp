#include "doctest.h"

#include <memory>
#include "CompositeWorkItem.h"
#include "Task.h"
#include "WorkflowDefinition.h"
#include "WorkflowInstance.h"
#include "workflowState.h"

namespace {
/** No participant registry, so only the State logic is tested. */
WorkflowInstance fresh(std::shared_ptr<CompositeWorkItem> root = std::shared_ptr<CompositeWorkItem>()) {
    return WorkflowInstance(1, std::make_shared<const WorkflowDefinition>("Generic", 1), WorkflowRequest("Request", 1), root);
}
bool hasIllegal(const WorkflowInstance& wf) {
    for (std::size_t i = 0; i < wf.getHistory().size(); ++i)
        if (wf.getHistory()[i].getType() == "ILLEGAL_STATE") return true;
    return false;
}
}  // namespace

TEST_CASE("State: a new instance starts Pending") {
    CHECK(fresh().getStateName() == "Pending");
}

TEST_CASE("State: Pending -> Accepted -> Approved -> Completed") {
    WorkflowInstance wf = fresh();
    CHECK(wf.validate());
    CHECK(wf.getStateName() == "Accepted");
    CHECK(wf.approve(7));
    CHECK(wf.getStateName() == "Approved");
    CHECK(wf.complete());                        // empty work tree: nothing outstanding
    CHECK(wf.getStateName() == "Completed");
}

TEST_CASE("State: validation failure Pending -> Rejected") {
    WorkflowInstance wf = fresh();
    CHECK(wf.reject(-1, "missing riskLevel"));
    CHECK(wf.getStateName() == "Rejected");
}

TEST_CASE("State: a person denies, Accepted -> Denied") {
    WorkflowInstance wf = fresh();
    REQUIRE(wf.validate());
    CHECK(wf.reject(7, "Budget unavailable"));
    CHECK(wf.getStateName() == "Denied");
}

TEST_CASE("State: illegal transitions are refused, recorded, and change nothing") {
    WorkflowInstance wf = fresh();
    CHECK_FALSE(wf.approve(7));                  // Pending cannot be approved
    CHECK(wf.getStateName() == "Pending");
    CHECK(hasIllegal(wf));
    CHECK_FALSE(wf.suspend(7));
    CHECK_FALSE(wf.resume(7));
    REQUIRE(wf.validate());
    CHECK_FALSE(wf.validate());                  // cannot be validated twice
    CHECK_FALSE(wf.suspend(7));                  // only approved work can be suspended
    CHECK(wf.getStateName() == "Accepted");
}

TEST_CASE("State: suspend and resume (SuspendedState::accepted resumes)") {
    WorkflowInstance wf = fresh();
    REQUIRE(wf.validate());
    REQUIRE(wf.approve(7));
    REQUIRE(wf.suspend(7));
    CHECK(wf.getStateName() == "Suspended");
    CHECK_FALSE(wf.complete());
    CHECK(wf.resume(7));
    CHECK(wf.getStateName() == "Approved");
}

TEST_CASE("State: a workflow cannot complete while work is outstanding") {
    std::shared_ptr<CompositeWorkItem> root = std::make_shared<CompositeWorkItem>(1, "Root", true);
    root->add(std::make_shared<Task>(2, "Run test", "Researcher"));
    WorkflowInstance wf = fresh(root);
    REQUIRE(wf.validate());
    REQUIRE(wf.approve(7));
    CHECK_FALSE(wf.complete());
    CHECK(wf.getStateName() == "Approved");
}

TEST_CASE("State: end states refuse every transition") {
    WorkflowInstance wf = fresh();
    REQUIRE(wf.reject(-1, "x"));
    CHECK_FALSE(wf.validate());
    CHECK_FALSE(wf.approve(7));
    CHECK_FALSE(wf.reject(7, "x"));
    CHECK_FALSE(wf.resume(7));
    CHECK(wf.getStateName() == "Rejected");
}

TEST_CASE("State: each concrete state names itself") {
    CHECK(PendingState().getStateName() == "Pending");
    CHECK(AcceptedState().getStateName() == "Accepted");
    CHECK(ApprovedState().getStateName() == "Approved");
    CHECK(RejectedState().getStateName() == "Rejected");
    CHECK(DeniedState().getStateName() == "Denied");
    CHECK(CompletedState().getStateName() == "Completed");
    CHECK(SuspendedState().getStateName() == "Suspended");
}
