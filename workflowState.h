#ifndef WORKFLOWSTATE_H
#define WORKFLOWSTATE_H

#include <string>
using namespace std;

// As this class isn't made yet, we change what it is called later
class WorkflowInstance;

// --------------------- Base State -----------------------
class WorkItemState
{
    public:
        virtual ~WorkItemState();
        // Will return a string of the item's current state
        virtual string getStateName() const = 0; // changed function name from "getName" to make more sense

        // changed from bool to void as they do not need to return and made them const as they won't change
        // Also made them virtual
        virtual void pending(WorkflowInstance& workflow) const; // Added: Pending for while they first submit their request and REMOVED "start". This is the default state when they begin
        virtual void accepted(WorkflowInstance& workflow) const; // Added: Bot checked request and accepted, now needs human approval
        virtual void rejected(WorkflowInstance& workflow) const; // Bot checked request and rejects
        virtual void approved(WorkflowInstance& workflow) const; // Human checks and approves or after some time passes (we will explain this time thing not code it)
        virtual void denied(WorkflowInstance& workflow) const; // Human chceks and denies the project
        virtual void completed(WorkflowInstance& workflow) const ; // Added: Project is complete
        virtual void suspended(WorkflowInstance& workflow) const; // Added: Project can be suspended only after it was approved by human
};


// ---------------- Concrete States ----------------------

// Pending can only be accepted or rejected
class PendingState : public WorkItemState
{
    public:
        string getStateName() const override;
        void accepted(WorkflowInstance& workflow) const override;
        void rejected(WorkflowInstance& workflow) const override;
};

// Accept can only be approved or denied
class AcceptedState : public WorkItemState
{
    public:
        string getStateName() const override;
        void approved(WorkflowInstance& workflow) const override;
        void denied(WorkflowInstance& workflow) const override;
};

// Approved can only be completed or suspended
class ApprovedState : public WorkItemState
{
    public:
        string getStateName() const override;
        void completed(WorkflowInstance& workflow) const override;
        void suspended(WorkflowInstance& workflow) const override;
};

class RejectedState : public WorkItemState
{
    public:
        string getStateName() const override;
};

class DeniedState : public WorkItemState
{
    public:
        string getStateName() const override;
};


#endif //WORKFLOWSTATE_H