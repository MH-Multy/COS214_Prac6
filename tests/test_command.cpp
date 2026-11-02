#include "doctest.h"

#include <memory>
#include "ApproveCommand.h"
#include "CommandInvoker.h"
#include "CompleteWorkItemCommand.h"
#include "EditReportCommand.h"
#include "RejectCommand.h"
#include "ResumeCommand.h"
#include "SuspendCommand.h"
#include "TestSystem.h"
#include "WorkItem.h"
#include "WorkflowInstance.h"

TEST_CASE("Command: approve succeeds while awaiting a decision and is recorded") {
    TestSystem sys;
    CommandInvoker invoker;
    int id = sys.labRequest();
    CHECK(invoker.executeCommand(std::make_shared<ApproveCommand>(*sys.engine, id, people::NAIDOO)));
    CHECK(sys.facade.getStatus(id) == "Approved");
    REQUIRE(invoker.getHistory().size() == 1);
    CHECK(invoker.getHistory()[0] == "Workflow " + std::to_string(id) + ": approved by participant 103");
}

TEST_CASE("Command: an action not allowed in the current state fails and is not recorded") {
    TestSystem sys;
    CommandInvoker invoker;
    int id = sys.labRequest(people::THABO, "5");
    CHECK_FALSE(invoker.executeCommand(std::make_shared<ApproveCommand>(*sys.engine, id, people::NAIDOO)));
    CHECK(sys.facade.getStatus(id) == "Rejected");
    CHECK(invoker.getHistory().empty());
}

TEST_CASE("Command: an action by someone without permission fails") {
    TestSystem sys;
    CommandInvoker invoker;
    int id = sys.labRequest();
    CHECK_FALSE(invoker.executeCommand(std::make_shared<ApproveCommand>(*sys.engine, id, people::SARAH)));
    CHECK_FALSE(invoker.executeCommand(std::make_shared<SuspendCommand>(*sys.engine, id, people::SARAH)));
    CHECK(sys.facade.getStatus(id) == "Accepted");
}

TEST_CASE("Command: a command for a workflow that does not exist fails") {
    TestSystem sys;
    CommandInvoker invoker;
    CHECK_FALSE(invoker.executeCommand(std::make_shared<SuspendCommand>(*sys.engine, 99, people::NAIDOO)));
    CHECK_FALSE(invoker.executeCommand(std::shared_ptr<WorkflowAction>()));
}

TEST_CASE("Command: undoing a suspend resumes, undoing a resume suspends") {
    TestSystem sys;
    CommandInvoker invoker;
    int id = sys.approvedLab();
    invoker.executeCommand(std::make_shared<SuspendCommand>(*sys.engine, id, people::NAIDOO));
    REQUIRE(sys.facade.getStatus(id) == "Suspended");
    CHECK(invoker.undoLast(id));
    CHECK(sys.facade.getStatus(id) == "Approved");
    sys.engine->getInstance(id)->suspend(people::NAIDOO);
    invoker.executeCommand(std::make_shared<ResumeCommand>(*sys.engine, id, people::NAIDOO));
    CHECK(invoker.undoLast(id));
    CHECK(sys.facade.getStatus(id) == "Suspended");
}

TEST_CASE("Command: approvals and rejections cannot be undone") {
    TestSystem sys;
    CommandInvoker invoker;
    int a = sys.labRequest(), d = sys.loanRequest();
    invoker.executeCommand(std::make_shared<ApproveCommand>(*sys.engine, a, people::NAIDOO));
    invoker.executeCommand(std::make_shared<RejectCommand>(*sys.engine, d, people::PIETER, "Too risky"));
    CHECK_FALSE(invoker.undoLast(a));
    CHECK_FALSE(invoker.undoLast(d));
    CHECK(sys.facade.getStatus(a) == "Approved");
    CHECK(sys.facade.getStatus(d) == "Denied");
}

TEST_CASE("Command: undo only affects the workflow it is asked about") {
    TestSystem sys;
    CommandInvoker invoker;
    int lab = sys.approvedLab(), loan = sys.approvedLoan();
    invoker.executeCommand(std::make_shared<SuspendCommand>(*sys.engine, lab, people::NAIDOO));
    invoker.executeCommand(std::make_shared<SuspendCommand>(*sys.engine, loan, people::PIETER));
    CHECK(invoker.undoLast(lab));
    CHECK(sys.facade.getStatus(lab) == "Approved");
    CHECK(sys.facade.getStatus(loan) == "Suspended");
}

TEST_CASE("Command: completing a task can be undone, which reopens it") {
    TestSystem sys;
    CommandInvoker invoker;
    int id = sys.approvedLab();
    REQUIRE(invoker.executeCommand(std::make_shared<CompleteWorkItemCommand>(*sys.engine, id, labTask::RESERVE, people::LERATO)));
    CHECK(invoker.undoLast(id));
    CHECK(sys.engine->getInstance(id)->findItem(labTask::RESERVE)->getStatus() == "Approved");
    CHECK(sys.hasEvent(id, "ITEM_REOPENED"));
}

TEST_CASE("Command: a task can only be completed by its assignee") {
    TestSystem sys;
    CommandInvoker invoker;
    int id = sys.approvedLab();
    CHECK_FALSE(invoker.executeCommand(std::make_shared<CompleteWorkItemCommand>(*sys.engine, id, labTask::RESERVE, people::SARAH)));
}

TEST_CASE("Command + Memento: undoing a report edit restores the previous version") {
    TestSystem sys;
    CommandInvoker invoker;
    int id = sys.approvedLab();
    invoker.executeCommand(std::make_shared<EditReportCommand>(*sys.engine, id, people::SARAH, "Day 1"));
    invoker.executeCommand(std::make_shared<EditReportCommand>(*sys.engine, id, people::SARAH, "Day 1\nDay 2"));
    CHECK(invoker.undoLast(id));
    CHECK(sys.facade.getReport(id) == "Day 1");
}

TEST_CASE("Command: the report is locked while the workflow is suspended") {
    TestSystem sys;
    CommandInvoker invoker;
    int id = sys.approvedLab();
    sys.facade.suspendRequest(id, people::NAIDOO);
    CHECK_FALSE(invoker.executeCommand(std::make_shared<EditReportCommand>(*sys.engine, id, people::SARAH, "late")));
    CHECK(sys.facade.getReport(id).empty());
}
