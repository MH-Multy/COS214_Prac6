#include "ResumeCommand.h"
#include "WorkflowInstance.h"

ResumeCommand::ResumeCommand(WorkflowEngine& engine, int workflowId, int actorId)
    : WorkflowAction(engine, workflowId, actorId) {}

bool ResumeCommand::execute() 
{
    std::shared_ptr<WorkflowInstance> wf = target();
    return wf && wf->resume(actorId_);
}

bool ResumeCommand::undo() 
{
    std::shared_ptr<WorkflowInstance> wf = target();
    return wf && wf->suspend(actorId_);
}

std::string ResumeCommand::describe() const 
{
    return "Workflow " + std::to_string(workflowId_) + ": resumed by participant " +
           std::to_string(actorId_);
}
