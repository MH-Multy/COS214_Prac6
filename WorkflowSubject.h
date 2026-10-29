
#ifndef WORKFLOW_SUBJECT_H
#define WORKFLOW_SUBJECT_H

#include "WorkflowObserver.h"
#include <vector>
#include <memory>
/**
 * @addtogroup Observer
 * @{
 */
/**
 * @brief Subject in the GoF Observer Pattern.
 *        Maintains observer subscriptions and broadcasts workflow events.
 */
class WorkflowSubject {
protected:
    std::vector<std::shared_ptr<WorkflowObserver>> observers_;

public:
    WorkflowSubject() = default;
    virtual ~WorkflowSubject() = default;
    /**
     * @brief Subscribes an observer to event broadcasts.
     */
    virtual void attach(std::shared_ptr<WorkflowObserver> observer);

    /**
     * @brief Unsubscribes an observer from event broadcasts.
     */
    virtual void detach(std::shared_ptr<WorkflowObserver> observer);

    /**
     * @brief Dispatches a workflow event to all attached observers.
     */
    virtual void notifyObservers(const WorkflowEvent& event);

    size_t getObserverCount() const;
};
/**
 * @}
 */
#endif // WORKFLOW_SUBJECT_H