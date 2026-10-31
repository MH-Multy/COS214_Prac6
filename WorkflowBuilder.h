/**
 * @file WorkflowBuilder.h
 * @brief Builder: abstract interface for building a workflow request step by step.
 */
#ifndef WORKFLOWBUILDER_H
#define WORKFLOWBUILDER_H

#include <string>

using namespace std;

/**
 * @brief Builder: the steps for building a workflow request.
 * @ingroup builder
 *
 * The WorkflowRequestDirector calls these steps in its fixed order; each
 * concrete builder decides what a step produces. The same steps therefore
 * build a WorkflowRequest object (WorkflowRequestBuilder) or a printable
 * summary for the user to confirm (RequestSummaryBuilder).
 *
 * reset() and buildHeader() are required. The remaining steps have empty
 * defaults so a builder can ignore parts its product does not need.
 */
class WorkflowBuilder{
    public:
        /** @brief Virtual destructor so builders can be deleted through a base pointer. */
        virtual ~WorkflowBuilder() = default;

        /** @brief Start a new product, discarding anything from a previous build. */
        virtual void reset() = 0;

        /**
         * @brief Set who is asking and what for.
         * @param name        Request name, e.g. "Blood panel".
         * @param requesterId Participant submitting the request.
         */
        virtual void buildHeader(const string& name, int requesterId) = 0;

        /**
         * @brief Add one industry-specific detail, e.g. riskLevel or loanAmount.
         * @param key   Attribute name.
         * @param value Attribute value.
         */
        virtual void buildAttribute(const string& key, const string& value){ (void)key; (void)value; }

        /** @brief Set how many days the request has. @param days Deadline in days. */
        virtual void buildDeadline(int days){ (void)days; }

        /** @brief Set how urgent the request is. @param level Priority; higher is more urgent. */
        virtual void buildPriority(int level){ (void)level; }
};

#endif //WORKFLOWBUILDER_H
