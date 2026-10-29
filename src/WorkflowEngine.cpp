#include "WorkflowEngine.h"
#include "WorkflowDefinition.h"
#include "WorkflowInstance.h"
#include <stdexcept>

WorkflowEngine::WorkflowEngine() : nextId_(1) {}

int WorkflowEngine::startWorkflow(std::shared_ptr<const WorkflowDefinition> def,
                                  const WorkflowRequest& req) 
{
    if (!def) throw std::invalid_argument("Cannot start a workflow without a definition");

    int id = nextId_++;

    instances_[id] = std::make_shared<WorkflowInstance>(id, def, req);

    return id;
}

std::shared_ptr<WorkflowInstance> WorkflowEngine::getInstance(int id) const 
{
    std::map<int, std::shared_ptr<WorkflowInstance>>::const_iterator it = instances_.find(id);

    return it == instances_.end() ? std::shared_ptr<WorkflowInstance>() : it->second;
}

std::vector<int> WorkflowEngine::listInstances() const 
{
    std::vector<int> ids;
    
    for (std::map<int, std::shared_ptr<WorkflowInstance>>::const_iterator it = instances_.begin();
         it != instances_.end(); ++it)
        ids.push_back(it->first);
    return ids;
}
