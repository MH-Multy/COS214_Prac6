#ifndef RESUMECOMMAND_H
#define RESUMECOMMAND_H

#include "WorkflowAction.h"

/** @brief Resume a suspended workflow. Undo suspends it again. */
class ResumeCommand : public WorkflowAction 
{
public:
    ResumeCommand(WorkflowEngine& engine, int workflowId, int actorId);
    bool execute();
    bool undo();
    std::string describe() const;
};

#endif
