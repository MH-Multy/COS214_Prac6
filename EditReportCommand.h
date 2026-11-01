#ifndef EDITREPORTCOMMAND_H
#define EDITREPORTCOMMAND_H

#include <memory>
#include "ReportMemento.h"
#include "WorkflowAction.h"

/**
 *  Edit the workflow's report. This is where Command and Memento meet.
 *
 * Before editing, the command asks the report for a memento of its current
 * state and keeps it; undo() hands that memento back to the report, so the
 * command never needs to know how a report stores its content. Every new
 * version is also added to the instance's ReportHistory so a participant can
 * later roll back to any revision, not just the previous one.
 */
class EditReportCommand : public WorkflowAction 
{
public:
    EditReportCommand(WorkflowEngine& engine, int workflowId, int actorId, const std::string& text);
    bool execute();
    bool undo();
    std::string describe() const;

private:
    std::string text_;
    std::unique_ptr<ReportMemento> before_;
};

#endif
