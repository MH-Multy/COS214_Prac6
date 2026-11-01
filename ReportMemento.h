#ifndef REPORTMEMENTO_H
#define REPORTMEMENTO_H

#include <string>

/**
 * Memento: an opaque snapshot of a WorkflowReport.
 *
 * Only WorkflowReport (the Originator) can create a memento or read its
 * content, because the constructor and content are private and
 * WorkflowReport is a friend. Caretakers such as ReportHistory can store
 * and copy mementos and see which revision they hold, but cannot inspect
 * or change what is inside, which keeps the report's state encapsulated.
 */
class ReportMemento 
{
public:
    /** Revision number captured in this snapshot. */
    int getRevision() const { return revision_; }

private:
    friend class WorkflowReport;

    ReportMemento(const std::string& content, int revision)
        : content_(content), revision_(revision) {}

    std::string content_;
    int revision_;
};

#endif
