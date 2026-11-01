#include "RequesterObserver.h"
#include <sstream>

RequesterObserver::RequesterObserver(int requesterId)
    : targetRequesterId_(requesterId) {}

void RequesterObserver::update(const WorkflowEvent& event) {
    std::stringstream note;
    note << "[Notification for Requester #" << targetRequesterId_ << "]: "
         << "Workflow #" << event.getWorkflowId() << " event -> "
         << event.getType() << ": " << event.getMessage();
    inbox_.push_back(note.str());
}

int RequesterObserver::getTargetRequesterId() const {
    return targetRequesterId_;
}

const std::vector<std::string>& RequesterObserver::getInbox() const {
    return inbox_;
}

size_t RequesterObserver::getUnreadCount() const {
    return inbox_.size();
}