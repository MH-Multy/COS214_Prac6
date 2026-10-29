#include "WorkflowEngine.h"

WorkflowEngine::WorkflowEngine() : nextId(1) {}

int WorkflowEngine::startWorkflow(std::shared_ptr<const WorkflowDefinition> def, const WorkflowRequest& req) {
    int id = nextId++;
    std::shared_ptr<WorkflowInstance> wf(new WorkflowInstance(id, def, req));
    wf->setApprovalPolicy(def->getFactory()->createApprovalPolicy(*def));  // fresh per instance
    wf->setRoot(def->getFactory()->createTaskGroup(*def));                 // fresh per instance
    instances[id] = wf;
    return id;
}

std::shared_ptr<WorkflowInstance> WorkflowEngine::getInstance(int id) const {
    std::map<int, std::shared_ptr<WorkflowInstance> >::const_iterator it = instances.find(id);
    return it == instances.end() ? std::shared_ptr<WorkflowInstance>() : it->second;
}

void WorkflowEngine::registerParticipant(const Participant& p) { participants[p.getId()] = p; }

const Participant& WorkflowEngine::getParticipant(int id) const { return participants.find(id)->second; }