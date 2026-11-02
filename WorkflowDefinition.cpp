/**
 * @file WorkflowDefinition.cpp
 * @brief Implementation of WorkflowDefinition.
 */
#include "WorkflowDefinition.h"

WorkflowDefinition::WorkflowDefinition(const std::string& name, int version)
    : name(name), version(version), approvalTimeout(0), reminderInterval(0) {}

WorkflowDefinition& WorkflowDefinition::setDescription(const std::string& text) { description = text; return *this; }
WorkflowDefinition& WorkflowDefinition::setApproverRole(const std::string& role) { approverRole = role; return *this; }
WorkflowDefinition& WorkflowDefinition::setApprovalTimeout(int seconds) { approvalTimeout = seconds < 0 ? 0 : seconds; return *this; }
WorkflowDefinition& WorkflowDefinition::setReminderInterval(int seconds) { reminderInterval = seconds; return *this; }

WorkflowDefinition& WorkflowDefinition::addRequestField(const std::string& key, const std::string& prompt) {
    requestFields.push_back(std::make_pair(key, prompt));
    return *this;
}

WorkflowDefinition& WorkflowDefinition::addStage(const std::string& stageName, const std::vector<std::string>& tasks,
                                                 bool sequential) {
    stages.push_back(std::make_pair(stageName, tasks));
    sequentialStages.push_back(sequential);
    return *this;
}

WorkflowDefinition& WorkflowDefinition::setValidationChain(std::shared_ptr<ValidationHandler> chain) { validationChain = chain; return *this; }
WorkflowDefinition& WorkflowDefinition::setFactory(std::shared_ptr<WorkflowFactory> f) { factory = f; return *this; }

std::string WorkflowDefinition::getName() const { return name; }
int WorkflowDefinition::getVersion() const { return version; }
const std::vector<std::pair<std::string, std::vector<std::string>>>& WorkflowDefinition::getStages() const { return stages; }
std::shared_ptr<ValidationHandler> WorkflowDefinition::getValidationChain() const { return validationChain; }
std::shared_ptr<WorkflowFactory> WorkflowDefinition::getFactory() const { return factory; }
std::string WorkflowDefinition::getDescription() const { return description; }
std::string WorkflowDefinition::getApproverRole() const { return approverRole; }
int WorkflowDefinition::getApprovalTimeout() const { return approvalTimeout; }
int WorkflowDefinition::getReminderInterval() const { return reminderInterval; }
const std::vector<std::pair<std::string, std::string>>& WorkflowDefinition::getRequestFields() const { return requestFields; }

bool WorkflowDefinition::isStageSequential(std::size_t index) const {
    return index < sequentialStages.size() ? sequentialStages[index] : true;
}
