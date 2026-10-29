#include "doctest.h"

#include <memory>
#include <stdexcept>
#include "CommandInvoker.h"
#include "FakeMediator.h"
#include "WorkflowDefinition.h"
#include "WorkflowEngine.h"
#include "WorkflowFacade.h"
#include "WorkflowInstance.h"

namespace {
const int APPLICANT = 1;
const int MANAGER = 7;

/** Everything a facade test needs, wired the way main() will wire it. */
struct Fixture {
    std::shared_ptr<WorkflowEngine> engine;
    std::shared_ptr<CommandInvoker> invoker;
    std::shared_ptr<FakeMediator> mediator;
    WorkflowFacade facade;

    Fixture()
        : engine(std::make_shared<WorkflowEngine>()),
          invoker(std::make_shared<CommandInvoker>()),
          mediator(std::make_shared<FakeMediator>()),
          facade(engine, invoker, mediator) {
        facade.registerWorkflow(std::make_shared<WorkflowDefinition>("Lab Test Request", 1));
        facade.registerWorkflow(std::make_shared<WorkflowDefinition>("Loan Application", 1));
    }

    /** Submit a request and play the mediator's part by validating it. */
    int submitted(const std::string& type) {
        int id = facade.submitRequest(type, WorkflowRequest("Request", APPLICANT));
        engine->getInstance(id)->validate();
        return id;
    }
};
}  // namespace

TEST_CASE("Facade: rejects missing collaborators") {
    std::shared_ptr<WorkflowEngine> engine = std::make_shared<WorkflowEngine>();
    std::shared_ptr<CommandInvoker> invoker = std::make_shared<CommandInvoker>();
    CHECK_THROWS_AS(WorkflowFacade(engine, invoker, std::shared_ptr<WorkflowMediator>()),
                    std::invalid_argument);
}

TEST_CASE("Facade: lists every registered workflow type") {
    Fixture f;
    std::vector<std::string> types = f.facade.listWorkflowTypes();
    REQUIRE(types.size() == 2);
    CHECK(types[0] == "Lab Test Request");
    CHECK(types[1] == "Loan Application");
}

TEST_CASE("Facade: submitting a request starts an instance and asks the mediator to validate it") {
    Fixture f;
    int id = f.facade.submitRequest("Loan Application", WorkflowRequest("Personal loan", APPLICANT));

    CHECK(f.facade.getStatus(id) == "PendingValidation");
    REQUIRE(f.mediator->validationRequests.size() == 1);
    CHECK(f.mediator->validationRequests[0] == id);
}

TEST_CASE("Facade: submitting an unknown workflow type throws") {
    Fixture f;
    CHECK_THROWS_AS(f.facade.submitRequest("Pizza Delivery", WorkflowRequest("x", 1)),
                    std::invalid_argument);
}

TEST_CASE("Facade: using a workflow id that does not exist throws") {
    Fixture f;
    CHECK_THROWS_AS(f.facade.approveRequest(42, MANAGER), std::invalid_argument);
    CHECK_THROWS_AS(f.facade.getStatus(42), std::invalid_argument);
    CHECK_THROWS_AS(f.facade.getHistory(42), std::invalid_argument);
    CHECK_THROWS_AS(f.facade.undoLastAction(42), std::invalid_argument);
}

TEST_CASE("Facade: approve moves the workflow on and is visible in its history") {
    Fixture f;
    int id = f.submitted("Lab Test Request");

    CHECK(f.facade.approveRequest(id, MANAGER));

    CHECK(f.facade.getStatus(id) == "Approved");
    CHECK(f.facade.getHistory(id).back().getType() == "APPROVED");
    CHECK(f.facade.getHistory(id).back().getActorId() == MANAGER);
}

TEST_CASE("Facade: an action that is invalid right now returns false without changing anything") {
    Fixture f;
    int id = f.submitted("Lab Test Request");
    CHECK_FALSE(f.facade.resumeRequest(id, MANAGER));   // not suspended
    CHECK(f.facade.getStatus(id) == "PendingApproval");
}

TEST_CASE("Facade: two workflows of different industries stay independent") {
    Fixture f;
    int lab = f.submitted("Lab Test Request");
    int loan = f.submitted("Loan Application");

    f.facade.rejectRequest(loan, MANAGER, "Affordability check failed");

    CHECK(f.facade.getStatus(loan) == "Rejected");
    CHECK(f.facade.getStatus(lab) == "PendingApproval");
}

TEST_CASE("Facade: undoLastAction reverses a suspend") {
    Fixture f;
    int id = f.submitted("Lab Test Request");
    f.facade.approveRequest(id, MANAGER);
    f.facade.suspendRequest(id, MANAGER);
    REQUIRE(f.facade.getStatus(id) == "Suspended");

    CHECK(f.facade.undoLastAction(id));
    CHECK(f.facade.getStatus(id) == "Approved");
}

TEST_CASE("Facade: report revisions can be listed and restored") {
    Fixture f;
    int id = f.submitted("Lab Test Request");
    f.facade.updateReport(id, APPLICANT, "Day 1: sample prepared");
    f.facade.updateReport(id, APPLICANT, "Day 2: contaminated, rerun needed");

    std::vector<std::string> revisions = f.facade.getReportRevisions(id);
    REQUIRE(revisions.size() == 2);
    CHECK(revisions[0].find("revision 1") != std::string::npos);

    CHECK(f.facade.restoreReport(id, 0, APPLICANT));
    CHECK(f.facade.getReport(id) == "Day 1: sample prepared");
    CHECK_FALSE(f.facade.restoreReport(id, 5, APPLICANT));   // no such revision
}

TEST_CASE("Facade: listWorkflows returns every running instance") {
    Fixture f;
    int a = f.submitted("Lab Test Request");
    int b = f.submitted("Loan Application");
    std::vector<int> ids = f.facade.listWorkflows();
    REQUIRE(ids.size() == 2);
    CHECK(ids[0] == a);
    CHECK(ids[1] == b);
}
