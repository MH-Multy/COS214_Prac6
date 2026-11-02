#include "WorkflowSubject.h"
#include <algorithm>

void WorkflowSubject::attach(std::shared_ptr<WorkflowObserver> observer) {
    if (!observer) return;
    auto it = std::find(observers_.begin(), observers_.end(), observer);
    if (it == observers_.end()) {
        observers_.push_back(observer);
    }
}

void WorkflowSubject::detach(std::shared_ptr<WorkflowObserver> observer) {
    if (!observer) return;
    auto it = std::find(observers_.begin(), observers_.end(), observer);
    if (it != observers_.end()) {
        observers_.erase(it);
    }
}

void WorkflowSubject::notifyObservers(const WorkflowEvent& event) {
    for (const auto& observer : observers_) {
        if (observer) {
            observer->update(event);
        }
    }
}

size_t WorkflowSubject::getObserverCount() const {
    return observers_.size();
}