#ifndef WORKFLOW_SUBJECT_H
#define WORKFLOW_SUBJECT_H

#include "WorkflowObserver.h"
#include <vector>
#include <memory>

class WorkflowSubject {
protected:
    std::vector<std::shared_ptr<WorkflowObserver>> observers_;

public:
    WorkflowSubject() = default;
    virtual ~WorkflowSubject() = default;

    virtual void attach(std::shared_ptr<WorkflowObserver> observer);
    virtual void detach(std::shared_ptr<WorkflowObserver> observer);
    virtual void notifyObservers(const WorkflowEvent& event);

    size_t getObserverCount() const;
};

#endif // WORKFLOW_SUBJECT_H