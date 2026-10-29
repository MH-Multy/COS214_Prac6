#ifndef WORKFLOWREQUESTBUILDER_H
#define WORKFLOWREQUESTBUILDER_H

#include "WorkflowBuilder.h"
#include "WorkflowRequest.h"

class WorkflowRequestBuilder : public WorkflowBuilder{
    private:
        WorkflowRequest request;
    public:
        void reset() override;
        void buildHeader(const string& name, int requesterId) override;
        void buildAttribute(const string& key, const string& value) override;
        void buildDeadline(int days) override;
        void buildPriority(int level) override;
        WorkflowRequest getResult();
};

#endif //WORKFLOWREQUESTBUILDER_H

