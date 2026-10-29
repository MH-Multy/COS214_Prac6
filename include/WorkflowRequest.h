#ifndef WORKFLOWREQUEST_H
#define WORKFLOWREQUEST_H

#include <map>
#include <string>

/**  Input data for one workflow run (Builder product).

* MINIMAL version:the Builder owner extends this class. Industry-specific
* values (risk level, loan amount, ...) live in the generic attribute map,
* so no class has to know which industry it serves.
*/
class WorkflowRequest 
{
public:
    WorkflowRequest() : requesterId_(-1), deadlineDays_(0), priority_(0) {}
    WorkflowRequest(const std::string& name, int requesterId)
        : name_(name), requesterId_(requesterId), deadlineDays_(0), priority_(0) {}

    std::string getName() const { return name_; }
    int getRequesterId() const { return requesterId_; }

    std::string getAttribute(const std::string& key) const 
    {
        std::map<std::string, std::string>::const_iterator it = attributes_.find(key);
        return it == attributes_.end() ? "" : it->second;
    }
    void setAttribute(const std::string& key, const std::string& value) { attributes_[key] = value; }

    int getDeadlineDays() const { return deadlineDays_; }
    void setDeadlineDays(int days) { deadlineDays_ = days; }
    int getPriority() const { return priority_; }
    void setPriority(int level) { priority_ = level; }

private:
    std::string name_;
    int requesterId_;
    std::map<std::string, std::string> attributes_;
    int deadlineDays_;
    int priority_;
};

#endif
