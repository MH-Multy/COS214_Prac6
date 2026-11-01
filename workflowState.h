/**
 * @file workflowState.h
 * @brief State pattern: the lifecycle of a workflow instance.
 */
#ifndef WORKFLOWSTATE_H
#define WORKFLOWSTATE_H

#include <string>
using namespace std;

class WorkflowInstance;

/**
 * @brief State: decides which lifecycle changes are legal for a workflow instance.
 * @ingroup state
 *
 *     Pending --accepted--> Accepted --approved--> Approved --completed--> Completed
 *        |                     |                    |   ^
 *        +--rejected--> Rejected +--denied--> Denied  suspended  accepted (resume)
 *                                                    v   |
 *                                                  Suspended
 *
 * Every transition is virtual and, by default, calls illegalState(). A concrete
 * state only overrides the transitions it allows, so illegal changes are
 * refused without any if/else or switch on the state.
 */
// --------------------- Base State -----------------------
class WorkItemState
{
    public:
        virtual ~WorkItemState();
        // Will return a string of the item's current state
        virtual string getStateName() const = 0;

        // By default every transition is illegal; concrete states override the ones they allow.
        virtual void pending(WorkflowInstance& workflow) const;   // the default state when a request is submitted
        virtual void accepted(WorkflowInstance& workflow) const;  // bot checked the request and accepted it (or: resume after a suspension)
        virtual void rejected(WorkflowInstance& workflow) const;  // bot checked the request and rejected it
        virtual void approved(WorkflowInstance& workflow) const;  // a person approved it, or the time limit passed
        virtual void denied(WorkflowInstance& workflow, const string& message) const;  // a person denied it
        virtual void completed(WorkflowInstance& workflow) const; // all work is done
        virtual void suspended(WorkflowInstance& workflow) const; // can only be suspended after it was approved

        // Shows and stops illegal state changes (like from Pending to Suspended): the attempt is
        // recorded in the instance's history and the state does not change.
        void illegalState(WorkflowInstance& workflow, const string& newState) const;
};


// ---------------- Concrete States ----------------------

/** @brief Submitted; waiting for automated validation. Can only be accepted or rejected. @ingroup state */
class PendingState : public WorkItemState
{
    public:
        string getStateName() const override;
        void accepted(WorkflowInstance& workflow) const override;
        void rejected(WorkflowInstance& workflow) const override;
};

/** @brief Passed validation; waiting for a person. Can only be approved or denied. @ingroup state */
class AcceptedState : public WorkItemState
{
    public:
        string getStateName() const override;
        void approved(WorkflowInstance& workflow) const override;
        void denied(WorkflowInstance& workflow, const string& message) const override;
};

/** @brief Approved; work is in progress. Can only be completed or suspended. @ingroup state */
class ApprovedState : public WorkItemState
{
    public:
        string getStateName() const override;
        void completed(WorkflowInstance& workflow) const override;
        void suspended(WorkflowInstance& workflow) const override;
};

/** @brief Failed validation (end state). @ingroup state */
class RejectedState : public WorkItemState
{
    public:
        string getStateName() const override;
};

/** @brief Turned down by an approver (end state). @ingroup state */
class DeniedState : public WorkItemState
{
    public:
        string getStateName() const override;
};

/** @brief All work done (end state). @ingroup state */
class CompletedState : public WorkItemState
{
    public:
        string getStateName() const override;
};

/** @brief Halted by an approver; accepted() resumes it to Approved. @ingroup state */
class SuspendedState : public WorkItemState
{
    public:
        string getStateName() const override;
        void accepted(WorkflowInstance& workflow) const override;
};


#endif //WORKFLOWSTATE_H
