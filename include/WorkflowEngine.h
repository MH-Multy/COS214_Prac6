#ifndef WORKFLOWENGINE_H
#define WORKFLOWENGINE_H

#include <map>
#include <memory>
#include <vector>

#include "WorkflowRequest.h"

class WorkflowDefinition;
class WorkflowInstance;

/**
 *  Owns every running workflow instance.
 *
 * MINIMAL VERSION: enough for the Command and Facade code. The engine owner
 * adds participants and uses the definition's factory to build each
 * instance's work items, keeping these three signatures unchanged.
 */
class WorkflowEngine 
{
public:
    WorkflowEngine();

    /**
     *  Create a new, independent instance of @p def.
     * @return the new instance's id.
     * @throws std::invalid_argument if @p def is null.
     */
    int startWorkflow(std::shared_ptr<const WorkflowDefinition> def, const WorkflowRequest& req);

    /**  The instance with @p id, or nullptr if there is none. */
    std::shared_ptr<WorkflowInstance> getInstance(int id) const;

    std::vector<int> listInstances() const;

private:
    std::map<int, std::shared_ptr<WorkflowInstance>> instances_;
    int nextId_;
};

#endif
