#ifndef WORKFLOWREQUEST_H
#define WORKFLOWREQUEST_H

#include <string>
#include <map>

using namespace std;

class WorkflowRequestBuilder;

class WorkflowRequest{
    friend class WorkflowRequestBuilder;
    private:
        string name;
        int requesterId;
        map<string, string> attributes;
        int deadlineDays;
        int priority;
    public:
        WorkflowRequest();
        string getName() const;
        int getRequesterId() const;
        string getAttribute(const string& key) const;
        void setAttribute(const string& key, const string& value);
        int getDeadlineDays() const;
        int getPriority() const;
};

#endif //WORKFLOWREQUEST_H