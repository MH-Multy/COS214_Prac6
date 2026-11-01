/**
 * @file WorkflowFactory.h
 * @brief Abstract Factory: a matching approval policy and work tree for an approval regime.
 */
#ifndef WORKFLOWFACTORY_H
#define WORKFLOWFACTORY_H

#include <memory>

class ApprovalPolicy;
class CompositeWorkItem;
class WorkflowDefinition;

/**
 * @brief Abstract Factory: creates a matching family of runtime components.
 * @ingroup abstractfactory
 *
 * The family is an ApprovalPolicy plus the instance's work tree. They must
 * match: the manual-review regime never auto-approves and adds a final
 * sign-off by an approver; the auto-approval regime does neither.
 */
class WorkflowFactory {
public:
    virtual ~WorkflowFactory();
    virtual std::shared_ptr<ApprovalPolicy> createApprovalPolicy(const WorkflowDefinition& def) const = 0;
    virtual std::shared_ptr<CompositeWorkItem> createTaskGroup(const WorkflowDefinition& def) const = 0;
};

/** @brief Concrete Factory: timeout auto-approval, the configured stages. @ingroup abstractfactory */
class AutoApprovalComponentFactory : public WorkflowFactory {
public:
    std::shared_ptr<ApprovalPolicy> createApprovalPolicy(const WorkflowDefinition& def) const override;
    std::shared_ptr<CompositeWorkItem> createTaskGroup(const WorkflowDefinition& def) const override;
};

/** @brief Concrete Factory: strict approval, the configured stages plus a final compliance sign-off. @ingroup abstractfactory */
class ManualReviewComponentFactory : public WorkflowFactory {
public:
    std::shared_ptr<ApprovalPolicy> createApprovalPolicy(const WorkflowDefinition& def) const override;
    std::shared_ptr<CompositeWorkItem> createTaskGroup(const WorkflowDefinition& def) const override;
};

#endif
