#ifndef WORKFLOWINSTANCE_H
#define WORKFLOWINSTANCE_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ReportHistory.h"
#include "WorkflowEvent.h"
#include "WorkflowReport.h"
#include "WorkflowRequest.h"

class WorkflowDefinition;
class WorkItem;

/**
 *  One independent run of a workflow definition.
 *
 * MINIMAL VERSION. The public interface matches the class diagram and is
 * what the Command and Facade code depends on. The bodies of the lifecycle
 * operations are placeholders: the State pattern owner replaces them with
 * delegation (e.g. approve() becomes currentState_->approve(*this, actorId))
 * and the Composite/Iterator owners replace the item bookkeeping.
 * Keep the signatures so the Command and Facade code keeps compiling.
 *
 * Memento role: the instance owns its WorkflowReport (Originator) and the
 * ReportHistory (Caretaker) for that report, so report revisions are
 * isolated per instance.
 */
class WorkflowInstance 
{
public:
    WorkflowInstance(int id, std::shared_ptr<const WorkflowDefinition> definition,
                     const WorkflowRequest& request);

    int getId() const { return id_; }
    std::shared_ptr<const WorkflowDefinition> getDefinition() const { return definition_; }
    const WorkflowRequest& getRequest() const { return request_; }
    std::string getStateName() const { return state_; }

    // ---- lifecycle (each returns false if not allowed in the current state)
    bool validate();
    bool approve(int actorId);
    bool reject(int actorId, const std::string& reason);
    bool start();
    bool suspend(int actorId);
    bool resume(int actorId);
    bool complete();

    // ---- work items
    bool completeItem(int itemId, int actorId);
    bool reopenItem(int itemId, int actorId);
    std::vector<std::shared_ptr<WorkItem>> getAvailableWork() const;

    // ---- history
    void addEvent(const WorkflowEvent& e);
    const std::vector<WorkflowEvent>& getHistory() const { return history_; }

    // ---- report (Memento)
    WorkflowReport& getReport() { return report_; }
    const WorkflowReport& getReport() const { return report_; }
    ReportHistory& getReportHistory() { return reportHistory_; }
    const ReportHistory& getReportHistory() const { return reportHistory_; }

    /** @brief PLACEHOLDER until the Composite tree exists: register a work item id. */
    void addPlaceholderItem(int itemId, const std::string& name);

private:
    bool moveTo(const std::string& next, const std::string& eventType, int actorId,
                const std::string& message);

    int id_;
    std::shared_ptr<const WorkflowDefinition> definition_;
    WorkflowRequest request_;
    std::string state_;
    std::string resumeTo_;
    std::map<int, std::pair<std::string, bool> > items_;   // id -> (name, done)
    std::vector<WorkflowEvent> history_;
    WorkflowReport report_;
    ReportHistory reportHistory_;
};

#endif
