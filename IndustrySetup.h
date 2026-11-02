/**
 * @file IndustrySetup.h
 * @brief Application configuration: the industries TIVIDY runs.
 */
#ifndef INDUSTRYSETUP_H
#define INDUSTRYSETUP_H

#include <string>
#include <utility>
#include <vector>

class WorkflowFacade;

/** @brief Register the laboratory testing industry (auto-approval after @p approvalTimeoutSeconds). */
void configureLabTesting(WorkflowFacade& facade, int approvalTimeoutSeconds = 30);

/** @brief Register the loan application industry (manual review, four-eyes). */
void configureLoanApplications(WorkflowFacade& facade);

/** @brief Register both industries. */
void configureAllIndustries(WorkflowFacade& facade, int labTimeoutSeconds = 30);

/**
 * @brief Register a new workflow type described at run time (e.g. typed in during the demo).
 *
 * Built entirely from the same generic engine classes as the two industries:
 * a validation chain (required fields, identity check through a new records
 * system behind the Adapter, and an optional numeric limit), stages for the
 * Composite work tree, and a factory choosing the approval regime.
 *
 * @param facade             The system to configure.
 * @param name               Workflow type name (must not already exist).
 * @param description        One-line description.
 * @param approverRole       Role that approves, denies, suspends and resumes.
 * @param autoApproveSeconds Seconds before auto-approval, or -1 for manual review (never automatic).
 * @param fields             Request fields; all are required.
 * @param limitField         Numeric field with a maximum, or "" for none.
 * @param limitMax           The maximum allowed for @p limitField.
 * @param stages             (stage name, tasks as "Task name (Role)") pairs.
 * @param sequential         For each stage, whether its tasks run in order.
 * @throws std::invalid_argument if the name is empty or already used, or there is no task.
 */
void configureCustomWorkflow(WorkflowFacade& facade, const std::string& name, const std::string& description,
                             const std::string& approverRole, int autoApproveSeconds,
                             const std::vector<std::string>& fields, const std::string& limitField, double limitMax,
                             const std::vector<std::pair<std::string, std::vector<std::string>>>& stages,
                             const std::vector<bool>& sequential);

#endif
