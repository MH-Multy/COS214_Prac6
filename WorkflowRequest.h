/**
 * @file WorkflowRequest.h
 * @brief Builder Product: the input data for one workflow run.
 */
#ifndef WORKFLOWREQUEST_H
#define WORKFLOWREQUEST_H

#include <string>
#include <map>

using namespace std;

class WorkflowRequestBuilder;

/**
 * @brief Builder Product: everything a requester supplies to start one workflow run.
 * @ingroup builder
 *
 * Industry-specific details (risk level for a lab test, loan amount for a
 * loan) are stored in a generic attribute map, so no class needs to know which
 * industry it serves. WorkflowRequestBuilder is a friend so it can set the
 * fixed fields while building.
 */
class WorkflowRequest{
    friend class WorkflowRequestBuilder;
    private:
        string name;                    ///< e.g. "Blood panel".
        int requesterId;                ///< Participant who submitted it.
        map<string, string> attributes; ///< Industry-specific details.
        int deadlineDays;               ///< Days allowed for the request.
        int priority;                   ///< Higher is more urgent.
    public:
        /** @brief An empty request: no name, requester 0, no deadline, priority 0. */
        WorkflowRequest();

        /**
         * @brief A request with just a name and requester.
         * @param name        Request name.
         * @param requesterId Participant submitting it.
         */
        WorkflowRequest(const string& name, int requesterId);

        /** @brief Request name. @return The name. */
        string getName() const;

        /** @brief Who submitted it. @return The requester's participant id. */
        int getRequesterId() const;

        /**
         * @brief Look up an industry-specific detail.
         * @param key Attribute name.
         * @return Its value, or an empty string if the request does not have it.
         */
        string getAttribute(const string& key) const;

        /**
         * @brief Set an industry-specific detail, replacing any existing value.
         * @param key   Attribute name.
         * @param value Attribute value.
         */
        void setAttribute(const string& key, const string& value);

        /** @brief Deadline. @return Days allowed. */
        int getDeadlineDays() const;

        /** @brief Urgency. @return Priority level; higher is more urgent. */
        int getPriority() const;
};

#endif //WORKFLOWREQUEST_H
<<<<<<< HEAD
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
=======
>>>>>>> 41fb330 (Updated Composite and Builder)
