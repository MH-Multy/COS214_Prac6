/**
 * @file WorkflowEngine.cpp
 * @brief Implementation of WorkflowEngine.
 */
#include "WorkflowEngine.h"

#include <stdexcept>
#include "ApprovalPolicy.h"
#include "CompositeWorkItem.h"
#include "RequesterObserver.h"
#include "WorkflowDefinition.h"
#include "WorkflowFactory.h"
#include "WorkflowInstance.h"
#include "WorkflowObserver.h"

WorkflowEngine::WorkflowEngine() : nextId(1) {}

int WorkflowEngine::startWorkflow(std::shared_ptr<const WorkflowDefinition> def, const WorkflowRequest& req) {
    if (!def) throw std::invalid_argument("Cannot start a workflow without a definition");
    std::shared_ptr<WorkflowFactory> factory = def->getFactory();
    if (!factory) throw std::invalid_argument("Workflow '" + def->getName() + "' has no factory");

    // Abstract Factory: a matching policy and work tree; the engine never names concrete types.
    int id = nextId++;
    std::shared_ptr<WorkflowInstance> instance = std::make_shared<WorkflowInstance>(
        id, def, req, factory->createTaskGroup(*def), factory->createApprovalPolicy(*def), &participants);

    for (std::size_t i = 0; i < globalObservers.size(); ++i) instance->attach(globalObservers[i]);
    std::shared_ptr<RequesterObserver> inbox = std::make_shared<RequesterObserver>(req.getRequesterId());
    instance->attach(inbox);
    requesterInboxes[id] = inbox;
    instances[id] = instance;

    // Recorded after the observers are attached, so they see it too.
    instance->addEvent(WorkflowEvent("CREATED", id, req.getRequesterId(),
        "'" + req.getName() + "' submitted by " + instance->participantName(req.getRequesterId()) +
        " (" + def->getName() + " v" + std::to_string(def->getVersion()) + ")"));
    return id;
}

std::shared_ptr<WorkflowInstance> WorkflowEngine::getInstance(int id) const {
    std::map<int, std::shared_ptr<WorkflowInstance>>::const_iterator it = instances.find(id);
    return it == instances.end() ? std::shared_ptr<WorkflowInstance>() : it->second;
}

std::vector<int> WorkflowEngine::listInstances() const {
    std::vector<int> ids;
    for (std::map<int, std::shared_ptr<WorkflowInstance>>::const_iterator it = instances.begin(); it != instances.end(); ++it)
        ids.push_back(it->first);
    return ids;
}

void WorkflowEngine::registerParticipant(const Participant& p) { participants[p.getId()] = p; }

const Participant& WorkflowEngine::getParticipant(int id) const {
    std::map<int, Participant>::const_iterator it = participants.find(id);
    if (it == participants.end()) throw std::invalid_argument("No participant with id " + std::to_string(id));
    return it->second;
}

std::vector<Participant> WorkflowEngine::listParticipants() const {
    std::vector<Participant> out;
    for (std::map<int, Participant>::const_iterator it = participants.begin(); it != participants.end(); ++it)
        out.push_back(it->second);
    return out;
}

void WorkflowEngine::addGlobalObserver(std::shared_ptr<WorkflowObserver> o) {
    if (o) globalObservers.push_back(o);
}

std::shared_ptr<RequesterObserver> WorkflowEngine::getRequesterInbox(int id) const {
    std::map<int, std::shared_ptr<RequesterObserver>>::const_iterator it = requesterInboxes.find(id);
    return it == requesterInboxes.end() ? std::shared_ptr<RequesterObserver>() : it->second;
}
