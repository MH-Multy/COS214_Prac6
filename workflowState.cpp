#include "workflowState.h"

// -------- WorkItemState ------
WorkItemState::~WorkItemState(){}

// All by defalut call the error if they are not overriden, therefor it does not need manual checking of states
// as the classes do not have overrides for them
void WorkItemState::illegalState(WorkflowInstance& workflow, string& newState)
{
    cout << "\nError <illegal state change>: Cannot change from '" << workflow.getStateName() << "' to '" << newState <<"' \n";
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

void WorkItemState::denied(WorkflowInstance& workflow) const 
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

void AcceptedState::denied(WorkflowInstance& workflow) const
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

// --------- DeniedSate -------------
string DeniedSate::getStateName() const
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