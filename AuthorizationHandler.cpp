#include "AuthorizationHandler.h"
#include "WorkflowInstance.h"

AuthorizationHandler::AuthorizationHandler(std::shared_ptr<OrganizationDirectory> dir, std::string act)
    : ValidationHandler(), directory(std::move(dir)), activity(std::move(act)) {}

std::string AuthorizationHandler::getHandlerName() const {
    return "AuthorizationHandler[" + activity + "]";
}

bool AuthorizationHandler::canHandle(const WorkflowInstance& wf) const {
    return !activity.empty() && wf.getRequest().getRequesterId() > 0;
}

ValidationResult AuthorizationHandler::check(const WorkflowInstance& wf) {
    ValidationResult result;
    int actorId = wf.getRequest().getRequesterId();

    if (!directory) {
        result.addIssue(
            getHandlerName(),
            "OrganizationDirectory adapter service is unavailable.",
            IssueSeverity::CRITICAL_ERROR
        );
        return result;
    }

    if (!directory->isAuthorized(actorId, activity)) {
        result.addIssue(
            getHandlerName(),
            "Actor ID " + std::to_string(actorId) + 
            " is not authorized to perform required activity: '" + activity + "'.",
            IssueSeverity::CRITICAL_ERROR
        );
    }

    return result;
}

const std::string& AuthorizationHandler::getActivity() const {
    return activity;
}

void AuthorizationHandler::setActivity(const std::string& act) {
    activity = act;
}

std::shared_ptr<OrganizationDirectory> AuthorizationHandler::getDirectory() const {
    return directory;
}