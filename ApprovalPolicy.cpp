/**
 * @file ApprovalPolicy.cpp
 * @brief Implementation of the approval policies.
 */
#include "ApprovalPolicy.h"
#include "Participant.h"
#include "WorkflowInstance.h"

ApprovalPolicy::ApprovalPolicy(const string& role) : approverRole(role){}

ApprovalPolicy::~ApprovalPolicy(){}

bool ApprovalPolicy::canApprove(const WorkflowInstance&, const Participant& actor) const
{
    return actor.hasRole(approverRole);
}

TimeoutApprovalPolicy::TimeoutApprovalPolicy(const string& role, int seconds)
    : ApprovalPolicy(role), timeoutSeconds(seconds){}

bool TimeoutApprovalPolicy::shouldAutoApprove(const WorkflowInstance& wf) const
{
    // Automatic approval only applies to Accepted work items.
    if (wf.getStateName() != "Accepted")
    {
        return false;
    }

    // Get the current time.
    auto currentTime = std::chrono::steady_clock::now();

    // Calculate how much time has passed since bot acceptance.
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(currentTime - wf.getAcceptedAt());

    // Automatically approve if the timeout has elapsed.
    return (elapsed.count() >= timeoutSeconds);
}

string TimeoutApprovalPolicy::describe() const
{
    return "Auto-approves after " + to_string(timeoutSeconds) + " s without a decision";
}

StrictApprovalPolicy::StrictApprovalPolicy(const string& role) : ApprovalPolicy(role){}

bool StrictApprovalPolicy::canApprove(const WorkflowInstance& wf, const Participant& actor) const
{
    // Four-eyes principle: nobody approves their own request.
    return ApprovalPolicy::canApprove(wf, actor) && actor.getId() != wf.getRequest().getRequesterId();
}

bool StrictApprovalPolicy::shouldAutoApprove(const WorkflowInstance& wf) const
{
    if (wf.getStateName() != "Accepted")
    {
        return false;
    }
    // Strict approval always requires human approval.
    return false;
}

string StrictApprovalPolicy::describe() const
{
    return "Manual approval only (four-eyes)";
}
