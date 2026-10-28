#ifndef WORKFLOWREQUESTDIRECTOR_H
#define WORKFLOWREQUESTDIRECTOR_H

#include "WorkflowBuilder.h"
#include <map>
#include <string>

using namespace std;

class WorkflowRequestDirector{
    private:
        WorkflowBuilder* builder;
    public:
        WorkflowRequestDirector(WorkflowBuilder* b);
        void setBuilder(WorkflowBuilder* b);
        void constructStandard(const map<string, string>& input);
        void constructUrgent(const map<string, string>& input);
};

#endif //WORKFLOWREQUESTDIRECTOR_H