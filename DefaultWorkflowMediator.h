/**
 * @file DefaultWorkflowMediator.h
 * @brief Concrete Mediator: submit -> validate -> decide -> notify.
 */
#ifndef DEFAULTWORKFLOWMEDIATOR_H
#define DEFAULTWORKFLOWMEDIATOR_H

#include "WorkflowColleague.h"
#include "WorkflowMediator.h"

class WorkflowEngine;

/**
 * @brief Concrete Mediator: owns the three colleagues and decides what each does next.
 * @ingroup mediator
 *
 * 1. ValidationService runs the chain -> validationCompleted().
 * 2. Valid: the instance is validated (Accepted), people are told, and ApprovalManager
 *    applies the policy (possibly approving automatically). Invalid: it is rejected.
 * 3. A decision -> approvalCompleted() -> the requester (and, if automatic, the approvers) are told.
 */
class DefaultWorkflowMediator : public WorkflowMediator {
public:
    explicit DefaultWorkflowMediator(WorkflowEngine& engine);
    void requestValidation(int workflowId) override;
    void validationCompleted(int workflowId, const ValidationResult& result) override;
    void requestApproval(int workflowId) override;
    void approvalCompleted(int workflowId, bool approved) override;
    int processTimeouts() override;
    std::vector<std::string> getNotifications(int participantId) const override;
    const NotificationService& getNotifier() const;

private:
    DefaultWorkflowMediator(const DefaultWorkflowMediator&);              // colleagues point back at this object
    DefaultWorkflowMediator& operator=(const DefaultWorkflowMediator&);

    WorkflowEngine& engine;
    ValidationService validator;
    ApprovalManager approvals;
    NotificationService notifier;
};

#endif
