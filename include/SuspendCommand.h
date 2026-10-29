#ifndef SUSPENDCOMMAND_H
#define SUSPENDCOMMAND_H

#include "WorkflowAction.h"

/**  Suspend a workflow. Undo resumes it. */
class SuspendCommand : public WorkflowAction 
{
public:
    SuspendCommand(WorkflowEngine& engine, int workflowId, int actorId);
    bool execute();
    bool undo();
    std::string describe() const;
};

#endif
