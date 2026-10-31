/**
 * @file WorkflowColleague.h
 * @brief Mediator colleagues: validation, approval and notification services.
 */
#ifndef WORKFLOWCOLLEAGUE_H
#define WORKFLOWCOLLEAGUE_H

#include <map>
#include <string>
#include <vector>

class WorkflowEngine;
class WorkflowInstance;
class WorkflowMediator;

/** @brief Mediator Colleague base: a service that reports to the mediator. @ingroup mediator */
class WorkflowColleague {
public:
    WorkflowColleague();
    virtual ~WorkflowColleague();
    void setMediator(WorkflowMediator* m);

protected:
    WorkflowMediator* mediator;
};

/** @brief Runs a definition's validation chain on a new request and reports the result. @ingroup mediator */
class ValidationService : public WorkflowColleague {
public:
    void validate(WorkflowInstance& wf);
};

/** @brief Applies each instance's approval policy, including time limits. @ingroup mediator */
class ApprovalManager : public WorkflowColleague {
public:
    explicit ApprovalManager(WorkflowEngine& engine);
    /** @brief Ask for a decision; approves at once if the policy says so. */
    void requestApproval(WorkflowInstance& wf);
    /** @brief Auto-approve every request whose time limit has passed. */
    void checkTimeouts();

private:
    WorkflowEngine& engine;
};

/** @brief Delivers messages to participants (participant id -1 = the approvers). @ingroup mediator */
class NotificationService : public WorkflowColleague {
public:
    void send(int participantId, const std::string& message);
    /** @brief Messages received by one participant (or -1 for the approvers), oldest first. */
    std::vector<std::string> getMessages(int participantId) const;
    int getSentCount() const;

private:
    std::map<int, std::vector<std::string>> inbox;
};

#endif
