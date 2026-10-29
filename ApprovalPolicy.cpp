#include "ApprovalPolicy.h"

ApprovalPolicy::~ApprovalPolicy(){}

TimeoutApprovalPolicy::TimeoutApprovalPolicy(int seconds) : timeoutSeconds(seconds){}

bool TimeoutApprovalPolicy::shouldAutoApprove(const WorkflowInstance& wf) const
{
    // Automatic approval only applies to Accepted work items.
    if (wf.getStateName() != "Accepted")
    {
        return false;
    }

    // Get the current time.
    auto currentTime = std::chrono::steady_clock::now();

    // Calculate how much time has passed since bot acceptance. Need to add this to workinstance
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(currentTime - wf.getAcceptedAt());

    // Automatically approve if the timeout has elapsed.
    return (elapsed.count() >= timeoutSeconds);
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