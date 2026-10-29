#include "WorkflowEvent.h"
#include <ctime>

WorkflowEvent::WorkflowEvent(const std::string& type, int workflowId, int actorId,
                             const std::string& message)
    : type_(type), workflowId_(workflowId), actorId_(actorId), message_(message) 
{
    std::time_t now = std::time(0);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    timestamp_ = buf;
}

std::string WorkflowEvent::toString() const 
{
    std::string who = actorId_ >= 0 ? " by " + std::to_string(actorId_) : "";
    return "[" + timestamp_ + "] " + type_ + who + ": " + message_;
}
