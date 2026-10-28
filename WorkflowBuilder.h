#ifndef WORKFLOWBUILDER_H
#define WORKFLOWBUILDER_H

#include <string>

using namespace std;

//Abstract Builder
class WorkflowBuilder{
    public:
        virtual ~WorkflowBuilder() = default;
        virtual void reset() = 0;
        virtual void buildHeader(const string& name, int requesterId) = 0;
        virtual void buildAttribute(const string& key, const string& value){}
        virtual void buildDeadline(int days){}
        virtual void buildPriority(int level){}
};

#endif //WORKFLOWBUILDER_H