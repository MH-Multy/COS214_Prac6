#ifndef WORKFLOW_OBSERVER_H
#define WORKFLOW_OBSERVER_H

#include "WorkflowEvent.h"

class WorkflowObserver {
public:
    virtual ~WorkflowObserver() = default;
    virtual void update(const WorkflowEvent& event) = 0;
};

#endif // WORKFLOW_OBSERVER_H