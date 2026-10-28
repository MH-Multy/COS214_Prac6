#ifndef REQUESTER_OBSERVER_H
#define REQUESTER_OBSERVER_H

#include "WorkflowObserver.h"
#include <string>
#include <vector>

class RequesterObserver : public WorkflowObserver {
private:
    int targetRequesterId_;
    std::vector<std::string> inbox_;

public:
    explicit RequesterObserver(int requesterId);
    virtual ~RequesterObserver() = default;

    void update(const WorkflowEvent& event) override;

    int getTargetRequesterId() const;
    const std::vector<std::string>& getInbox() const;
    size_t getUnreadCount() const;
};

#endif // REQUESTER_OBSERVER_H