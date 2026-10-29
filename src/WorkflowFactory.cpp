#include "WorkflowFactory.h"

std::shared_ptr<CompositeWorkItem> WorkflowFactory::buildGroup(const WorkflowDefinition& def, bool tasksSequential) {
    std::shared_ptr<CompositeWorkItem> root(new CompositeWorkItem(def.getName(), true));
    const std::vector<std::pair<std::string, std::vector<std::string> > >& st = def.getStages();
    for (size_t s = 0; s < st.size(); ++s) {
        std::shared_ptr<CompositeWorkItem> stage(new CompositeWorkItem(st[s].first, tasksSequential));
        for (size_t t = 0; t < st[s].second.size(); ++t)
            stage->add(std::shared_ptr<WorkItem>(new Task(st[s].second[t])));
        root->add(stage);
    }
    return root;
}

std::shared_ptr<ApprovalPolicy> ManualReviewComponentFactory::createApprovalPolicy(const WorkflowDefinition& def) const {
    return std::shared_ptr<ApprovalPolicy>(new StrictApprovalPolicy(def.getApproverRole()));
}
std::shared_ptr<CompositeWorkItem> ManualReviewComponentFactory::createTaskGroup(const WorkflowDefinition& def) const {
    return buildGroup(def, true);
}

std::shared_ptr<ApprovalPolicy> AutoApprovalComponentFactory::createApprovalPolicy(const WorkflowDefinition& def) const {
    return std::shared_ptr<ApprovalPolicy>(new TimeoutApprovalPolicy(def.getApproverRole(), def.getApprovalTimeout()));
}
std::shared_ptr<CompositeWorkItem> AutoApprovalComponentFactory::createTaskGroup(const WorkflowDefinition& def) const {
    return buildGroup(def, false);
}