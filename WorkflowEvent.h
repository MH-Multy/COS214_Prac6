#ifndef WORKFLOWEVENT_H
#define WORKFLOWEVENT_H

#include <string>

/**
 * One entry in a workflow instance's audit history.
 *
 * Events are immutable once created: the history is a permanent record, so
 * undoing an action adds a new event rather than deleting the old one.
 */
class WorkflowEvent 
{
public:
    WorkflowEvent(const std::string& type, int workflowId, int actorId, const std::string& message);

    std::string getType() const { return type_; }
    int getWorkflowId() const { return workflowId_; }
    int getActorId() const { return actorId_; }
    std::string getMessage() const { return message_; }
    std::string getTimestamp() const { return timestamp_; }

    /** Single-line form for the TUI, e.g. "[12:01:33] APPROVED by 7: ...". */
    std::string toString() const;

private:
    std::string type_;
    int workflowId_;
    int actorId_;
    std::string message_;
    std::string timestamp_;
};

#endif
