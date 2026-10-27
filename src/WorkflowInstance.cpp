#include "WorkflowInstance.h"
#include "WorkflowDefinition.h"

// NOTE: the state checks below are a PLACEHOLDER for the State pattern.
// When WorkflowState and its concrete states exist, each operation becomes a
// one-line delegation to currentState_ and these string checks disappear.

WorkflowInstance::WorkflowInstance(int id, std::shared_ptr<const WorkflowDefinition> definition,
                                   const WorkflowRequest& request)
    : id_(id), definition_(definition), request_(request), state_("PendingValidation") 
{
    std::string type = definition_ ? definition_->getName() : "unknown";
    addEvent(WorkflowEvent("CREATED", id_, request_.getRequesterId(),
                           "Instance of '" + type + "' created"));
}

bool WorkflowInstance::moveTo(const std::string& next, const std::string& eventType, int actorId,
                              const std::string& message) 
{
    state_ = next;
    addEvent(WorkflowEvent(eventType, id_, actorId, message + " (now " + next + ")"));
    return true;
}

bool WorkflowInstance::validate() 
{
    if (state_ != "PendingValidation") return false;
    return moveTo("PendingApproval", "VALIDATED", -1, "Validation passed");
}

bool WorkflowInstance::approve(int actorId) 
{
    if (state_ != "PendingApproval") return false;
    return moveTo("Approved", "APPROVED", actorId, "Request approved");
}

bool WorkflowInstance::reject(int actorId, const std::string& reason) 
{
    if (state_ != "PendingApproval") return false;
    return moveTo("Rejected", "REJECTED", actorId, "Request rejected: " + reason);
}

bool WorkflowInstance::start() 
{
    if (state_ != "Approved") return false;
    return moveTo("InProgress", "STARTED", -1, "Work started");
}

bool WorkflowInstance::suspend(int actorId) 
{
    if (state_ != "Approved" && state_ != "InProgress") return false;
    resumeTo_ = state_;
    return moveTo("Suspended", "SUSPENDED", actorId, "Workflow suspended");
}

bool WorkflowInstance::resume(int actorId) 
{
    if (state_ != "Suspended") return false;
    return moveTo(resumeTo_, "RESUMED", actorId, "Workflow resumed");
}

bool WorkflowInstance::complete() 
{
    if (state_ != "InProgress") return false;
    for (std::map<int, std::pair<std::string, bool> >::const_iterator it = items_.begin();
         it != items_.end(); ++it)
        if (!it->second.second) return false;
    return moveTo("Completed", "COMPLETED", -1, "All work complete");
}

bool WorkflowInstance::completeItem(int itemId, int actorId) 
{
    if (state_ != "InProgress") return false;
    std::map<int, std::pair<std::string, bool> >::iterator it = items_.find(itemId);
    if (it == items_.end() || it->second.second) return false;
    it->second.second = true;
    addEvent(WorkflowEvent("ITEM_COMPLETED", id_, actorId, "Completed '" + it->second.first + "'"));
    return true;
}

bool WorkflowInstance::reopenItem(int itemId, int actorId) 
{
    if (state_ != "InProgress") return false;
    std::map<int, std::pair<std::string, bool> >::iterator it = items_.find(itemId);
    if (it == items_.end() || !it->second.second) return false;
    it->second.second = false;
    addEvent(WorkflowEvent("ITEM_REOPENED", id_, actorId, "Reopened '" + it->second.first + "'"));
    return true;
}

std::vector<std::shared_ptr<WorkItem>> WorkflowInstance::getAvailableWork() const 
{
    // PLACEHOLDER: the Iterator owner walks root_ with an AvailableWorkIterator here.
    return std::vector<std::shared_ptr<WorkItem>>();
}

void WorkflowInstance::addEvent(const WorkflowEvent& e) 
{
    history_.push_back(e);
    // Observer owner: call notifyObservers(e) here once WorkflowSubject is in place.
}

void WorkflowInstance::addPlaceholderItem(int itemId, const std::string& name) 
{
    items_[itemId] = std::make_pair(name, false);
}
