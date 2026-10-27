#ifndef REJECTCOMMAND_H
#define REJECTCOMMAND_H

#include "WorkflowAction.h"

/**  Reject a request that is awaiting approval, with a reason. Not undoable. */
class RejectCommand : public WorkflowAction 
{
public:
    RejectCommand(WorkflowEngine& engine, int workflowId, int actorId, const std::string& reason);
    bool execute();
    std::string describe() const;

private:
    std::string reason_;
};

#endif
