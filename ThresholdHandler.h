#ifndef THRESHOLD_HANDLER_H
#define THRESHOLD_HANDLER_H

#include "ValidationHandler.h"
#include <string>

/**
 * @brief ConcreteHandler validating that numeric attributes do not exceed
 *        a prescribed maximum boundary (e.g., risk levels, loan amounts, temperatures).
 */
class ThresholdHandler : public ValidationHandler {
private:
    std::string attribute;
    double maxValue;

protected:
    ValidationResult check(const WorkflowInstance& wf) override;
    bool canHandle(const WorkflowInstance& wf) const override;
    std::string getHandlerName() const override;

public:
    ThresholdHandler(std::string attr, double maxVal);
    virtual ~ThresholdHandler() = default;

    const std::string& getAttribute() const;
    double getMaxValue() const;
    void setMaxValue(double maxVal);
};

#endif // THRESHOLD_HANDLER_H