#ifndef WORKFLOWMEDIATOR_H
#define WORKFLOWMEDIATOR_H

class ValidationResult;

/**
 * Mediator interface coordinating validation, approval and
 *        notification for a running workflow (see class diagram).
 *
 * The Facade only needs requestValidation(); the Mediator owner provides
 * DefaultWorkflowMediator.
 */
class WorkflowMediator 
{
public:
    virtual ~WorkflowMediator() {}
    virtual void requestValidation(int workflowId) = 0;
    virtual void validationCompleted(int workflowId, const ValidationResult& result) = 0;
    virtual void requestApproval(int workflowId) = 0;
    virtual void approvalCompleted(int workflowId, bool approved) = 0;
};

#endif
