#include "WorkflowFacade.h"

#include <stdexcept>

#include "ApproveCommand.h"
#include "CommandInvoker.h"
#include "CompleteWorkItemCommand.h"
#include "EditReportCommand.h"
#include "RejectCommand.h"
#include "ResumeCommand.h"
#include "SuspendCommand.h"
#include "WorkflowDefinition.h"
#include "WorkflowEngine.h"
#include "WorkflowInstance.h"
#include "WorkflowMediator.h"

WorkflowFacade::WorkflowFacade(std::shared_ptr<WorkflowEngine> engine,
                               std::shared_ptr<CommandInvoker> invoker,
                               std::shared_ptr<WorkflowMediator> mediator)
    : engine_(engine), invoker_(invoker), mediator_(mediator) 
{
    if (!engine_ || !invoker_ || !mediator_)
        throw std::invalid_argument("WorkflowFacade needs an engine, an invoker and a mediator");
}

// ------------------------------------------------------------ configuration
void WorkflowFacade::registerWorkflow(std::shared_ptr<const WorkflowDefinition> def) 
{
    if (!def) throw std::invalid_argument("Cannot register a null workflow definition");
    definitions_[def->getName()] = def;
}

std::vector<std::string> WorkflowFacade::listWorkflowTypes() const {
    std::vector<std::string> names;
    for (std::map<std::string, std::shared_ptr<const WorkflowDefinition>>::const_iterator it =
             definitions_.begin(); it != definitions_.end(); ++it)
        names.push_back(it->first);
    return names;
}

// ------------------------------------------------------------ actions
int WorkflowFacade::submitRequest(const std::string& type, const WorkflowRequest& request) 
{
    std::map<std::string, std::shared_ptr<const WorkflowDefinition>>::const_iterator it =
        definitions_.find(type);
    if (it == definitions_.end())
        throw std::invalid_argument("Unknown workflow type: " + type);
    int id = engine_->startWorkflow(it->second, request);
    mediator_->requestValidation(id);
    return id;
}

bool WorkflowFacade::approveRequest(int workflowId, int actorId) 
{
    return run(workflowId, std::make_shared<ApproveCommand>(*engine_, workflowId, actorId));
}

bool WorkflowFacade::rejectRequest(int workflowId, int actorId, const std::string& reason) 
{
    return run(workflowId, std::make_shared<RejectCommand>(*engine_, workflowId, actorId, reason));
}

bool WorkflowFacade::suspendRequest(int workflowId, int actorId) 
{
    return run(workflowId, std::make_shared<SuspendCommand>(*engine_, workflowId, actorId));
}

bool WorkflowFacade::resumeRequest(int workflowId, int actorId) 
{
    return run(workflowId, std::make_shared<ResumeCommand>(*engine_, workflowId, actorId));
}

bool WorkflowFacade::completeWorkItem(int workflowId, int itemId, int actorId) 
{
    return run(workflowId,
               std::make_shared<CompleteWorkItemCommand>(*engine_, workflowId, itemId, actorId));
}

bool WorkflowFacade::undoLastAction(int workflowId) 
{
    requireInstance(workflowId);
    return invoker_->undoLast(workflowId);
}

// ------------------------------------------------------------ report
bool WorkflowFacade::updateReport(int workflowId, int actorId, const std::string& text) 
{
    return run(workflowId, std::make_shared<EditReportCommand>(*engine_, workflowId, actorId, text));
}

std::string WorkflowFacade::getReport(int workflowId) const 
{
    return requireInstance(workflowId)->getReport().getContent();
}

std::vector<std::string> WorkflowFacade::getReportRevisions(int workflowId) const 
{
    const ReportHistory& h = requireInstance(workflowId)->getReportHistory();
    std::vector<std::string> lines;
    for (int i = 0; i < h.size(); ++i) {
        const ReportMemento& m = h.getSnapshot(i);
        lines.push_back(std::to_string(i) + ": revision " + std::to_string(m.getRevision()) +
                        " - " + WorkflowReport::preview(m));
    }
    return lines;
}

bool WorkflowFacade::restoreReport(int workflowId, int index, int actorId) 
{
    std::shared_ptr<WorkflowInstance> wf = requireInstance(workflowId);
    const ReportHistory& h = wf->getReportHistory();
    if (index < 0 || index >= h.size()) return false;
    wf->getReport().restore(h.getSnapshot(index));
    wf->addEvent(WorkflowEvent("REPORT_RESTORED", workflowId, actorId,
                               "Report restored to revision " +
                                   std::to_string(wf->getReport().getRevision())));
    return true;
}

// ------------------------------------------------------------ inspection
std::vector<int> WorkflowFacade::listWorkflows() const { return engine_->listInstances(); }

std::string WorkflowFacade::getStatus(int workflowId) const 
{
    return requireInstance(workflowId)->getStateName();
}

std::vector<std::shared_ptr<WorkItem>> WorkflowFacade::getAvailableWork(int workflowId) const 
{
    return requireInstance(workflowId)->getAvailableWork();
}

std::vector<WorkflowEvent> WorkflowFacade::getHistory(int workflowId) const 
{
    return requireInstance(workflowId)->getHistory();
}

// ------------------------------------------------------------ helpers
std::shared_ptr<WorkflowInstance> WorkflowFacade::requireInstance(int workflowId) const 
{
    std::shared_ptr<WorkflowInstance> wf = engine_->getInstance(workflowId);
    if (!wf) throw std::invalid_argument("No workflow with id " + std::to_string(workflowId));
    return wf;
}

bool WorkflowFacade::run(int workflowId, std::shared_ptr<WorkflowAction> cmd) {
    requireInstance(workflowId);
    return invoker_->executeCommand(cmd);
}
