#ifndef WORKFLOWMEDIATOR_H
#define WORKFLOWMEDIATOR_H
#include "WorkflowEngine.h"

/**
 
 * Problem solved: validation, approval and notification must happen in a
 * fixed order, but ValidationService, ApprovalManager and NotificationService
 * must not know about each other. They only talk to the mediator, which
 * drives the WorkflowInstance through its states.
 */
class WorkflowMediator {
public:
    virtual ~WorkflowMediator() {}
    virtual void requestValidation(int workflowId) = 0;
    virtual void validationCompleted(int workflowId, const ValidationResult& result) = 0;
    virtual void requestApproval(int workflowId) = 0;
    virtual void approvalCompleted(int workflowId, bool approved) = 0;
};

/** Colleague base (v4: WorkflowColleague). */
class WorkflowColleague {
public:
    WorkflowColleague();
    virtual ~WorkflowColleague() {}
    void setMediator(WorkflowMediator* m);
protected:
    WorkflowMediator* mediator;
};

/** Runs the definition's Chain of Responsibility, reports back to the mediator. */
class ValidationService : public WorkflowColleague {
public:
    void validate(WorkflowInstance& wf);
};

/** Asks the instance's ApprovalPolicy (Strategy) whether approval is automatic. */
class ApprovalManager : public WorkflowColleague {
public:
    void requestApproval(WorkflowInstance& wf);
    /** Call periodically (the engine's clock). */
    void checkTimeouts();
private:
    std::vector<WorkflowInstance*> pending;
};

class NotificationService : public WorkflowColleague {
public:
    void send(int participantId, const std::string& message);
    const std::vector<std::string>& getLog() const;
private:
    std::vector<std::string> log;
};

/** Concrete mediator (v4: DefaultWorkflowMediator). */
class DefaultWorkflowMediator : public WorkflowMediator {
public:
    static const int SYSTEM_ACTOR = 0;
    DefaultWorkflowMediator(WorkflowEngine& e, ValidationService& v, ApprovalManager& a, NotificationService& n);
    void requestValidation(int id);
    void validationCompleted(int id, const ValidationResult& r);
    void requestApproval(int id);
    void approvalCompleted(int id, bool approved);
private:
    WorkflowEngine& engine;
    ValidationService& validator;
    ApprovalManager& approvals;
    NotificationService& notifier;
};
#endif
