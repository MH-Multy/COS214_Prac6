#ifndef WORKFLOWINSTANCE_H
#define WORKFLOWINSTANCE_H

#include <memory>
#include <string>
#include "workflowState.h"
#include "ApprovalPolicy.h"

// THIS IS NOT COMPLETE
// I did it for what was needed in State design

class WorkflowInstance
{
    private:
        std::unique_ptr<WorkItemState> currentState;
    public:
        WorkflowInstance();
        ~WorkflowInstance();

        // The context owns the current state.
        void setState(WorkItemState* newState);

        // Gets the state name...
        std::string getStateName() const;

        // Actions delegated to the current state.
        void pending();
        void accepted();
        void rejected();
        void approved();
        void denied();
        void completed();
        void suspended();
};

#endif // WORKFLOWINSTANCE_H