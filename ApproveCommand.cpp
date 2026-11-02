#include "ApproveCommand.h"
#include "WorkflowInstance.h"
// Approval is undoable
ApproveCommand::ApproveCommand(WorkflowEngine& engine, int workflowId, int actorId)
    : WorkflowAction(engine, workflowId, actorId) {}

bool ApproveCommand::execute() 
{
    std::shared_ptr<WorkflowInstance> wf = target();
    return wf && wf->approve(actorId_);
}

std::string ApproveCommand::describe() const 
{
    return "Workflow " + std::to_string(workflowId_) + ": approved by participant " +
           std::to_string(actorId_);
}
