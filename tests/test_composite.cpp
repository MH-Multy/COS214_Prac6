#include "doctest.h"

#include <memory>
#include "CompositeWorkItem.h"
#include "Task.h"

namespace {
const int RESEARCHER = 5;
const int ANALYST = 6;

/** An approved task already assigned to @p actor, i.e. ready to be done. */
shared_ptr<Task> readyTask(int id, const string& name, int actor) {
    shared_ptr<Task> t = make_shared<Task>(id, name, "Researcher", "Approved");
    t->assignTo(actor);
    return t;
}
}  // namespace

// ------------------------------------------------------------------ Task (leaf)

TEST_CASE("Task: a new task is pending, not available and not complete") {
    Task task(1, "Reserve equipment", "Researcher");
    CHECK(task.getStatus() == "Pending");
    CHECK_FALSE(task.isAvailable());
    CHECK_FALSE(task.isComplete());
    CHECK(task.getAssignee() == -1);
}

TEST_CASE("Task: the assigned participant can complete an available task") {
    shared_ptr<Task> task = readyTask(1, "Run test", RESEARCHER);   // Arrange

    bool done = task->execute(RESEARCHER);                          // Act

    CHECK(done);                                                    // Assert
    CHECK(task->isComplete());
    CHECK(task->getStatus() == "Completed");
}

TEST_CASE("Task: someone other than the assignee cannot complete it") {
    shared_ptr<Task> task = readyTask(1, "Run test", RESEARCHER);
    CHECK_FALSE(task->execute(ANALYST));
    CHECK_FALSE(task->isComplete());
}

TEST_CASE("Task: an unassigned task cannot be completed by anyone") {
    Task task(1, "Run test", "Researcher", "Approved");
    CHECK_FALSE(task.execute(RESEARCHER));
}

TEST_CASE("Task: a pending task cannot be completed, even by its assignee") {
    Task task(1, "Run test", "Researcher");
    task.assignTo(RESEARCHER);
    CHECK_FALSE(task.execute(RESEARCHER));
    CHECK(task.getStatus() == "Pending");
}

TEST_CASE("Task: a completed task cannot be completed twice") {
    shared_ptr<Task> task = readyTask(1, "Run test", RESEARCHER);
    REQUIRE(task->execute(RESEARCHER));
    CHECK_FALSE(task->execute(RESEARCHER));
}

TEST_CASE("Task: a task is a leaf with no children") {
    Task task(1, "Run test", "Researcher");
    CHECK(task.getChildren().empty());
    CHECK(task.getRequiredRole() == "Researcher");
}

// ------------------------------------------------------------------ CompositeWorkItem

TEST_CASE("Composite: adding a null item is ignored") {
    CompositeWorkItem stage(10, "Screening");
    stage.add(shared_ptr<WorkItem>());
    CHECK(stage.getChildren().empty());
}

TEST_CASE("Composite: a parallel stage completes only when every child is done") {
    CompositeWorkItem stage(10, "Execution");                 // parallel by default
    stage.add(readyTask(1, "Run test", RESEARCHER));
    stage.add(readyTask(2, "Write log", RESEARCHER));
    stage.add(readyTask(3, "Analyse results", ANALYST));

    CHECK(stage.execute(RESEARCHER));                         // does both researcher tasks
    CHECK_FALSE(stage.isComplete());                          // analyst task still open
    CHECK(stage.getStatus() == "Pending");

    CHECK(stage.execute(ANALYST));
    CHECK(stage.isComplete());
    CHECK(stage.getStatus() == "Completed");
}

TEST_CASE("Composite: a parallel stage is available if any child is available") {
    CompositeWorkItem stage(10, "Execution");
    stage.add(make_shared<Task>(1, "Blocked task", "Researcher"));   // pending
    stage.add(readyTask(2, "Ready task", RESEARCHER));
    CHECK(stage.isAvailable());
}

TEST_CASE("Composite: a sequential stage does its tasks one at a time, in order") {
    CompositeWorkItem stage(10, "Loan processing", true);
    shared_ptr<Task> credit = readyTask(1, "Credit check", ANALYST);
    shared_ptr<Task> affordability = readyTask(2, "Affordability check", ANALYST);
    stage.add(credit);
    stage.add(affordability);

    CHECK(stage.execute(ANALYST));
    CHECK(credit->isComplete());
    CHECK_FALSE(affordability->isComplete());                 // not reached yet

    CHECK(stage.execute(ANALYST));
    CHECK(affordability->isComplete());
    CHECK(stage.isComplete());
}

TEST_CASE("Composite: a sequential stage is blocked while its current task is not available") {
    CompositeWorkItem stage(10, "Loan processing", true);
    stage.add(make_shared<Task>(1, "Credit check", "Analyst"));       // pending: blocks the stage
    shared_ptr<Task> later = readyTask(2, "Affordability check", ANALYST);
    stage.add(later);

    CHECK_FALSE(stage.isAvailable());
    CHECK_FALSE(stage.execute(ANALYST));
    CHECK_FALSE(later->isComplete());                          // order is enforced
}

TEST_CASE("Composite: nested stages are executed and completed through the root") {
    CompositeWorkItem root(100, "Lab test request");
    shared_ptr<CompositeWorkItem> prep = make_shared<CompositeWorkItem>(10, "Preparation");
    shared_ptr<CompositeWorkItem> run = make_shared<CompositeWorkItem>(20, "Execution");
    prep->add(readyTask(1, "Reserve equipment", RESEARCHER));
    run->add(readyTask(2, "Run test", RESEARCHER));
    root.add(prep);
    root.add(run);

    CHECK_FALSE(root.isComplete());
    CHECK(root.execute(RESEARCHER));

    CHECK(prep->isComplete());
    CHECK(run->isComplete());
    CHECK(root.isComplete());
    CHECK(root.getStatus() == "Completed");
}

TEST_CASE("Composite: an empty stage counts as complete but has no available work") {
    CompositeWorkItem empty(10, "Nothing to do");
    CHECK(empty.isComplete());
    CHECK_FALSE(empty.isAvailable());
    CHECK_FALSE(empty.execute(RESEARCHER));
}

// ------------------------------------------------------------------ releasing work (added for the engine)

TEST_CASE("Composite: releasing a task makes a pending task available (status Approved)") {
    Task task(1, "Run test", "Researcher");
    task.release();
    CHECK(task.getStatus() == "Approved");
    CHECK(task.isAvailable());
}

TEST_CASE("Composite: a sequential stage releases the next task only after the current one is done") {
    CompositeWorkItem stage(10, "Testing", true);
    shared_ptr<Task> first = make_shared<Task>(1, "Run test", "Researcher");
    shared_ptr<Task> second = make_shared<Task>(2, "Record results", "Researcher");
    stage.add(first);
    stage.add(second);
    stage.release();
    CHECK(first->isAvailable());
    CHECK_FALSE(second->isAvailable());
    first->assignTo(RESEARCHER);
    REQUIRE(first->execute(RESEARCHER));
    stage.release();
    CHECK(second->isAvailable());
}

TEST_CASE("Composite: a parallel stage releases every task at once") {
    CompositeWorkItem stage(10, "Preparation");
    shared_ptr<Task> a = make_shared<Task>(1, "Reserve equipment", "LabTechnician");
    shared_ptr<Task> b = make_shared<Task>(2, "Prepare samples", "Researcher");
    stage.add(a);
    stage.add(b);
    stage.release();
    CHECK(a->isAvailable());
    CHECK(b->isAvailable());
}

TEST_CASE("Composite: a completed task can be reopened, a pending one cannot") {
    shared_ptr<Task> task = readyTask(1, "Run test", RESEARCHER);
    CHECK_FALSE(task->reopen());
    REQUIRE(task->execute(RESEARCHER));
    CHECK(task->reopen());
    CHECK(task->isAvailable());
}
