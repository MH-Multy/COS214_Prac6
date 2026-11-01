#include "ApproverObserver.h"
#include <sstream>

ApproverObserver::ApproverObserver(std::string role)
    : role_(std::move(role)) {}

void ApproverObserver::update(const WorkflowEvent& event) {
    const std::string& type = event.getType();
    // Intercept action items requiring reviewer/approver attention
    if (type == "CREATED" || type == "VALIDATED" || type == "SUSPENDED" || type == "PENDING_APPROVAL") {
        std::stringstream notice;
        notice << "[" << role_ << " ACTION REQUIRED]: Workflow #" 
               << event.getWorkflowId() << " - " << event.getMessage();
        pendingApprovals_.push_back(notice.str());
    }
}

const std::string& ApproverObserver::getRole() const {
    return role_;
}

const std::vector<std::string>& ApproverObserver::getPendingApprovals() const {
    return pendingApprovals_;
}

size_t ApproverObserver::getPendingCount() const {
    return pendingApprovals_.size();
}