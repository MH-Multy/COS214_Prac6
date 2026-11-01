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
