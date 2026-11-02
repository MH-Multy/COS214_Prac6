#include "doctest.h"

#include <stdexcept>
#include "ReportHistory.h"
#include "WorkflowReport.h"

TEST_CASE("Memento: a new report starts empty at revision 0") {
    WorkflowReport report;
    CHECK(report.getContent() == "");
    CHECK(report.getRevision() == 0);
}

TEST_CASE("Memento: each edit replaces the content and advances the revision") {
    WorkflowReport report;
    report.edit("Day 1: sample prepared");
    report.edit("Day 2: centrifuge run");
    CHECK(report.getContent() == "Day 2: centrifuge run");
    CHECK(report.getRevision() == 2);
}

TEST_CASE("Memento: restore returns the report to exactly the saved state") {
    WorkflowReport report;                       // Arrange
    report.edit("Income verified");
    ReportMemento saved = report.save();
    report.edit("Affordability failed");

    report.restore(saved);                       // Act

    CHECK(report.getContent() == "Income verified");   // Assert
    CHECK(report.getRevision() == 1);
}

TEST_CASE("Memento: a saved snapshot is not affected by later edits") {
    WorkflowReport report;
    report.edit("v1");
    ReportMemento saved = report.save();
    report.edit("v2");
    report.edit("v3");
    CHECK(saved.getRevision() == 1);
    report.restore(saved);
    CHECK(report.getContent() == "v1");
}

TEST_CASE("Memento: history stores snapshots in order") {
    WorkflowReport report;
    ReportHistory history;
    report.edit("first");
    history.addSnapshot(report.save());
    report.edit("second");
    history.addSnapshot(report.save());

    REQUIRE(history.size() == 2);
    CHECK(history.getSnapshot(0).getRevision() == 1);
    CHECK(history.getSnapshot(1).getRevision() == 2);
}

TEST_CASE("Memento: asking for a revision that does not exist throws") {
    ReportHistory history;
    CHECK_THROWS_AS(history.getSnapshot(0), std::out_of_range);
    CHECK_THROWS_AS(history.getSnapshot(-1), std::out_of_range);
}
