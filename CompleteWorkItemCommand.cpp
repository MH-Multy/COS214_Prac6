#include "CompleteWorkItemCommand.h"
#include "WorkflowInstance.h"

CompleteWorkItemCommand::CompleteWorkItemCommand(WorkflowEngine& engine, int workflowId,
                                                 int itemId, int actorId)
    : WorkflowAction(engine, workflowId, actorId), itemId_(itemId) {}

bool CompleteWorkItemCommand::execute() 
{
    std::shared_ptr<WorkflowInstance> wf = target();

    return wf && wf->completeItem(itemId_, actorId_);
}

bool CompleteWorkItemCommand::undo() 
{
    std::shared_ptr<WorkflowInstance> wf = target();
    return wf && wf->reopenItem(itemId_, actorId_);
}

std::string CompleteWorkItemCommand::describe() const 
{
    return "Workflow " + std::to_string(workflowId_) + ": item " + std::to_string(itemId_) +
           " completed by participant " + std::to_string(actorId_);
}
