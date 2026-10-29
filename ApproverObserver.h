#ifndef APPROVER_OBSERVER_H
#define APPROVER_OBSERVER_H

#include "WorkflowObserver.h"
#include <string>
#include <vector>
/**
 * @addtogroup Observer
 * @{
 */
/**
 * @brief Concrete Observer alerting reviewers and managers of pending decisions.
 */
class ApproverObserver : public WorkflowObserver {
private:
    std::string role_;
    std::vector<std::string> pendingApprovals_;

public:

    /**
     * @param approverRole Identifier tag (e.g. "HEAD_RESEARCHER", "RISK_COMMITTEE").
     */
    explicit ApproverObserver(std::string role);
    virtual ~ApproverObserver() = default;

    /**
     * @brief Intercepts events requiring managerial action.
     */
    void update(const WorkflowEvent& event) override;

    const std::string& getRole() const;
    const std::vector<std::string>& getPendingApprovals() const;
    size_t getPendingCount() const;
};
/**
 * @}
 */
#endif // APPROVER_OBSERVER_H