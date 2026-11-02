/**
 * @file workflowState.cpp
 * @brief Implementation of the lifecycle states.
 */
#include "workflowState.h"
#include "WorkflowInstance.h"
#include "WorkflowEvent.h"

// -------- WorkItemState ------
WorkItemState::~WorkItemState(){}

// All transitions call this by default if they are not overridden, so no manual checking of
// states is needed: a state simply does not override the transitions it does not allow.
void WorkItemState::illegalState(WorkflowInstance& workflow, const string& newState) const
{
    workflow.addEvent(WorkflowEvent("ILLEGAL_STATE", workflow.getId(), -1,
        "Cannot change from '" + getStateName() + "' to '" + newState + "'"));
}

void WorkItemState::pending(WorkflowInstance& workflow) const
{
    illegalState(workflow, "Pending");
}

void WorkItemState::accepted(WorkflowInstance& workflow) const
{
    illegalState(workflow, "Accepted");
}

void WorkItemState::rejected(WorkflowInstance& workflow) const
{
    illegalState(workflow, "Rejected");
}

void WorkItemState::approved(WorkflowInstance& workflow) const
{
    illegalState(workflow, "Approved");
}

void WorkItemState::denied(WorkflowInstance& workflow, const string&) const
{
    illegalState(workflow, "Denied");
}

void WorkItemState::completed(WorkflowInstance& workflow) const
{
    illegalState(workflow, "Completed");
}

void WorkItemState::suspended(WorkflowInstance& workflow) const
{
    illegalState(workflow, "Suspended");
}

// Note: setState() replaces (and deletes) the current state object, so each transition
// below does nothing else after calling it.

// -------- PendingState -------
string PendingState::getStateName() const
{
    return "Pending";
}

void PendingState::accepted(WorkflowInstance& workflow) const
{
    workflow.setState(new AcceptedState{});
}

void PendingState::rejected(WorkflowInstance& workflow) const
{
    workflow.setState(new RejectedState{});
}

// --------- AcceptedState --------
string AcceptedState::getStateName() const
{
    return "Accepted";
}

void AcceptedState::approved(WorkflowInstance& workflow) const
{
    workflow.setState(new ApprovedState{});
}

void AcceptedState::denied(WorkflowInstance& workflow, const string&) const
{
    workflow.setState(new DeniedState{});
}

// --------- ApprovedState ----------
string ApprovedState::getStateName() const
{
    return "Approved";
}

void ApprovedState::completed(WorkflowInstance& workflow) const
{
    workflow.setState(new CompletedState{});
}

void ApprovedState::suspended(WorkflowInstance& workflow) const
{
    workflow.setState(new SuspendedState{});
}

// --------- RejectedState -----------
string RejectedState::getStateName() const
{
    return "Rejected";
}

// --------- DeniedState -------------
string DeniedState::getStateName() const
{
    return "Denied";
}

// --------- CompletedState --------
string CompletedState::getStateName() const
{
    return "Completed";
}

// --------- SuspendedState --------
string SuspendedState::getStateName() const
{
    return "Suspended";
}

void SuspendedState::accepted(WorkflowInstance& workflow) const
{
    // Resume: the suspension is lifted and work continues.
    workflow.setState(new ApprovedState{});
}
