#ifndef WORKFLOWACTION_H
#define WORKFLOWACTION_H

#include <memory>
#include <string>

class WorkflowEngine;
class WorkflowInstance;

/**
 *  Command: an action a participant performs on one workflow instance.
 *
 * Turning each action into an object lets the CommandInvoker keep a history
 * of what was done and undo it where that makes sense. The engine, workflow
 * id and actor id are shared by every command, so they live here once
 * (generalisation) and subclasses only add their own data.
 *
 * execute() and describe() must be provided by every command. undo() returns
 * false by default because not every action may be reversed: an approval or
 * rejection is a recorded decision. Commands whose effect has a natural
 * inverse (suspend/resume, completing an item, editing the report) override it.
 */
class WorkflowAction 
{
public:
    WorkflowAction(WorkflowEngine& engine, int workflowId, int actorId);
    virtual ~WorkflowAction() {}

    /**  Perform the action. @return false if it is not allowed now. */
    virtual bool execute() = 0;

    /**  Reverse the action. @return false if it cannot be reversed. */
    virtual bool undo();

    /**  Readable one-line description for the action history. */
    virtual std::string describe() const = 0;

    int getWorkflowId() const { return workflowId_; }
    int getActorId() const { return actorId_; }

protected:
    /**  The receiver: the instance this command acts on (nullptr if it does not exist). */
    std::shared_ptr<WorkflowInstance> target() const;

    WorkflowEngine& engine_;
    int workflowId_;
    int actorId_;
};

#endif
