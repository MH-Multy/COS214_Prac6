#ifndef REQUESTSUMMARYBUILDER_H
#define REQUESTSUMMARYBUILDER_H

#include "WorkflowBuilder.h"
#include <sstream>

class RequestSummaryBuilder : public WorkflowBuilder{
    private:
        ostringstream summary;
    public:
        void reset() override;
        void buildHeader(const string& name, int requesterId) override;
        void buildAttribute(const string& key, const string& value) override;
        void buildDeadline(int days) override;
        void buildPriority(int level) override;
        string getResult();
};

#endif //REQUESTSUMMARYBUILDER_H