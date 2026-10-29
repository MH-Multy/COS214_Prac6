#ifndef WORKFLOWENGINE_H
#define WORKFLOWENGINE_H
#include "WorkflowFactory.h"

/** STAND-IN for the core WorkflowEngine. The two factory calls in
 *  startWorkflow() are the Abstract Factory's only client code: copy them
 *  into the real engine. Nothing here knows about any industry. */
class WorkflowEngine {
public:
    WorkflowEngine();
    int startWorkflow(std::shared_ptr<const WorkflowDefinition> def, const WorkflowRequest& req);
    std::shared_ptr<WorkflowInstance> getInstance(int id) const;
    void registerParticipant(const Participant& p);
    const Participant& getParticipant(int id) const;
private:
    std::map<int, std::shared_ptr<WorkflowInstance> > instances;
    std::map<int, Participant> participants;
    int nextId;
};
#endif