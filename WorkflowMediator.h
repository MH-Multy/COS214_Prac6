/**
 * @file WorkflowMediator.h
 * @brief Mediator interface coordinating validation, approval and notification.
 */
#ifndef WORKFLOWMEDIATOR_H
#define WORKFLOWMEDIATOR_H

#include <string>
#include <vector>

class ValidationResult;

/**
 * @brief Mediator: validation, approval and notification never call each other directly;
 *        they report here and the mediator decides what happens next.
 * @ingroup mediator
 */
class WorkflowMediator {
public:
    virtual ~WorkflowMediator() {}
    virtual void requestValidation(int workflowId) = 0;
    virtual void validationCompleted(int workflowId, const ValidationResult& result) = 0;
    virtual void requestApproval(int workflowId) = 0;
    virtual void approvalCompleted(int workflowId, bool approved) = 0;
    /** @brief Apply the approval time limits. @return How many requests were auto-approved. */
    virtual int processTimeouts() = 0;
    /** @brief Messages sent to one participant (-1 = the approvers), oldest first. */
    virtual std::vector<std::string> getNotifications(int participantId) const = 0;
};

#endif
