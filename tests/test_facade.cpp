#include "doctest.h"

#include <memory>
#include <stdexcept>
#include "FakeMediator.h"
#include "TestSystem.h"
#include "WorkItem.h"
#include "WorkflowDefinition.h"
#include "WorkflowFactory.h"
#include "WorkflowInstance.h"

TEST_CASE("Facade: rejects missing collaborators") {
    std::shared_ptr<WorkflowEngine> engine = std::make_shared<WorkflowEngine>();
    std::shared_ptr<CommandInvoker> invoker = std::make_shared<CommandInvoker>();
    CHECK_THROWS_AS(WorkflowFacade(engine, invoker, std::shared_ptr<WorkflowMediator>()), std::invalid_argument);
}

TEST_CASE("Facade: in isolation, submitting asks the mediator to validate (mocked mediator)") {
    std::shared_ptr<WorkflowEngine> engine = std::make_shared<WorkflowEngine>();
    std::shared_ptr<FakeMediator> mediator = std::make_shared<FakeMediator>();
    WorkflowFacade facade(engine, std::make_shared<CommandInvoker>(), mediator);
    std::shared_ptr<WorkflowDefinition> def = std::make_shared<WorkflowDefinition>("Generic", 1);
    def->setFactory(std::make_shared<ManualReviewComponentFactory>());
    facade.registerWorkflow(def);
    int id = facade.submitRequest("Generic", WorkflowRequest("Request", 1));
    CHECK(facade.getStatus(id) == "Pending");             // the fake mediator does not validate
    REQUIRE(mediator->validationRequests.size() == 1);
    CHECK(mediator->validationRequests[0] == id);
}

TEST_CASE("Facade: lists both industries") {
    TestSystem sys;
    std::vector<std::string> types = sys.facade.listWorkflowTypes();
    REQUIRE(types.size() == 2);
    CHECK(types[0] == "Lab Test Request");
    CHECK(types[1] == "Loan Application");
}

TEST_CASE("Facade: unknown workflow types and ids are reported as errors") {
    TestSystem sys;
    CHECK_THROWS_AS(sys.facade.submitRequest("Pizza Delivery", WorkflowRequest("x", 1)), std::invalid_argument);
    CHECK_THROWS_AS(sys.facade.approveRequest(42, people::NAIDOO), std::invalid_argument);
    CHECK_THROWS_AS(sys.facade.getStatus(42), std::invalid_argument);
    CHECK_THROWS_AS(sys.facade.getWorkflow(42), std::invalid_argument);
    CHECK_THROWS_AS(sys.facade.undoLastAction(42), std::invalid_argument);
}

TEST_CASE("Facade: an action that is not allowed right now returns false and changes nothing") {
    TestSystem sys;
    int id = sys.labRequest();
    CHECK_FALSE(sys.facade.resumeRequest(id, people::NAIDOO));
    CHECK_FALSE(sys.facade.completeWorkItem(id, labTask::RESERVE, people::LERATO));
    CHECK(sys.facade.getStatus(id) == "Accepted");
}

TEST_CASE("Facade: available work appears only once the workflow is approved") {
    TestSystem sys;
    int id = sys.labRequest();
    CHECK(sys.facade.getAvailableWork(id).empty());
    CHECK(sys.facade.canApprove(id, people::NAIDOO));
    REQUIRE(sys.facade.approveRequest(id, people::NAIDOO));
    CHECK(sys.facade.getAvailableWork(id).size() == 2);
    CHECK_FALSE(sys.facade.canApprove(id, people::NAIDOO));   // already decided
}

TEST_CASE("Facade: report revisions can be listed and restored") {
    TestSystem sys;
    int id = sys.approvedLab();
    sys.facade.updateReport(id, people::SARAH, "Day 1");
    sys.facade.updateReport(id, people::SARAH, "Day 1\nDay 2");
    REQUIRE(sys.facade.getReportRevisions(id).size() == 2);
    CHECK(sys.facade.restoreReport(id, 0, people::SARAH));
    CHECK(sys.facade.getReport(id) == "Day 1");
    CHECK_FALSE(sys.facade.restoreReport(id, 5, people::SARAH));
}

TEST_CASE("Facade: audit log, approver queue, inbox, notifications and action history are available") {
    TestSystem sys;
    int id = sys.labRequest();
    sys.facade.approveRequest(id, people::NAIDOO);
    CHECK(sys.facade.getAuditLog().size() >= 4);
    CHECK_FALSE(sys.facade.getApproverQueue().empty());
    CHECK_FALSE(sys.facade.getRequesterInbox(id).empty());
    CHECK(sys.facade.getNotifications(people::SARAH).size() == 2);
    CHECK(sys.facade.getActionHistory().size() == 1);
    CHECK(sys.facade.participantName(people::NAIDOO) == "Prof Anil Naidoo");
    CHECK(sys.facade.participantName(999) == "#999");
}

TEST_CASE("Facade: a new workflow type can be created on the spot and run to completion") {
    TestSystem sys;
    sys.facade.registerParticipant(Participant(301, "Grace", std::vector<std::string>(1, "Manager")));
    sys.facade.registerParticipant(Participant(302, "Sipho", std::vector<std::string>(1, "Buyer")));
    sys.facade.registerParticipant(Participant(303, "Lindiwe", std::vector<std::string>(1, "Staff")));
    std::vector<std::pair<std::string, std::vector<std::string>>> stages;
    stages.push_back(std::make_pair(std::string("Procurement"), std::vector<std::string>(1, "Place the order (Buyer)")));
    std::vector<std::string> fields(1, "item");
    fields.push_back("cost");
    configureCustomWorkflow(sys.facade, "Equipment Purchase", "Buy things", "Manager", -1, fields, "cost", 50000,
                            stages, std::vector<bool>(1, true));
    CHECK_THROWS_AS(configureCustomWorkflow(sys.facade, "Equipment Purchase", "", "Manager", -1, fields, "", 0, stages,
                                            std::vector<bool>()), std::invalid_argument);

    WorkflowRequest tooDear("Server", 303);
    tooDear.setAttribute("item", "Server");
    tooDear.setAttribute("cost", "90000");
    CHECK(sys.facade.getStatus(sys.facade.submitRequest("Equipment Purchase", tooDear)) == "Rejected");

    WorkflowRequest ok("Laptop", 303);
    ok.setAttribute("item", "Laptop");
    ok.setAttribute("cost", "18000");
    int id = sys.facade.submitRequest("Equipment Purchase", ok);
    REQUIRE(sys.facade.approveRequest(id, 301));
    for (int guard = 0; guard < 5 && !sys.facade.getAvailableWork(id).empty(); ++guard) {
        std::shared_ptr<WorkItem> next = sys.facade.getAvailableWork(id)[0];
        REQUIRE(sys.facade.completeWorkItem(id, next->getId(), next->getAssignee()));
    }
    CHECK(sys.facade.getStatus(id) == "Completed");
}
