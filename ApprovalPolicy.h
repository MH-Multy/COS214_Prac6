/**
 * @file ApprovalPolicy.h
 * @brief Strategy: who may approve a request, and whether it is approved automatically.
 */
#ifndef APPROVALPOLICY_H
#define APPROVALPOLICY_H

#include <chrono>
#include <string>
class WorkflowInstance;
class Participant;
using namespace std;

/**
 * @brief Strategy: decides who may approve and whether to approve automatically.
 * @ingroup strategy
 *
 * Each workflow instance holds its own policy, so a lab request and a loan
 * application follow different rules at the same time.
 */
class ApprovalPolicy
{
    protected:
        string approverRole;   ///< Role allowed to decide.

    public:
        explicit ApprovalPolicy(const string& approverRole);
        virtual ~ApprovalPolicy();
        /** @brief Whether @p actor may approve or deny @p wf (holds the approver role). */
        virtual bool canApprove(const WorkflowInstance& wf, const Participant& actor) const;
        /** @brief Whether @p wf should be approved without a person deciding. */
        virtual bool shouldAutoApprove(const WorkflowInstance& wf) const = 0;
        /** @brief The rule in words, for the user interface. */
        virtual string describe() const = 0;
};

// --------- Concrete Strategy --------------

/** @brief Auto-approve if nobody decides within a time limit (used for lab tests). @ingroup strategy */
class TimeoutApprovalPolicy : public ApprovalPolicy
{
    private:
        int timeoutSeconds;

    public:
        TimeoutApprovalPolicy(const string& approverRole, int seconds);
        bool shouldAutoApprove(const WorkflowInstance& wf) const override;
        string describe() const override;
};

/** @brief Never auto-approve; nobody may approve their own request (used for loans). @ingroup strategy */
class StrictApprovalPolicy : public ApprovalPolicy
{
    public:
        explicit StrictApprovalPolicy(const string& approverRole);
        bool canApprove(const WorkflowInstance& wf, const Participant& actor) const override;
        bool shouldAutoApprove(const WorkflowInstance& wf) const override;
        string describe() const override;
};

#endif //APPROVALPOLICY_H
