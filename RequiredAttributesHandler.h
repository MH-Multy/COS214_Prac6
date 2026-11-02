#ifndef REQUIRED_ATTRIBUTES_HANDLER_H
#define REQUIRED_ATTRIBUTES_HANDLER_H

#include "ValidationHandler.h"
#include <vector>
#include <string>

/**
 * @brief ConcreteHandler that verifies all mandatory data fields
 *        exist on the WorkflowInstance before proceeding.
 */
class RequiredAttributesHandler : public ValidationHandler {
private:
    std::vector<std::string> keys;

protected:
    ValidationResult check(const WorkflowInstance& wf) override;
    std::string getHandlerName() const override;

public:
    explicit RequiredAttributesHandler(std::vector<std::string> requiredKeys);
    virtual ~RequiredAttributesHandler() = default;

    const std::vector<std::string>& getKeys() const;
    void addKey(const std::string& key);
};

#endif // REQUIRED_ATTRIBUTES_HANDLER_H