#ifndef APPROVECOMMAND_H
#define APPROVECOMMAND_H

#include "WorkflowAction.h"

/** Approve a request that is awaiting approval. Not undoable: approval is a recorded decision. */
class ApproveCommand : public WorkflowAction 
{
public:
    ApproveCommand(WorkflowEngine& engine, int workflowId, int actorId);
    bool execute();
    std::string describe() const;
};

#endif
