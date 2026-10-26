#include "IdentityVerificationHandler.h"
#include "WorkflowInstance.h"

IdentityVerificationHandler::IdentityVerificationHandler(std::shared_ptr<OrganizationDirectory> dir)
    : ValidationHandler(), directory(std::move(dir)) {}

std::string IdentityVerificationHandler::getHandlerName() const {
    return "IdentityVerificationHandler";
}

bool IdentityVerificationHandler::canHandle(const WorkflowInstance& wf) const {
    return wf.getCurrentActorId() > 0;
}

ValidationResult IdentityVerificationHandler::check(const WorkflowInstance& wf) {
    ValidationResult result;
    int actorId = wf.getCurrentActorId();
    if (!directory) {
        result.addIssue(
            getHandlerName(),
            "OrganizationDirectory adapter service is unavailable or unconfigured.",
            IssueSeverity::CRITICAL_ERROR
        );
        return result;
    }

    if (!directory->userExists(actorId)) {
        result.addIssue(
            getHandlerName(),
            "Participant ID " + std::to_string(actorId) + 
            " is not registered or active in the organization directory.",
            IssueSeverity::CRITICAL_ERROR
        );
    }

    return result;
}

std::shared_ptr<OrganizationDirectory> IdentityVerificationHandler::getDirectory() const {
    return directory;
}