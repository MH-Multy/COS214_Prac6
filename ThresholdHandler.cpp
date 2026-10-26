#include "ThresholdHandler.h"
#include "WorkflowInstance.h"
#include <string>
#include <sstream>

ThresholdHandler::ThresholdHandler(std::string attr, double maxVal)
    : ValidationHandler(), attribute(std::move(attr)), maxValue(maxVal) {}

std::string ThresholdHandler::getHandlerName() const {
    return "ThresholdHandler[" + attribute + "]";
}

bool ThresholdHandler::canHandle(const WorkflowInstance& wf) const {
    return wf.hasAttribute(attribute);
}

ValidationResult ThresholdHandler::check(const WorkflowInstance& wf) {
    ValidationResult result;
    std::string rawVal = wf.getAttribute(attribute);
    std::stringstream inStream(rawVal);
    double val = 0.0;
    if (!(inStream >> val)) {
        result.addIssue(
            getHandlerName(),
            "Attribute '" + attribute + "' cannot be parsed as a valid numeric value.",
            IssueSeverity::CRITICAL_ERROR
        );
        return result;
    }

    if (val > maxValue) {
        std::stringstream outStream;
        outStream << maxValue;
        result.addIssue(
            getHandlerName(),
            "Attribute '" + attribute + "' value (" + rawVal + 
            ") exceeds maximum allowable threshold (" + outStream.str() + ").",
            IssueSeverity::CRITICAL_ERROR
        );
    }

    return result;
}

const std::string& ThresholdHandler::getAttribute() const {
    return attribute;
}

double ThresholdHandler::getMaxValue() const {
    return maxValue;
}

void ThresholdHandler::setMaxValue(double maxVal) {
    maxValue = maxVal;
}