/**
 * @file ValidationHandler.h
 * @brief Base Handler participant in the Chain of Responsibility pattern.
 * 
 */
/**
 * @defgroup ChainOfResponsibility
 * @brief Dynamic validation pipeline and automated invariant safeguard handlers.
 * @{
 * @}
 */

#ifndef VALIDATION_HANDLER_H
#define VALIDATION_HANDLER_H

#include <memory>
#include <string>
#include "ValidationResult.h"
/**
 * @addtogroup ChainOfResponsibility
 * @{
 */

class WorkflowInstance;
enum class ChainExecutionMode {
    FAIL_FAST,
    AUDIT_ACCUMULATE
};

/**
 * @class ValidationHandler

 * Implements the GoF Chain of Responsibility pattern for workflow invariant checking.
 * Provides dynamic link chaining via setNext() and recursive add(), and defines
 * the template method handle() with a polymorphic check() hook.
 */
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

    /**
     * @brief Selective participation hook.
     * @return True if this handler applies to the request; false to bypass.
     */
    virtual bool canHandle(const WorkflowInstance&) const {
    return true; 
    }

    std::shared_ptr<ValidationHandler> setNext(std::shared_ptr<ValidationHandler> successor) {
        next = successor;
        return successor;
    }

    /**
     * @brief Template execution method dispatching the request down the chain.
     * 
     */
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
/**
 * @}
 */
#endif