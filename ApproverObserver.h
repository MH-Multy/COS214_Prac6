#ifndef APPROVER_OBSERVER_H
#define APPROVER_OBSERVER_H

#include "WorkflowObserver.h"
#include <string>
#include <vector>

class ApproverObserver : public WorkflowObserver {
private:
    std::string role_;
    std::vector<std::string> pendingApprovals_;

public:
    explicit ApproverObserver(std::string role);
    virtual ~ApproverObserver() = default;

    void update(const WorkflowEvent& event) override;

    const std::string& getRole() const;
    const std::vector<std::string>& getPendingApprovals() const;
    size_t getPendingCount() const;
};

#endif // APPROVER_OBSERVER_H