#ifndef WORKFLOWFACTORY_H
#define WORKFLOWFACTORY_H
#include "standins.h"
/**
 * ABSTRACT FACTORY (v4: red).
 * Problem solved: an approval policy and the shape of the task group must
 * match (manual review needs strict approval AND strictly ordered work).
 * One factory per definition keeps the pair consistent, and the engine never
 * names a concrete policy or group type.
 * Products: ApprovalPolicy (Strategy) and CompositeWorkItem (Composite).
 */
 class  WorkflowFactory{
    public:
 virtual ~WorkflowFactory() {}
    virtual std::shared_ptr<ApprovalPolicy> createApprovalPolicy(const WorkflowDefinition& def) const = 0;
    virtual std::shared_ptr<CompositeWorkItem> createTaskGroup(const WorkflowDefinition& def) const = 0;
    protected:
    /** Builds root -> one group per stage -> one Task per task name. */
    static std::shared_ptr<CompositeWorkItem> 
    buildGroup(const WorkflowDefinition& def, bool tasksSequential);
 };

 /** Family 1: every step needs senior sign-off, 
  * work strictly in order. *//** Family 1: every step needs senior sign-off, work strictly in order. */
  class ManualReviewComponentFactory : public WorkflowFactory {
public:
    std::shared_ptr<ApprovalPolicy> createApprovalPolicy(const WorkflowDefinition& def) const;
    std::shared_ptr<CompositeWorkItem> createTaskGroup(const WorkflowDefinition& def) const;
};

/** Family 2: auto-approves after a timeout, tasks inside a stage may run in parallel. */
class AutoApprovalComponentFactory : public WorkflowFactory {
public:
    std::shared_ptr<ApprovalPolicy> createApprovalPolicy(const WorkflowDefinition& def) const;
    std::shared_ptr<CompositeWorkItem> createTaskGroup(const WorkflowDefinition& def) const;
};
#endif