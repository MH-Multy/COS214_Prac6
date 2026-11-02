/**
 * @file WorkflowEngine.h
 * @brief Owns every participant and every running workflow instance.
 */
#ifndef WORKFLOWENGINE_H
#define WORKFLOWENGINE_H

#include <map>
#include <memory>
#include <vector>

#include "Participant.h"
#include "WorkflowRequest.h"

class RequesterObserver;
class WorkflowDefinition;
class WorkflowInstance;
class WorkflowObserver;

/**
 * @brief The reusable engine: creates and owns instances across all industries.
 * @ingroup core
 *
 * Starting a workflow asks the definition's factory for the instance's approval
 * policy and work tree, attaches the observers, and records CREATED.
 */
class WorkflowEngine {
public:
    WorkflowEngine();

    /**
     * @brief Create a new, independent instance of @p def.
     * @return The new instance's id.
     * @throws std::invalid_argument if @p def is null or has no factory.
     */
    int startWorkflow(std::shared_ptr<const WorkflowDefinition> def, const WorkflowRequest& req);
    /** @brief The instance with @p id, or an empty pointer. */
    std::shared_ptr<WorkflowInstance> getInstance(int id) const;
    std::vector<int> listInstances() const;

    void registerParticipant(const Participant& p);
    /** @brief A registered participant. @throws std::invalid_argument if there is none with @p id. */
    const Participant& getParticipant(int id) const;
    std::vector<Participant> listParticipants() const;

    /** @brief Observe every instance created from now on (audit log, approver alerts). */
    void addGlobalObserver(std::shared_ptr<WorkflowObserver> o);
    /** @brief The requester's personal inbox for one instance, or empty. */
    std::shared_ptr<RequesterObserver> getRequesterInbox(int id) const;

private:
    std::map<int, std::shared_ptr<WorkflowInstance>> instances;
    std::map<int, Participant> participants;
    std::vector<std::shared_ptr<WorkflowObserver>> globalObservers;
    std::map<int, std::shared_ptr<RequesterObserver>> requesterInboxes;
    int nextId;
};

#endif
