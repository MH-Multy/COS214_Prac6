#ifndef WORKFLOWDEFINITION_H
#define WORKFLOWDEFINITION_H

#include <string>

/**
 * @brief Immutable description of a type of workflow.
 *
 * MINIMAL VERSION: only the name and version used by the Facade and
 * Command code. The owner of this class adds stages, the validation chain
 * and the component factory as shown in the class diagram.
 */
class WorkflowDefinition {
public:
    WorkflowDefinition(const std::string& name, int version) : name_(name), version_(version) {}

    std::string getName() const { return name_; }
    int getVersion() const { return version_; }

private:
    std::string name_;
    int version_;
};

#endif
