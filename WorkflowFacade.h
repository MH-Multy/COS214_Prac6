/**
 * @file WorkflowFacade.h
 * @brief Facade: the single entry point the user interface uses to drive TIVIDY.
 */
#ifndef WORKFLOWFACADE_H
#define WORKFLOWFACADE_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Participant.h"
#include "WorkflowEvent.h"
#include "WorkflowRequest.h"

class ApproverObserver;
class AuditLogger;
class CommandInvoker;
class WorkflowAction;
class WorkflowDefinition;
class WorkflowEngine;
class WorkflowInstance;
class WorkflowMediator;
class WorkItem;

/**
 * @brief Facade: the single entry point the TUI uses to drive TIVIDY.
 * @ingroup facade
 *
 * The TUI never creates commands or changes instances itself. Industries are not
 * hard-coded: configuration calls registerWorkflow() for each workflow type.
 * Every operation that changes a workflow creates the matching Command and runs
 * it through the CommandInvoker, which is why undoLastAction() works.
 *
 * Errors: an unknown workflow id or type throws std::invalid_argument; an action
 * that is not allowed right now returns false and changes nothing.
 */
class WorkflowFacade {
public:
    /** @throws std::invalid_argument if any collaborator is null. */
    WorkflowFacade(std::shared_ptr<WorkflowEngine> engine,
                   std::shared_ptr<CommandInvoker> invoker,
                   std::shared_ptr<WorkflowMediator> mediator);

    // ---- configuration
    void registerWorkflow(std::shared_ptr<const WorkflowDefinition> def);
    std::vector<std::string> listWorkflowTypes() const;
    /** @throws std::invalid_argument if @p type was never registered. */
    std::shared_ptr<const WorkflowDefinition> getDefinition(const std::string& type) const;
    void registerParticipant(const Participant& p);
    std::vector<Participant> listParticipants() const;
    std::string participantName(int participantId) const;

    // ---- actions
    /** @brief Start an instance, then validate it and request a decision. @throws std::invalid_argument if @p type is unknown. */
    int submitRequest(const std::string& type, const WorkflowRequest& request);
    bool approveRequest(int workflowId, int actorId);
    bool rejectRequest(int workflowId, int actorId, const std::string& reason);
    bool suspendRequest(int workflowId, int actorId);
    bool resumeRequest(int workflowId, int actorId);
    bool completeWorkItem(int workflowId, int itemId, int actorId);
    bool undoLastAction(int workflowId);
    /** @brief Whether @p actorId may decide on @p workflowId right now. */
    bool canApprove(int workflowId, int actorId) const;
    /** @brief Apply approval time limits. @return How many requests were auto-approved. */
    int processTimeouts();

    // ---- report (Memento)
    bool updateReport(int workflowId, int actorId, const std::string& text);
    std::string getReport(int workflowId) const;
    std::vector<std::string> getReportRevisions(int workflowId) const;
    bool restoreReport(int workflowId, int index, int actorId);

    // ---- inspecting progress
    std::vector<int> listWorkflows() const;
    std::string getStatus(int workflowId) const;
    /** @brief Read-only view of an instance, for display. */
    std::shared_ptr<const WorkflowInstance> getWorkflow(int workflowId) const;
    /** @brief Tasks that can be done right now (empty unless the workflow is Approved). */
    std::vector<std::shared_ptr<WorkItem>> getAvailableWork(int workflowId) const;
    std::vector<WorkflowEvent> getHistory(int workflowId) const;
    std::vector<std::string> getRequesterInbox(int workflowId) const;
    std::vector<std::string> getNotifications(int participantId) const;
    std::vector<std::string> getAuditLog() const;
    std::vector<std::string> getApproverQueue() const;
    std::vector<std::string> getActionHistory() const;

private:
    std::shared_ptr<WorkflowInstance> requireInstance(int workflowId) const;
    bool run(int workflowId, std::shared_ptr<WorkflowAction> cmd);

    std::shared_ptr<WorkflowEngine> engine;
    std::shared_ptr<CommandInvoker> invoker;
    std::shared_ptr<WorkflowMediator> mediator;
    std::shared_ptr<AuditLogger> audit;
    std::shared_ptr<ApproverObserver> approvers;
    std::map<std::string, std::shared_ptr<const WorkflowDefinition>> definitions;
};

#endif
