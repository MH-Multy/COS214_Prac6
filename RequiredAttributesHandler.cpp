#include "RequiredAttributesHandler.h"
#include "WorkflowInstance.h"

RequiredAttributesHandler::RequiredAttributesHandler(std::vector<std::string> requiredKeys)
    : ValidationHandler(), keys(std::move(requiredKeys)) {}

std::string RequiredAttributesHandler::getHandlerName() const {
    return "RequiredAttributesHandler";
}

ValidationResult RequiredAttributesHandler::check(const WorkflowInstance& wf) {
    ValidationResult result;

    for (const auto& key : keys) {
        if (!wf.hasAttribute(key) || wf.getAttribute(key).empty()) {
            result.addIssue(
                getHandlerName(),
                "Missing or empty mandatory attribute: '" + key + "'.",
                IssueSeverity::CRITICAL_ERROR
            );
        }
    }

    return result;
}

const std::vector<std::string>& RequiredAttributesHandler::getKeys() const {
    return keys;
}

void RequiredAttributesHandler::addKey(const std::string& key) {
    keys.push_back(key);
}