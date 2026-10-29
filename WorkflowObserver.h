/**
 * @defgroup Observer Observer Pattern
 * @brief Event dispatching and subscriber notifications for workflow lifecycle transitions.
 * @{
 * @}
 */
#ifndef WORKFLOW_OBSERVER_H
#define WORKFLOW_OBSERVER_H

#include "WorkflowEvent.h"
/**
 * @addtogroup Observer
 * @{
 */
/**
 * @brief Observer Interface in the GoF Observer Pattern.
 *        Subscribes to state changes and notifications dispatched by WorkflowSubject.
 */
class WorkflowObserver {
public:
    virtual ~WorkflowObserver() = default;

    /**
     * @brief Receive event notifications from the subject.
     * @param event The workflow event details.
     */
    virtual void update(const WorkflowEvent& event) = 0;
};
/**
 * @}
 */
#endif // WORKFLOW_OBSERVER_H