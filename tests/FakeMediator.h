#ifndef FAKEMEDIATOR_H
#define FAKEMEDIATOR_H

#include <string>
#include <vector>
#include "WorkflowMediator.h"

/**
 * @brief Test double for the Mediator: records calls instead of coordinating,
 *        so the Facade can be tested on its own.
 */
class FakeMediator : public WorkflowMediator {
public:
    std::vector<int> validationRequests;
    std::vector<int> decisions;
    int timeoutChecks = 0;

    void requestValidation(int workflowId) override { validationRequests.push_back(workflowId); }
    void validationCompleted(int, const ValidationResult&) override {}
    void requestApproval(int) override {}
    void approvalCompleted(int workflowId, bool) override { decisions.push_back(workflowId); }
    int processTimeouts() override { ++timeoutChecks; return 0; }
    std::vector<std::string> getNotifications(int) const override { return std::vector<std::string>(); }
};

#endif
