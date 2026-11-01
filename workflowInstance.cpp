/**
 * @file WorkflowInstance.cpp
 * @brief Implementation of WorkflowInstance.
 */
#include "WorkflowInstance.h"

#include "ApprovalPolicy.h"
#include "CompositeWorkItem.h"
#include "Participant.h"
#include "WorkItemIterator.h"
#include "WorkflowDefinition.h"

WorkflowInstance::WorkflowInstance(int id, std::shared_ptr<const WorkflowDefinition> definition,
                                   const WorkflowRequest& request,
                                   std::shared_ptr<CompositeWorkItem> root,
                                   std::shared_ptr<ApprovalPolicy> policy,
                                   std::map<int, Participant>* participants)
    : id(id),
      definition(definition),
      request(request),
      root(root ? root : std::make_shared<CompositeWorkItem>(0, "Work", true)),
      currentState(new PendingState()),   // the default state when a request is submitted
      approvalPolicy(policy ? policy : std::shared_ptr<ApprovalPolicy>(
                                           new StrictApprovalPolicy(definition ? definition->getApproverRole() : ""))),
      participants(participants),
      acceptedAt(std::chrono::steady_clock::now()) {}

std::string WorkflowInstance::participantName(int participantId) const {
    Participant* p = findParticipant(participantId);
    return p ? p->getName() : "#" + std::to_string(participantId);
}

// ------------------------------------------------------------------ lifecycle
// Each operation asks the current state to change; the state either replaces itself
// (allowed) or records an ILLEGAL_STATE event and stays (not allowed). Comparing the
// state name before and after tells us which happened.

bool WorkflowInstance::validate() {
    if (getStateName() != "Pending") return false;     // only a new request is validated
    currentState->accepted(*this);
    acceptedAt = std::chrono::steady_clock::now();
    std::string approver = definition ? definition->getApproverRole() : "an approver";
    addEvent(WorkflowEvent("VALIDATED", id, -1, "Validation passed; awaiting a decision from " + approver));
    return true;
}

bool WorkflowInstance::approve(int actorId) {
    bool automatic = actorId < 0;
    if (!automatic) {
        Participant* p = findParticipant(actorId);
        if (participants && (!p || !canApprove(*p))) return false;
    }
    std::string before = getStateName();
    currentState->approved(*this);
    if (getStateName() == before) return false;
    if (automatic)
        addEvent(WorkflowEvent("AUTO_APPROVED", id, -1, "No decision within the time limit; approved automatically"));
    else
        addEvent(WorkflowEvent("APPROVED", id, actorId, "Approved by " + participantName(actorId)));
    start();
    return true;
}

bool WorkflowInstance::reject(int actorId, const std::string& reason) {
    std::string before = getStateName();
    if (actorId < 0) {
        // The system rejects a request that failed validation.
        currentState->rejected(*this);
        if (getStateName() == before) return false;
        addEvent(WorkflowEvent("REJECTED", id, -1, "Validation failed: " + reason));
        return true;
    }
    // A person denies a request that is waiting for a decision.
    Participant* p = findParticipant(actorId);
    if (participants && (!p || !canApprove(*p))) return false;
    currentState->denied(*this, reason);
    if (getStateName() == before) return false;
    addEvent(WorkflowEvent("DENIED", id, actorId, "Denied by " + participantName(actorId) + ": " + reason));
    return true;
}

bool WorkflowInstance::start() {
    if (getStateName() != "Approved") return false;
    root->release();
    assignWork();
    return true;
}

bool WorkflowInstance::suspend(int actorId) {
    if (definition && !actorHasRole(actorId, definition->getApproverRole())) return false;
    std::string before = getStateName();
    currentState->suspended(*this);
    if (getStateName() == before) return false;
    addEvent(WorkflowEvent("SUSPENDED", id, actorId,
                           "Suspended by " + participantName(actorId) + "; work and logs are paused"));
    return true;
}

bool WorkflowInstance::resume(int actorId) {
    if (definition && !actorHasRole(actorId, definition->getApproverRole())) return false;
    if (getStateName() != "Suspended") {
        currentState->illegalState(*this, "Approved (resume)");
        return false;
    }
    currentState->accepted(*this);              // SuspendedState::accepted() resumes the work
    addEvent(WorkflowEvent("RESUMED", id, actorId, "Resumed by " + participantName(actorId)));
    return true;
}

bool WorkflowInstance::complete() {
    if (!allWorkComplete()) return false;
    std::string before = getStateName();
    currentState->completed(*this);
    if (getStateName() == before) return false;
    addEvent(WorkflowEvent("COMPLETED", id, -1, "All work complete"));
    return true;
}

// ------------------------------------------------------------------ work
bool WorkflowInstance::completeItem(int itemId, int actorId) {
    if (getStateName() != "Approved") return false;      // no work while waiting, suspended or finished
    std::shared_ptr<WorkItem> item = findItem(itemId);
    if (!item || !item->getChildren().empty()) return false;
    if (!item->execute(actorId)) return false;           // only the assignee, only when available
    Participant* worker = findParticipant(actorId);
    if (worker) worker->finishWork();
    addEvent(WorkflowEvent("ITEM_COMPLETED", id, actorId,
                           "'" + item->getName() + "' completed by " + participantName(actorId)));
    start();                                             // may release the next task in a sequence
    if (allWorkComplete()) complete();
    return true;
}

bool WorkflowInstance::reopenItem(int itemId, int actorId) {
    if (getStateName() != "Approved") return false;
    std::shared_ptr<WorkItem> item = findItem(itemId);
    if (!item || !item->reopen()) return false;
    Participant* worker = findParticipant(item->getAssignee());
    if (worker) worker->addWork();
    addEvent(WorkflowEvent("ITEM_REOPENED", id, actorId,
                           "'" + item->getName() + "' reopened by " + participantName(actorId)));
    return true;
}

std::vector<std::shared_ptr<WorkItem>> WorkflowInstance::getAvailableWork() const {
    std::vector<std::shared_ptr<WorkItem>> out;
    std::unique_ptr<WorkItemIterator> it = root->createAvailableIterator();
    for (it->first(); !it->isDone(); it->next()) out.push_back(it->currentItem());
    return out;
}

std::shared_ptr<WorkItem> WorkflowInstance::findItem(int itemId) const {
    std::unique_ptr<WorkItemIterator> it = root->createIterator();
    for (it->first(); !it->isDone(); it->next())
        if (it->currentItem()->getId() == itemId) return it->currentItem();
    return std::shared_ptr<WorkItem>();
}

bool WorkflowInstance::allWorkComplete() const { return root->isComplete(); }

void WorkflowInstance::assignWork() {
    if (!participants) return;
    std::vector<std::shared_ptr<WorkItem>> available = getAvailableWork();
    for (std::size_t i = 0; i < available.size(); ++i) {
        std::shared_ptr<WorkItem> task = available[i];
        if (task->getAssignee() != -1) continue;
        // "Requester" tasks go to whoever submitted the request. Any other role goes to someone
        // other than the requester where possible, then to the least busy holder of the role.
        Participant* chosen = 0;
        int requester = request.getRequesterId();
        if (task->getRequiredRole() == "Requester") {
            chosen = findParticipant(requester);
        } else {
            for (std::map<int, Participant>::iterator p = participants->begin(); p != participants->end(); ++p) {
                if (!p->second.hasRole(task->getRequiredRole())) continue;
                if (!chosen) { chosen = &p->second; continue; }
                bool pIsRequester = p->first == requester, chosenIsRequester = chosen->getId() == requester;
                if (pIsRequester != chosenIsRequester) {
                    if (chosenIsRequester) chosen = &p->second;
                } else if (p->second.getWorkload() < chosen->getWorkload()) {
                    chosen = &p->second;
                }
            }
        }
        if (!chosen) {
            addEvent(WorkflowEvent("UNASSIGNED", id, -1, "Nobody holds role '" + task->getRequiredRole() +
                                                             "' for '" + task->getName() + "'"));
            continue;
        }
        task->assignTo(chosen->getId());
        chosen->addWork();
        addEvent(WorkflowEvent("ITEM_ASSIGNED", id, chosen->getId(),
                               "'" + task->getName() + "' assigned to " + chosen->getName()));
    }
}

// ------------------------------------------------------------------ State / Strategy / history
void WorkflowInstance::setState(WorkItemState* next) { currentState.reset(next); }

std::string WorkflowInstance::getStateName() const { return currentState->getStateName(); }

bool WorkflowInstance::canApprove(const Participant& actor) const { return approvalPolicy->canApprove(*this, actor); }

bool WorkflowInstance::shouldAutoApprove() const { return approvalPolicy->shouldAutoApprove(*this); }

void WorkflowInstance::addEvent(const WorkflowEvent& e) {
    history.push_back(e);
    notifyObservers(e);
}

bool WorkflowInstance::actorHasRole(int actorId, const std::string& role) const {
    if (!participants) return true;
    Participant* p = findParticipant(actorId);
    return p && p->hasRole(role);
}

Participant* WorkflowInstance::findParticipant(int participantId) const {
    if (!participants) return 0;
    std::map<int, Participant>::iterator it = participants->find(participantId);
    return it == participants->end() ? 0 : &it->second;
}
