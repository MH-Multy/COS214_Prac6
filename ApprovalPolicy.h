#ifndef APPROVALPOLICY_H
#define APPROVALPOLICY_H

#include <chrono>
class WorkflowInstance;
using namespace std;

class ApprovalPolicy
{
    public:
        virtual ~ApprovalPolicy();
        virtual bool shouldAutoApprove(const WorkflowInstance& wf) const = 0;
};

// --------- Concrete Strategy --------------
class TimeoutApprovalPolicy : public ApprovalPolicy
{
    private:
        int timeoutSeconds;

    public:
        explicit TimeoutApprovalPolicy(int seconds);
        bool shouldAutoApprove(const WorkflowInstance& wf) const override;
};

class StrictApprovalPolicy : public ApprovalPolicy
{
    public:
        bool shouldAutoApprove(const WorkflowInstance& wf) const override;
};

#endif //APPROVALPOLICY_H