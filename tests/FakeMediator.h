#ifndef FAKEMEDIATOR_H
#define FAKEMEDIATOR_H

#include <vector>
#include "WorkflowMediator.h"

/**
 * @brief Test double for the Mediator: records calls instead of coordinating.
 *
 * Lets the Facade be unit-tested in isolation ("mock dependencies" in the
 * Arrange step), without needing the real validation and approval services.
 */
class FakeMediator : public WorkflowMediator {
public:
    std::vector<int> validationRequests;

    void requestValidation(int workflowId) { validationRequests.push_back(workflowId); }
    void validationCompleted(int, const ValidationResult&) {}
    void requestApproval(int) {}
    void approvalCompleted(int, bool) {}
};

#endif
