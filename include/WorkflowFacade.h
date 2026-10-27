#ifndef WORKFLOWFACADE_H
#define WORKFLOWFACADE_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "WorkflowEvent.h"
#include "WorkflowRequest.h"

class CommandInvoker;
class WorkflowAction;
class WorkflowDefinition;
class WorkflowEngine;
class WorkflowInstance;
class WorkflowMediator;
class WorkItem;

/**
 * @brief Facade: the single entry point the TUI uses to drive TIVIDY.
 *
 * The TUI never touches the engine, instances, commands or mediator
 * directly; each public operation here corresponds to one menu option.
 * Industries are not hard-coded: main() calls registerWorkflow() for each
 * workflow type, and the TUI lets the user pick one from listWorkflowTypes().
 *
 * Every action that changes a workflow is wrapped in a Command and run
 * through the CommandInvoker, which is why undoLastAction() works.
 *
 * Error handling: any operation given a workflow id that does not exist
 * throws std::invalid_argument. An action that exists but is not allowed in
 * the workflow's current state returns false.
 */
class WorkflowFacade 
{
public:
    /** @throws std::invalid_argument if any collaborator is null. */
    WorkflowFacade(std::shared_ptr<WorkflowEngine> engine,
                   std::shared_ptr<CommandInvoker> invoker,
                   std::shared_ptr<WorkflowMediator> mediator);

    // ---- configuration (called from main) and selection
    void registerWorkflow(std::shared_ptr<const WorkflowDefinition> def);
    std::vector<std::string> listWorkflowTypes() const;

    // ---- starting and taking actions
    /** @throws std::invalid_argument if @p type was never registered. */
    int submitRequest(const std::string& type, const WorkflowRequest& request);
    bool approveRequest(int workflowId, int actorId);
    bool rejectRequest(int workflowId, int actorId, const std::string& reason);
    bool suspendRequest(int workflowId, int actorId);
    bool resumeRequest(int workflowId, int actorId);
    bool completeWorkItem(int workflowId, int itemId, int actorId);
    bool undoLastAction(int workflowId);

    // ---- report (Memento)
    bool updateReport(int workflowId, int actorId, const std::string& text);
    std::string getReport(int workflowId) const;
    /** @brief One line per saved revision: "index: revision n - preview". */
    std::vector<std::string> getReportRevisions(int workflowId) const;
    bool restoreReport(int workflowId, int index, int actorId);

    // ---- inspecting progress
    std::vector<int> listWorkflows() const;
    std::string getStatus(int workflowId) const;
    std::vector<std::shared_ptr<WorkItem>> getAvailableWork(int workflowId) const;
    std::vector<WorkflowEvent> getHistory(int workflowId) const;

private:
    std::shared_ptr<WorkflowInstance> requireInstance(int workflowId) const;
    bool run(int workflowId, std::shared_ptr<WorkflowAction> cmd);

    std::shared_ptr<WorkflowEngine> engine_;
    std::shared_ptr<CommandInvoker> invoker_;
    std::shared_ptr<WorkflowMediator> mediator_;
    std::map<std::string, std::shared_ptr<const WorkflowDefinition>> definitions_;
};

#endif
