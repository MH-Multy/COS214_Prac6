#ifndef REQUESTER_OBSERVER_H
#define REQUESTER_OBSERVER_H

#include "WorkflowObserver.h"
#include <string>
#include <vector>
/**
 * @addtogroup Observer
 * @{
 */
/**
 * @brief Concrete Observer notifying the requester of status updates.
 */
class RequesterObserver : public WorkflowObserver {
private:
    int targetRequesterId_;
    std::vector<std::string> inbox_;

public:

    /**
     * @param requesterId The participant ID receiving inbox notifications.
     */
    explicit RequesterObserver(int requesterId);
    virtual ~RequesterObserver() = default;

    /**
     * @brief Filters and buffers status notifications for the target requester.
     */
    void update(const WorkflowEvent& event) override;

    int getTargetRequesterId() const;
    const std::vector<std::string>& getInbox() const;
    size_t getUnreadCount() const;
};
/**
 * @}
 */
#endif // REQUESTER_OBSERVER_H