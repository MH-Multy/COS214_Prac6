#ifndef WORKFLOWREPORT_H
#define WORKFLOWREPORT_H

#include <string>
#include "ReportMemento.h"

/**
 * Memento Originator: the editable report attached to a workflow
 *        instance (for example lab test logs or loan assessment notes).
 *
 * Every edit increases the revision number. save() captures the current
 * state in a ReportMemento and restore() returns the report to exactly the
 * state held in a memento.
 */
class WorkflowReport 
{
public:
    WorkflowReport();

    /** Replace the report content and advance the revision. */
    void edit(const std::string& text);

    /** Capture the current content and revision. */
    ReportMemento save() const;

    /** Return the report to the exact state stored in @p m. */
    void restore(const ReportMemento& m);

    std::string getContent() const;
    int getRevision() const;

    /** Short single-line preview of a memento, for listing revisions. */
    static std::string preview(const ReportMemento& m, std::size_t maxLength = 30);

private:
    std::string content_;
    int revision_;
};

#endif
