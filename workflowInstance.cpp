#include "WorkflowInstance.h"

WorkflowInstance::WorkflowInstance() : currentState(new PendingState{}){} // Default by start is Pending state

WorkflowInstance::~WorkflowInstance() {}

void WorkflowInstance::setState(WorkItemState* newState)
{
    currentState.reset(newState); // Deletes the current pointer and assigns it the new pointer
}

std::string WorkflowInstance::getStateName() const
{
    return currentState->getStateName();
}

void WorkflowInstance::pending()
{
    currentState->pending(*this);
}

void WorkflowInstance::accepted()   
{
    currentState->accepted(*this);
}

void WorkflowInstance::rejected()
{
    currentState->rejected(*this);
}

void WorkflowInstance::approved()
{
    currentState->approved(*this);
}

void WorkflowInstance::denied()
{
    currentState->denied(*this);
}

void WorkflowInstance::completed()
{
    currentState->completed(*this);
}

void WorkflowInstance::suspended()
{
    currentState->suspended(*this);
}
