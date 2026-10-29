#include "EditReportCommand.h"
#include "WorkflowInstance.h"

EditReportCommand::EditReportCommand(WorkflowEngine& engine, int workflowId, int actorId,
                                     const std::string& text)
    : WorkflowAction(engine, workflowId, actorId), text_(text) {}

bool EditReportCommand::execute() {
    std::shared_ptr<WorkflowInstance> wf = target();
    if (!wf) return false;
    WorkflowReport& report = wf->getReport();
    before_.reset(new ReportMemento(report.save()));
    report.edit(text_);
    wf->getReportHistory().addSnapshot(report.save());
    wf->addEvent(WorkflowEvent("REPORT_EDITED", workflowId_, actorId_,
                               "Report now at revision " + std::to_string(report.getRevision())));
    return true;
}

bool EditReportCommand::undo() {
    std::shared_ptr<WorkflowInstance> wf = target();
    if (!wf || !before_) return false;
    wf->getReport().restore(*before_);
    wf->addEvent(WorkflowEvent("REPORT_REVERTED", workflowId_, actorId_,
                               "Report reverted to revision " + std::to_string(before_->getRevision())));
    before_.reset();
    return true;
}

std::string EditReportCommand::describe() const {
    return "Workflow " + std::to_string(workflowId_) + ": report edited by participant " +
           std::to_string(actorId_);
}
