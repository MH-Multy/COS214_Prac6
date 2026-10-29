#include "CommandInvoker.h"
#include "WorkflowAction.h"
#include <iterator>

bool CommandInvoker::executeCommand(std::shared_ptr<WorkflowAction> cmd) {
    if (!cmd || !cmd->execute()) return false;
    history_.push_back(cmd);
    return true;
}

bool CommandInvoker::undoLast(int workflowId) {
    for (std::vector<std::shared_ptr<WorkflowAction>>::reverse_iterator it = history_.rbegin();
         it != history_.rend(); ++it) {
        if ((*it)->getWorkflowId() != workflowId) continue;
        if (!(*it)->undo()) return false;
        history_.erase(std::next(it).base());
        return true;
    }
    return false;
}

std::vector<std::string> CommandInvoker::getHistory() const {
    std::vector<std::string> out;
    for (std::size_t i = 0; i < history_.size(); ++i) out.push_back(history_[i]->describe());
    return out;
}
