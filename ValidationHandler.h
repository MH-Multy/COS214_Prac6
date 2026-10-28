#ifndef VALIDATION_HANDLER_H
#define VALIDATION_HANDLER_H

#include <memory>
#include <string>
#include "ValidationResult.h"

// Forward declaration of the engine instance
class WorkflowInstance;

enum class ChainExecutionMode {
    FAIL_FAST,
    AUDIT_ACCUMULATE
};

class ValidationHandler {
protected:
    std::shared_ptr<ValidationHandler> next;

    virtual ValidationResult check(const WorkflowInstance& wf) = 0;
    virtual std::string getHandlerName() const = 0;

public:
    ValidationHandler() : next(nullptr) {}
    virtual ~ValidationHandler() = default;
    void add(std::shared_ptr<ValidationHandler> successor) {
        if (next) {
            next->add(successor);
        } else {
            next = successor;
        }
    }

    virtual bool canHandle(const WorkflowInstance&) const {
    return true; 
    }

    std::shared_ptr<ValidationHandler> setNext(std::shared_ptr<ValidationHandler> successor) {
        next = successor;
        return successor;
    }

    virtual ValidationResult handle(const WorkflowInstance& wf, ChainExecutionMode mode = ChainExecutionMode::AUDIT_ACCUMULATE) {
      ValidationResult current;
    if (this->canHandle(wf)) {
        current = this->check(wf);
        if (mode == ChainExecutionMode::FAIL_FAST && !current.isValid()) {
            return current;
        }
    }

    if (next != nullptr) {
        ValidationResult nextResult = next->handle(wf, mode);
        current.merge(nextResult);
    }

    return current;
    }
};

#endif