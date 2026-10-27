#include "doctest.h"

#include <memory>
#include "ApproveCommand.h"
#include "CommandInvoker.h"
#include "CompleteWorkItemCommand.h"
#include "EditReportCommand.h"
#include "RejectCommand.h"
#include "ResumeCommand.h"
#include "SuspendCommand.h"
#include "WorkflowDefinition.h"
#include "WorkflowEngine.h"
#include "WorkflowInstance.h"

namespace {
const int MANAGER = 7;

/** Start a workflow and move it to PendingApproval. */
int pendingApproval(WorkflowEngine& engine) {
    std::shared_ptr<const WorkflowDefinition> def =
        std::make_shared<WorkflowDefinition>("Generic", 1);
    int id = engine.startWorkflow(def, WorkflowRequest("Request", 1));
    engine.getInstance(id)->validate();
    return id;
}

/** Start a workflow and move it to InProgress. */
int inProgress(WorkflowEngine& engine) {
    int id = pendingApproval(engine);
    engine.getInstance(id)->approve(MANAGER);
    engine.getInstance(id)->start();
    return id;
}
}  // namespace

TEST_CASE("Command: approve succeeds while awaiting approval and is recorded") {
    WorkflowEngine engine;
    CommandInvoker invoker;
    int id = pendingApproval(engine);

    bool ok = invoker.executeCommand(std::make_shared<ApproveCommand>(engine, id, MANAGER));

    CHECK(ok);
    CHECK(engine.getInstance(id)->getStateName() == "Approved");
    REQUIRE(invoker.getHistory().size() == 1);
    CHECK(invoker.getHistory()[0].find("approved") != std::string::npos);
}

TEST_CASE("Command: an action not allowed in the current state fails and is not recorded") {
    WorkflowEngine engine;
    CommandInvoker invoker;
    int id = engine.startWorkflow(std::make_shared<WorkflowDefinition>("Generic", 1),
                                  WorkflowRequest("Request", 1));   // still PendingValidation

    CHECK_FALSE(invoker.executeCommand(std::make_shared<ApproveCommand>(engine, id, MANAGER)));
    CHECK(engine.getInstance(id)->getStateName() == "PendingValidation");
    CHECK(invoker.getHistory().empty());
}

TEST_CASE("Command: a command for a workflow that does not exist fails") {
    WorkflowEngine engine;
    CommandInvoker invoker;
    CHECK_FALSE(invoker.executeCommand(std::make_shared<SuspendCommand>(engine, 99, MANAGER)));
}

TEST_CASE("Command: undoing a suspend resumes the workflow") {
    WorkflowEngine engine;
    CommandInvoker invoker;
    int id = inProgress(engine);
    invoker.executeCommand(std::make_shared<SuspendCommand>(engine, id, MANAGER));
    REQUIRE(engine.getInstance(id)->getStateName() == "Suspended");

    CHECK(invoker.undoLast(id));

    CHECK(engine.getInstance(id)->getStateName() == "InProgress");
    CHECK(invoker.getHistory().empty());
}

TEST_CASE("Command: undoing a resume suspends the workflow again") {
    WorkflowEngine engine;
    CommandInvoker invoker;
    int id = inProgress(engine);
    engine.getInstance(id)->suspend(MANAGER);
    invoker.executeCommand(std::make_shared<ResumeCommand>(engine, id, MANAGER));

    CHECK(invoker.undoLast(id));
    CHECK(engine.getInstance(id)->getStateName() == "Suspended");
}

TEST_CASE("Command: approvals and rejections cannot be undone") {
    WorkflowEngine engine;
    CommandInvoker invoker;
    int a = pendingApproval(engine);
    int r = pendingApproval(engine);
    invoker.executeCommand(std::make_shared<ApproveCommand>(engine, a, MANAGER));
    invoker.executeCommand(std::make_shared<RejectCommand>(engine, r, MANAGER, "Incomplete"));

    CHECK_FALSE(invoker.undoLast(a));
    CHECK_FALSE(invoker.undoLast(r));
    CHECK(engine.getInstance(a)->getStateName() == "Approved");
    CHECK(engine.getInstance(r)->getStateName() == "Rejected");
}

TEST_CASE("Command: undo only affects the workflow it is asked about (instance isolation)") {
    WorkflowEngine engine;
    CommandInvoker invoker;
    int first = inProgress(engine);
    int second = inProgress(engine);
    invoker.executeCommand(std::make_shared<SuspendCommand>(engine, first, MANAGER));
    invoker.executeCommand(std::make_shared<SuspendCommand>(engine, second, MANAGER));

    CHECK(invoker.undoLast(first));     // second was suspended more recently

    CHECK(engine.getInstance(first)->getStateName() == "InProgress");
    CHECK(engine.getInstance(second)->getStateName() == "Suspended");
}

TEST_CASE("Command: undo with nothing to undo returns false") {
    WorkflowEngine engine;
    CommandInvoker invoker;
    int id = inProgress(engine);
    CHECK_FALSE(invoker.undoLast(id));
}

TEST_CASE("Command: completing a work item can be undone") {
    WorkflowEngine engine;
    CommandInvoker invoker;
    int id = inProgress(engine);
    engine.getInstance(id)->addPlaceholderItem(1, "Run test");
    REQUIRE(invoker.executeCommand(std::make_shared<CompleteWorkItemCommand>(engine, id, 1, MANAGER)));
    CHECK(engine.getInstance(id)->getHistory().back().getType() == "ITEM_COMPLETED");

    CHECK(invoker.undoLast(id));

    CHECK(engine.getInstance(id)->getHistory().back().getType() == "ITEM_REOPENED");
    CHECK_FALSE(engine.getInstance(id)->complete());   // the item is open again, so the workflow can't finish
    CHECK(engine.getInstance(id)->getStateName() == "InProgress");
}

TEST_CASE("Command + Memento: undoing a report edit restores the previous version") {
    WorkflowEngine engine;
    CommandInvoker invoker;
    int id = inProgress(engine);
    invoker.executeCommand(std::make_shared<EditReportCommand>(engine, id, MANAGER, "Draft 1"));
    invoker.executeCommand(std::make_shared<EditReportCommand>(engine, id, MANAGER, "Draft 2"));
    std::shared_ptr<WorkflowInstance> wf = engine.getInstance(id);
    REQUIRE(wf->getReport().getContent() == "Draft 2");

    CHECK(invoker.undoLast(id));

    CHECK(wf->getReport().getContent() == "Draft 1");
    CHECK(wf->getReport().getRevision() == 1);
    CHECK(wf->getReportHistory().size() == 2);   // history keeps every version that was saved
}
