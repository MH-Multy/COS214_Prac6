/**
 * @file WorkflowInstance.h
 * @brief One independent run of a workflow definition.
 */
#ifndef WORKFLOWINSTANCE_H
#define WORKFLOWINSTANCE_H

#include <chrono>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ReportHistory.h"
#include "WorkflowEvent.h"
#include "WorkflowReport.h"
#include "WorkflowRequest.h"
#include "WorkflowSubject.h"
#include "workflowState.h"

class ApprovalPolicy;
class CompositeWorkItem;
class Participant;
class WorkflowDefinition;
class WorkItem;

/**
 * @brief One independent run of a workflow definition, e.g. one researcher's test request.
 * @ingroup core
 *
 * Each instance owns its own request data, state, work tree, approval policy,
 * history and report, so actions on one instance never affect another.
 *
 * Pattern roles: State Context (lifecycle changes are delegated to the current
 * WorkItemState), Strategy Context (its own ApprovalPolicy), Observer Subject
 * (every event is broadcast), Command Receiver, and Iterator/Memento client.
 *
 * Every action returns false, and changes nothing, if it is not allowed in the
 * current state or the participant is not permitted. actorId -1 means "the
 * system" (automatic validation or approval).
 */
class WorkflowInstance : public WorkflowSubject
{
public:
    /**
     * @param id           Unique id from the engine.
     * @param definition   The workflow type.
     * @param request      This run's request data (copied).
     * @param root         Work tree from the factory (an empty tree if not given).
     * @param policy       Approval policy from the factory (strict if not given).
     * @param participants The engine's participants, for role checks and routing.
     *                     If null (isolated unit tests), role checks are skipped.
     */
    WorkflowInstance(int id, std::shared_ptr<const WorkflowDefinition> definition,
                     const WorkflowRequest& request,
                     std::shared_ptr<CompositeWorkItem> root = std::shared_ptr<CompositeWorkItem>(),
                     std::shared_ptr<ApprovalPolicy> policy = std::shared_ptr<ApprovalPolicy>(),
                     std::map<int, Participant>* participants = 0);

    int getId() const { return id; }
    std::shared_ptr<const WorkflowDefinition> getDefinition() const { return definition; }
    const WorkflowRequest& getRequest() const { return request; }
    std::shared_ptr<CompositeWorkItem> getRoot() const { return root; }
    std::shared_ptr<ApprovalPolicy> getApprovalPolicy() const { return approvalPolicy; }
    std::chrono::steady_clock::time_point getAcceptedAt() const { return acceptedAt; }
    /** @brief A participant's name, or "#id" if unknown. */
    std::string participantName(int participantId) const;

    // ---- lifecycle (delegated to the State)
    /** @brief Validation passed: Pending -> Accepted. */
    bool validate();
    /** @brief Accepted -> Approved, then work starts. @param actorId Approver, or -1 for automatic approval. */
    bool approve(int actorId);
    /** @brief actorId -1: validation failed, Pending -> Rejected. Otherwise a person denies it: Accepted -> Denied. */
    bool reject(int actorId, const std::string& reason);
    /** @brief Release the work that is now allowed and route it to people (only while Approved). */
    bool start();
    /** @brief Approved -> Suspended. @param actorId Must hold the approver role. */
    bool suspend(int actorId);
    /** @brief Suspended -> Approved. @param actorId Must hold the approver role. */
    bool resume(int actorId);
    /** @brief Approved -> Completed, once every task is done. */
    bool complete();

    // ---- work
    /** @brief Complete a task (only its assignee, only while Approved); releases the next task and completes the workflow when nothing is left. */
    bool completeItem(int itemId, int actorId);
    /** @brief Reopen a completed task (used to undo a completion). */
    bool reopenItem(int itemId, int actorId);
    std::vector<std::shared_ptr<WorkItem>> getAvailableWork() const;
    std::shared_ptr<WorkItem> findItem(int itemId) const;
    bool allWorkComplete() const;

    // ---- State context
    /** @brief Replace the current state; called by the state objects (ownership is taken). */
    void setState(WorkItemState* next);
    std::string getStateName() const;

    // ---- Strategy context
    bool canApprove(const Participant& actor) const;
    bool shouldAutoApprove() const;

    // ---- history (Observer subject) and report (Memento)
    /** @brief Record an event and broadcast it to the observers. */
    void addEvent(const WorkflowEvent& e);
    const std::vector<WorkflowEvent>& getHistory() const { return history; }
    WorkflowReport& getReport() { return report; }
    const WorkflowReport& getReport() const { return report; }
    ReportHistory& getReportHistory() { return reportHistory; }
    const ReportHistory& getReportHistory() const { return reportHistory; }

private:
    bool actorHasRole(int actorId, const std::string& role) const;
    Participant* findParticipant(int participantId) const;
    void assignWork();

    int id;
    std::shared_ptr<const WorkflowDefinition> definition;
    WorkflowRequest request;
    std::shared_ptr<CompositeWorkItem> root;
    std::unique_ptr<WorkItemState> currentState;
    std::shared_ptr<ApprovalPolicy> approvalPolicy;
    WorkflowReport report;
    std::vector<WorkflowEvent> history;
    ReportHistory reportHistory;
    std::map<int, Participant>* participants;
    std::chrono::steady_clock::time_point acceptedAt;
};

#endif
