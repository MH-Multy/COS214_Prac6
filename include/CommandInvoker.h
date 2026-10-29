#ifndef COMMANDINVOKER_H
#define COMMANDINVOKER_H

#include <memory>
#include <string>
#include <vector>

class WorkflowAction;

/**
 *  Command Invoker: runs commands and remembers the successful ones.
 *
 * The history stores command objects, because only an object can
 * be undone. Undo is per workflow: undoLast(id) reverses the most recent
 * action on that instance only, so undoing in one workflow can never change
 * another one (instance isolation). If that most recent action is not
 * reversible (an approval or rejection), undo is refused rather than skipping
 * past the decision to something older.
 */
class CommandInvoker 
{
public:
    /** record it only if it succeeded. */
    bool executeCommand(std::shared_ptr<WorkflowAction> cmd);

    /**  Undo the most recent action on */
    bool undoLast(int workflowId);

    /** Descriptions of every recorded action, oldest first. */
    std::vector<std::string> getHistory() const;

private:
    std::vector<std::shared_ptr<WorkflowAction>> history_;
};

#endif
