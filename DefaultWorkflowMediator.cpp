/**
 * @file DefaultWorkflowMediator.cpp
 * @brief Implementation of DefaultWorkflowMediator.
 */
#include "DefaultWorkflowMediator.h"

#include "ValidationResult.h"
#include "WorkflowEngine.h"
#include "WorkflowInstance.h"

namespace {
/** Every critical validation problem on one line. */
std::string summarise(const ValidationResult& result) {
    std::string out;
    const std::vector<ValidationIssue>& issues = result.getIssues();
    for (std::size_t i = 0; i < issues.size(); ++i) {
        if (issues[i].severity != IssueSeverity::CRITICAL_ERROR) continue;
        std::string message = issues[i].message;
        if (!message.empty() && message[message.size() - 1] == '.') message.erase(message.size() - 1);
        if (!out.empty()) out += "; ";
        out += message;
    }
    return out.empty() ? "request did not meet the requirements" : out;
}
}  // namespace

DefaultWorkflowMediator::DefaultWorkflowMediator(WorkflowEngine& engine) : engine(engine), approvals(engine) {
    validator.setMediator(this);
    approvals.setMediator(this);
    notifier.setMediator(this);
}

void DefaultWorkflowMediator::requestValidation(int workflowId) {
    std::shared_ptr<WorkflowInstance> wf = engine.getInstance(workflowId);
    if (wf) validator.validate(*wf);
}

void DefaultWorkflowMediator::validationCompleted(int workflowId, const ValidationResult& result) {
    std::shared_ptr<WorkflowInstance> wf = engine.getInstance(workflowId);
    if (!wf) return;
    int requester = wf->getRequest().getRequesterId();
    std::string tag = "Workflow #" + std::to_string(workflowId) + ": ";
    if (result.isValid()) {
        if (!wf->validate()) return;
        notifier.send(requester, tag + "your request passed validation and is awaiting approval");
        notifier.send(-1, tag + "decision required");
        requestApproval(workflowId);
    } else {
        if (!wf->reject(-1, summarise(result))) return;
        notifier.send(requester, tag + "your request was rejected: " + summarise(result));
    }
}

void DefaultWorkflowMediator::requestApproval(int workflowId) {
    std::shared_ptr<WorkflowInstance> wf = engine.getInstance(workflowId);
    if (wf) approvals.requestApproval(*wf);
}

void DefaultWorkflowMediator::approvalCompleted(int workflowId, bool approved) {
    std::shared_ptr<WorkflowInstance> wf = engine.getInstance(workflowId);
    if (!wf) return;
    std::string tag = "Workflow #" + std::to_string(workflowId) + ": ";
    notifier.send(wf->getRequest().getRequesterId(),
                  tag + (approved ? "your request was approved; work has started" : "your request was denied"));
    if (!approved) return;
    // An instance is approved at most once, so an AUTO_APPROVED event means this decision was automatic.
    const std::vector<WorkflowEvent>& h = wf->getHistory();
    for (std::size_t i = 0; i < h.size(); ++i)
        if (h[i].getType() == "AUTO_APPROVED") {
            notifier.send(-1, tag + "approved automatically because no decision was made in time");
            return;
        }
}

int DefaultWorkflowMediator::processTimeouts() {
    std::vector<int> ids = engine.listInstances();
    int waitingBefore = 0, waitingAfter = 0;
    for (std::size_t i = 0; i < ids.size(); ++i)
        if (engine.getInstance(ids[i])->getStateName() == "Accepted") ++waitingBefore;
    approvals.checkTimeouts();
    for (std::size_t i = 0; i < ids.size(); ++i)
        if (engine.getInstance(ids[i])->getStateName() == "Accepted") ++waitingAfter;
    return waitingBefore - waitingAfter;
}

std::vector<std::string> DefaultWorkflowMediator::getNotifications(int participantId) const {
    return notifier.getMessages(participantId);
}

const NotificationService& DefaultWorkflowMediator::getNotifier() const { return notifier; }
