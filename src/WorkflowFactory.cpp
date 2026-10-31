/**
 * @file WorkflowFactory.cpp
 * @brief Implementation of the approval-regime component factories.
 */
#include "WorkflowFactory.h"

#include "ApprovalPolicy.h"
#include "CompositeWorkItem.h"
#include "Task.h"
#include "WorkflowDefinition.h"

namespace {
/** Split "Run test (Requester)" into name "Run test" and role "Requester". */
void parseTask(const std::string& text, std::string& name, std::string& role) 
{
    std::size_t open = text.rfind('(');
    std::size_t close = text.rfind(')');
    if (open == std::string::npos || close == std::string::npos || close < open) 
    {
        name = text;
        role = "";
        return;
    }
    name = text.substr(0, open);
    while (!name.empty() && name[name.size() - 1] == ' ') name.erase(name.size() - 1);
    role = text.substr(open + 1, close - open - 1);
}

/** Shared by both factories: a sequential root holding one group per configured stage. */
std::shared_ptr<CompositeWorkItem> buildStages(const WorkflowDefinition& def, int& nextId) 
{
    std::shared_ptr<CompositeWorkItem> root = std::make_shared<CompositeWorkItem>(nextId++, def.getName(), true);
    const std::vector<std::pair<std::string, std::vector<std::string>>>& stages = def.getStages();
    for (std::size_t s = 0; s < stages.size(); ++s) 
    {
        std::shared_ptr<CompositeWorkItem> stage =
            std::make_shared<CompositeWorkItem>(nextId++, stages[s].first, def.isStageSequential(s));
        for (std::size_t t = 0; t < stages[s].second.size(); ++t) 
        {
            std::string name, role;
            parseTask(stages[s].second[t], name, role);
            stage->add(std::make_shared<Task>(nextId++, name, role));
        }
        root->add(stage);
    }
    return root;
}
}  // namespace

WorkflowFactory::~WorkflowFactory() {}

// ---------------- AutoApprovalComponentFactory
std::shared_ptr<ApprovalPolicy> AutoApprovalComponentFactory::createApprovalPolicy(const WorkflowDefinition& def) const {
    return std::make_shared<TimeoutApprovalPolicy>(def.getApproverRole(), def.getApprovalTimeout());
}

std::shared_ptr<CompositeWorkItem> AutoApprovalComponentFactory::createTaskGroup(const WorkflowDefinition& def) const {
    int nextId = 1;
    return buildStages(def, nextId);
}

// ---------------- ManualReviewComponentFactory
std::shared_ptr<ApprovalPolicy> ManualReviewComponentFactory::createApprovalPolicy(const WorkflowDefinition& def) const {
    return std::make_shared<StrictApprovalPolicy>(def.getApproverRole());
}

std::shared_ptr<CompositeWorkItem> ManualReviewComponentFactory::createTaskGroup(const WorkflowDefinition& def) const 
{
    int nextId = 1;
    std::shared_ptr<CompositeWorkItem> root = buildStages(def, nextId);
    // Part of this family: nothing is auto-approved, so an approver signs the work off at the end.
    std::shared_ptr<CompositeWorkItem> review = std::make_shared<CompositeWorkItem>(nextId++, "Final review", true);
    review->add(std::make_shared<Task>(nextId++, "Compliance sign-off", def.getApproverRole()));
    root->add(review);
    return root;
}
