/**
 * @file WorkflowColleague.cpp
 * @brief Implementation of the Mediator colleagues.
 */
#include "WorkflowColleague.h"

#include "ValidationHandler.h"
#include "WorkflowDefinition.h"
#include "WorkflowEngine.h"
#include "WorkflowInstance.h"
#include "WorkflowMediator.h"

WorkflowColleague::WorkflowColleague() : mediator(0) {}
WorkflowColleague::~WorkflowColleague() {}
void WorkflowColleague::setMediator(WorkflowMediator* m) { mediator = m; }

// ---------------- ValidationService
void ValidationService::validate(WorkflowInstance& wf) {
    ValidationResult result;
    std::shared_ptr<const WorkflowDefinition> def = wf.getDefinition();
    if (def && def->getValidationChain())
        result = def->getValidationChain()->handle(wf, ChainExecutionMode::AUDIT_ACCUMULATE);  // report every problem
    if (mediator) mediator->validationCompleted(wf.getId(), result);
}

// ---------------- ApprovalManager
ApprovalManager::ApprovalManager(WorkflowEngine& engine) : engine(engine) {}

void ApprovalManager::requestApproval(WorkflowInstance& wf) {
    if (wf.shouldAutoApprove() && wf.approve(-1) && mediator) mediator->approvalCompleted(wf.getId(), true);
}

void ApprovalManager::checkTimeouts() {
    std::vector<int> ids = engine.listInstances();
    for (std::size_t i = 0; i < ids.size(); ++i) {
        std::shared_ptr<WorkflowInstance> wf = engine.getInstance(ids[i]);
        if (wf && wf->shouldAutoApprove() && wf->approve(-1) && mediator)
            mediator->approvalCompleted(wf->getId(), true);
    }
}

// ---------------- NotificationService
void NotificationService::send(int participantId, const std::string& message) {
    inbox[participantId].push_back(message);
}

std::vector<std::string> NotificationService::getMessages(int participantId) const {
    std::map<int, std::vector<std::string>>::const_iterator it = inbox.find(participantId);
    return it == inbox.end() ? std::vector<std::string>() : it->second;
}

int NotificationService::getSentCount() const {
    int n = 0;
    for (std::map<int, std::vector<std::string>>::const_iterator it = inbox.begin(); it != inbox.end(); ++it)
        n += static_cast<int>(it->second.size());
    return n;
}
