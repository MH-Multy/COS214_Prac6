#ifndef COMPLETEWORKITEMCOMMAND_H
#define COMPLETEWORKITEMCOMMAND_H

#include "WorkflowAction.h"

/**  Mark one work item as done. Undo reopens it. */
class CompleteWorkItemCommand : public WorkflowAction 
{
public:
    CompleteWorkItemCommand(WorkflowEngine& engine, int workflowId, int itemId, int actorId);
    bool execute();
    bool undo();
    std::string describe() const;

private:
    int itemId_;
};

#endif
