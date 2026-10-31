/**
 * @file WorkflowRequestBuilder.h
 * @brief Concrete Builder that produces a WorkflowRequest object.
 */
#ifndef WORKFLOWREQUESTBUILDER_H
#define WORKFLOWREQUESTBUILDER_H

#include "WorkflowBuilder.h"
#include "WorkflowRequest.h"

/**
 * @brief Concrete Builder: assembles the WorkflowRequest the engine runs.
 * @ingroup builder
 */
class WorkflowRequestBuilder : public WorkflowBuilder{
    private:
        WorkflowRequest request;  ///< The request being built.
    public:
        /** @brief Start a fresh, empty request. */
        void reset() override;

        /** @copydoc WorkflowBuilder::buildHeader */
        void buildHeader(const string& name, int requesterId) override;

        /** @copydoc WorkflowBuilder::buildAttribute */
        void buildAttribute(const string& key, const string& value) override;

        /** @copydoc WorkflowBuilder::buildDeadline */
        void buildDeadline(int days) override;

        /** @copydoc WorkflowBuilder::buildPriority */
        void buildPriority(int level) override;

        /** @brief The finished product. @return A copy of the request built so far. */
        WorkflowRequest getResult();
};

#endif //WORKFLOWREQUESTBUILDER_H
