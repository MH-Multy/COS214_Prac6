/**
 * @file WorkflowFacade.cpp
 * @brief Implementation of WorkflowFacade.
 */
#include "WorkflowFacade.h"

#include <stdexcept>

#include "ApproveCommand.h"
#include "ApproverObserver.h"
#include "AuditLogger.h"
#include "CommandInvoker.h"
#include "CompleteWorkItemCommand.h"
#include "EditReportCommand.h"
#include "RejectCommand.h"
#include "RequesterObserver.h"
#include "ResumeCommand.h"
#include "SuspendCommand.h"
#include "WorkItem.h"
#include "WorkflowDefinition.h"
#include "WorkflowEngine.h"
#include "WorkflowInstance.h"
#include "WorkflowMediator.h"

WorkflowFacade::WorkflowFacade(std::shared_ptr<WorkflowEngine> engine,
                               std::shared_ptr<CommandInvoker> invoker,
                               std::shared_ptr<WorkflowMediator> mediator)
    : engine(engine), invoker(invoker), mediator(mediator),
      audit(std::make_shared<AuditLogger>()),
      approvers(std::make_shared<ApproverObserver>("Approvers")) {
    if (!this->engine || !this->invoker || !this->mediator)
        throw std::invalid_argument("WorkflowFacade needs an engine, an invoker and a mediator");
    // System-wide observers: every instance created from now on reports to them.
    this->engine->addGlobalObserver(audit);
    this->engine->addGlobalObserver(approvers);
}

// ------------------------------------------------------------ configuration
void WorkflowFacade::registerWorkflow(std::shared_ptr<const WorkflowDefinition> def) {
    if (!def) throw std::invalid_argument("Cannot register a null workflow definition");
    definitions[def->getName()] = def;
}

std::vector<std::string> WorkflowFacade::listWorkflowTypes() const {
    std::vector<std::string> names;
    for (std::map<std::string, std::shared_ptr<const WorkflowDefinition>>::const_iterator it = definitions.begin();
         it != definitions.end(); ++it)
        names.push_back(it->first);
    return names;
}

std::shared_ptr<const WorkflowDefinition> WorkflowFacade::getDefinition(const std::string& type) const {
    std::map<std::string, std::shared_ptr<const WorkflowDefinition>>::const_iterator it = definitions.find(type);
    if (it == definitions.end()) throw std::invalid_argument("Unknown workflow type: " + type);
    return it->second;
}

void WorkflowFacade::registerParticipant(const Participant& p) { engine->registerParticipant(p); }

std::vector<Participant> WorkflowFacade::listParticipants() const { return engine->listParticipants(); }

std::string WorkflowFacade::participantName(int participantId) const {
    try {
        return engine->getParticipant(participantId).getName();
    } catch (const std::invalid_argument&) {
        return "#" + std::to_string(participantId);
    }
}

// ------------------------------------------------------------ actions
int WorkflowFacade::submitRequest(const std::string& type, const WorkflowRequest& request) {
    std::shared_ptr<const WorkflowDefinition> def = getDefinition(type);
    int id = engine->startWorkflow(def, request);   // the engine creates the instance...
    mediator->requestValidation(id);                // ...and the mediator validates it and requests a decision
    return id;
}

bool WorkflowFacade::approveRequest(int workflowId, int actorId) {
    bool ok = run(workflowId, std::make_shared<ApproveCommand>(*engine, workflowId, actorId));
    if (ok) mediator->approvalCompleted(workflowId, true);
    return ok;
}

bool WorkflowFacade::rejectRequest(int workflowId, int actorId, const std::string& reason) {
    bool ok = run(workflowId, std::make_shared<RejectCommand>(*engine, workflowId, actorId, reason));
    if (ok) mediator->approvalCompleted(workflowId, false);
    return ok;
}

bool WorkflowFacade::suspendRequest(int workflowId, int actorId) {
    return run(workflowId, std::make_shared<SuspendCommand>(*engine, workflowId, actorId));
}

bool WorkflowFacade::resumeRequest(int workflowId, int actorId) {
    return run(workflowId, std::make_shared<ResumeCommand>(*engine, workflowId, actorId));
}

bool WorkflowFacade::completeWorkItem(int workflowId, int itemId, int actorId) {
    return run(workflowId, std::make_shared<CompleteWorkItemCommand>(*engine, workflowId, itemId, actorId));
}

bool WorkflowFacade::undoLastAction(int workflowId) {
    requireInstance(workflowId);
    return invoker->undoLast(workflowId);
}

bool WorkflowFacade::canApprove(int workflowId, int actorId) const {
    std::shared_ptr<WorkflowInstance> wf = requireInstance(workflowId);
    if (wf->getStateName() != "Accepted") return false;
    try {
        return wf->canApprove(engine->getParticipant(actorId));
    } catch (const std::invalid_argument&) {
        return false;
    }
}

int WorkflowFacade::processTimeouts() { return mediator->processTimeouts(); }

// ------------------------------------------------------------ report
bool WorkflowFacade::updateReport(int workflowId, int actorId, const std::string& text) {
    return run(workflowId, std::make_shared<EditReportCommand>(*engine, workflowId, actorId, text));
}

std::string WorkflowFacade::getReport(int workflowId) const {
    return requireInstance(workflowId)->getReport().getContent();
}

std::vector<std::string> WorkflowFacade::getReportRevisions(int workflowId) const {
    const ReportHistory& h = requireInstance(workflowId)->getReportHistory();
    std::vector<std::string> lines;
    for (int i = 0; i < h.size(); ++i) {
        const ReportMemento& m = h.getSnapshot(i);
        lines.push_back(std::to_string(i) + ": revision " + std::to_string(m.getRevision()) + " - " +
                        WorkflowReport::preview(m, 50));
    }
    return lines;
}

bool WorkflowFacade::restoreReport(int workflowId, int index, int actorId) {
    std::shared_ptr<WorkflowInstance> wf = requireInstance(workflowId);
    const ReportHistory& h = wf->getReportHistory();
    std::string state = wf->getStateName();
    bool locked = state == "Suspended" || state == "Completed" || state == "Rejected" || state == "Denied";
    if (index < 0 || index >= h.size() || locked) return false;
    wf->getReport().restore(h.getSnapshot(index));
    wf->addEvent(WorkflowEvent("REPORT_RESTORED", workflowId, actorId,
                               "Report restored to revision " + std::to_string(wf->getReport().getRevision())));
    return true;
}

// ------------------------------------------------------------ inspection
std::vector<int> WorkflowFacade::listWorkflows() const { return engine->listInstances(); }

std::string WorkflowFacade::getStatus(int workflowId) const { return requireInstance(workflowId)->getStateName(); }

std::shared_ptr<const WorkflowInstance> WorkflowFacade::getWorkflow(int workflowId) const {
    return requireInstance(workflowId);
}

std::vector<std::shared_ptr<WorkItem>> WorkflowFacade::getAvailableWork(int workflowId) const {
    std::shared_ptr<WorkflowInstance> wf = requireInstance(workflowId);
    if (wf->getStateName() != "Approved") return std::vector<std::shared_ptr<WorkItem>>();
    return wf->getAvailableWork();
}

std::vector<WorkflowEvent> WorkflowFacade::getHistory(int workflowId) const {
    return requireInstance(workflowId)->getHistory();
}

std::vector<std::string> WorkflowFacade::getRequesterInbox(int workflowId) const {
    requireInstance(workflowId);
    std::shared_ptr<RequesterObserver> inbox = engine->getRequesterInbox(workflowId);
    return inbox ? inbox->getInbox() : std::vector<std::string>();
}

std::vector<std::string> WorkflowFacade::getNotifications(int participantId) const {
    return mediator->getNotifications(participantId);
}

std::vector<std::string> WorkflowFacade::getAuditLog() const { return audit->getAuditLog(); }

std::vector<std::string> WorkflowFacade::getApproverQueue() const { return approvers->getPendingApprovals(); }

std::vector<std::string> WorkflowFacade::getActionHistory() const { return invoker->getHistory(); }

// ------------------------------------------------------------ helpers
std::shared_ptr<WorkflowInstance> WorkflowFacade::requireInstance(int workflowId) const {
    std::shared_ptr<WorkflowInstance> wf = engine->getInstance(workflowId);
    if (!wf) throw std::invalid_argument("No workflow with id " + std::to_string(workflowId));
    return wf;
}

bool WorkflowFacade::run(int workflowId, std::shared_ptr<WorkflowAction> cmd) {
    requireInstance(workflowId);   // unknown id -> exception; "not allowed now" -> false
    return invoker->executeCommand(cmd);
}
