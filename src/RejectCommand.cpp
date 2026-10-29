#include "RejectCommand.h"
#include "WorkflowInstance.h"

RejectCommand::RejectCommand(WorkflowEngine& engine, int workflowId, int actorId,
                             const std::string& reason)
    : WorkflowAction(engine, workflowId, actorId), reason_(reason) {}

bool RejectCommand::execute() 
{
    std::shared_ptr<WorkflowInstance> wf = target();
    return wf && wf->reject(actorId_, reason_);
}

std::string RejectCommand::describe() const 
{
    return "Workflow " + std::to_string(workflowId_) + ": rejected by participant " +
           std::to_string(actorId_) + " (" + reason_ + ")";
}
