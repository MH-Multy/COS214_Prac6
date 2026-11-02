#include "SuspendCommand.h"
#include "WorkflowInstance.h"

SuspendCommand::SuspendCommand(WorkflowEngine& engine, int workflowId, int actorId)
    : WorkflowAction(engine, workflowId, actorId) {}

bool SuspendCommand::execute() 
{
    std::shared_ptr<WorkflowInstance> wf = target();
    return wf && wf->suspend(actorId_);
}

bool SuspendCommand::undo() 
{
    std::shared_ptr<WorkflowInstance> wf = target();
    return wf && wf->resume(actorId_);
}

std::string SuspendCommand::describe() const 
{
    return "Workflow " + std::to_string(workflowId_) + ": suspended by participant " +
           std::to_string(actorId_);
}
