/**
 * @file WorkflowDefinition.h
 * @brief Immutable blueprint for one type of workflow.
 */
#ifndef WORKFLOWDEFINITION_H
#define WORKFLOWDEFINITION_H

#include <memory>
#include <string>
#include <utility>
#include <vector>

class ValidationHandler;
class WorkflowFactory;

/**
 * @brief Immutable blueprint for one type of workflow, e.g. "Lab Test Request".
 * @ingroup core
 *
 * A definition describes what a workflow is allowed to do; it holds no data
 * about any particular request. Everything that differs between industries is
 * configuration here, so the engine never names an industry.
 *
 * Stages are pairs of (stage name, task list). Each task is written as
 * "Task name (Role)", for example "Run test (Requester)". The role "Requester"
 * means whoever submitted the request.
 *
 * The setters are used once, while configuring, before the definition is
 * registered; each returns the definition so calls can be chained.
 */
class WorkflowDefinition {
public:
    /**
     * @param name    Workflow type name shown to users; also its registration key.
     * @param version Definition version.
     */
    WorkflowDefinition(const std::string& name, int version);

    /// @name Configuration
    /// @{
    WorkflowDefinition& setDescription(const std::string& text);
    /** @brief Role allowed to approve, deny, suspend and resume. */
    WorkflowDefinition& setApproverRole(const std::string& role);
    /** @brief Seconds before an unanswered request is auto-approved (auto-approval regime only). */
    WorkflowDefinition& setApprovalTimeout(int seconds);
    WorkflowDefinition& setReminderInterval(int seconds);
    /** @brief A request field the user interface asks for. @param key Attribute key. @param prompt Question shown. */
    WorkflowDefinition& addRequestField(const std::string& key, const std::string& prompt);
    /**
     * @brief Append a stage. Stages always run in order.
     * @param name       Stage name.
     * @param tasks      Tasks as "Task name (Role)".
     * @param sequential Whether the stage's tasks must be done in the listed order.
     */
    WorkflowDefinition& addStage(const std::string& name, const std::vector<std::string>& tasks,
                                 bool sequential = true);
    WorkflowDefinition& setValidationChain(std::shared_ptr<ValidationHandler> chain);
    WorkflowDefinition& setFactory(std::shared_ptr<WorkflowFactory> factory);
    /// @}

    std::string getName() const;
    int getVersion() const;
    const std::vector<std::pair<std::string, std::vector<std::string>>>& getStages() const;
    std::shared_ptr<ValidationHandler> getValidationChain() const;
    std::shared_ptr<WorkflowFactory> getFactory() const;

    std::string getDescription() const;
    std::string getApproverRole() const;
    int getApprovalTimeout() const;
    int getReminderInterval() const;
    /** @brief Fields to ask for, as (key, prompt) pairs. */
    const std::vector<std::pair<std::string, std::string>>& getRequestFields() const;
    /** @brief Whether stage number @p index runs its tasks in order. */
    bool isStageSequential(std::size_t index) const;

private:
    std::string name;
    int version;
    std::string description;
    std::string approverRole;
    int approvalTimeout;
    int reminderInterval;
    std::vector<std::pair<std::string, std::vector<std::string>>> stages;
    std::vector<bool> sequentialStages;
    std::vector<std::pair<std::string, std::string>> requestFields;
    std::shared_ptr<ValidationHandler> validationChain;
    std::shared_ptr<WorkflowFactory> factory;
};

#endif
