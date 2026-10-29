
#include "WorkflowMediator.h"

/* ---------- WorkflowColleague ---------- */
WorkflowColleague::WorkflowColleague() : mediator(0) {}
void WorkflowColleague::setMediator(WorkflowMediator* m) { mediator = m; }

/* ---------- ValidationService ---------- */
void ValidationService::validate(WorkflowInstance& wf) {
    std::shared_ptr<ValidationHandler> chain = wf.getDefinition()->getValidationChain();
    ValidationResult r = chain ? chain->handle(wf) : ValidationResult(true);
    mediator->validationCompleted(wf.getId(), r);
}

/* ---------- ApprovalManager ---------- */
void ApprovalManager::requestApproval(WorkflowInstance& wf) {
    if (wf.shouldAutoApprove()) { mediator->approvalCompleted(wf.getId(), true); return; }
    pending.push_back(&wf);                       // waits for a human or the timeout
}

void ApprovalManager::checkTimeouts() {
    std::vector<WorkflowInstance*> waiting;
    waiting.swap(pending);
    for (size_t k = 0; k < waiting.size(); ++k) {
        WorkflowInstance* wf = waiting[k];
        if (wf->getStateName() != "PendingApproval") continue;   // already decided
        if (wf->shouldAutoApprove()) mediator->approvalCompleted(wf->getId(), true);
        else pending.push_back(wf);
    }
}

/* ---------- NotificationService ---------- */
void NotificationService::send(int participantId, const std::string& message) {
    std::cout << "  [notify participant " << participantId << "] " << message << "\n";
    log.push_back(message);
}
const std::vector<std::string>& NotificationService::getLog() const { return log; }

/* ---------- DefaultWorkflowMediator ---------- */
const int DefaultWorkflowMediator::SYSTEM_ACTOR;

DefaultWorkflowMediator::DefaultWorkflowMediator(WorkflowEngine& e, ValidationService& v,
                                                 ApprovalManager& a, NotificationService& n)
    : engine(e), validator(v), approvals(a), notifier(n) {
    v.setMediator(this); a.setMediator(this); n.setMediator(this);
}

void DefaultWorkflowMediator::requestValidation(int id) {
    std::shared_ptr<WorkflowInstance> wf = engine.getInstance(id);
    if (wf) validator.validate(*wf);
}

void DefaultWorkflowMediator::validationCompleted(int id, const ValidationResult& r) {
    std::shared_ptr<WorkflowInstance> wf = engine.getInstance(id);
    if (!wf) return;
    if (!r.isPassed()) {
        wf->reject(SYSTEM_ACTOR, r.getReason());
        notifier.send(wf->getRequest().getRequesterId(), "request rejected: " + r.getReason());
        return;
    }
    wf->validate();
    requestApproval(id);
}

void DefaultWorkflowMediator::requestApproval(int id) {
    std::shared_ptr<WorkflowInstance> wf = engine.getInstance(id);
    if (!wf) return;
    approvals.requestApproval(*wf);               // may complete synchronously
    if (wf->getStateName() == "PendingApproval")
        notifier.send(wf->getRequest().getRequesterId(), "awaiting approval");
}

void DefaultWorkflowMediator::approvalCompleted(int id, bool approved) {
    std::shared_ptr<WorkflowInstance> wf = engine.getInstance(id);
    if (!wf) return;
    int requester = wf->getRequest().getRequesterId();
    if (approved && wf->approve(SYSTEM_ACTOR)) {
        wf->start();
        notifier.send(requester, "approved, work started");
    } else if (!approved && wf->reject(SYSTEM_ACTOR, "approval denied")) {
        notifier.send(requester, "approval denied");
    }
}