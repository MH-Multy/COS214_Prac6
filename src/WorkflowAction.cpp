#include "WorkflowAction.h"
#include "WorkflowEngine.h"
#include "WorkflowInstance.h"

WorkflowAction::WorkflowAction(WorkflowEngine& engine, int workflowId, int actorId)
    : engine_(engine), workflowId_(workflowId), actorId_(actorId) {}

bool WorkflowAction::undo() { return false; }

std::shared_ptr<WorkflowInstance> WorkflowAction::target() const 
{
    return engine_.getInstance(workflowId_);
}
