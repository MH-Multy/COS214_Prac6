/**
 * @file RequestSummaryBuilder.h
 * @brief Concrete Builder that produces a printable summary of a request.
 */
#ifndef REQUESTSUMMARYBUILDER_H
#define REQUESTSUMMARYBUILDER_H

#include "WorkflowBuilder.h"
#include <sstream>

/**
 * @brief Concrete Builder: produces a text summary the TUI shows before a request is submitted.
 * @ingroup builder
 *
 * Driven by the same director steps as WorkflowRequestBuilder, but its
 * product is a string rather than a WorkflowRequest: same construction
 * process, different representation.
 */
class RequestSummaryBuilder : public WorkflowBuilder{
    private:
        ostringstream summary;  ///< Text built so far.
    public:
        /** @brief Clear the summary. */
        void reset() override;

        /** @brief Add the "Request: name (requester id)" line. @param name Request name. @param requesterId Requester. */
        void buildHeader(const string& name, int requesterId) override;

        /** @brief Add a "key: value" line. @param key Attribute name. @param value Attribute value. */
        void buildAttribute(const string& key, const string& value) override;

        /** @brief Add the deadline line. @param days Deadline in days. */
        void buildDeadline(int days) override;

        /** @brief Add the priority line. @param level Priority level. */
        void buildPriority(int level) override;

        /** @brief The finished summary. @return One line per part, ready to print. */
        string getResult();
};

#endif //REQUESTSUMMARYBUILDER_H
