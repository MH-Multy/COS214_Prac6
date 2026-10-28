#ifndef AUDIT_LOGGER_H
#define AUDIT_LOGGER_H

#include "WorkflowObserver.h"
#include <vector>
#include <string>
/**
 * @addtogroup Observer
 * @{
 */
/**
 * @brief Concrete Observer recording an immutable audit trail of workflow events.
 */
class AuditLogger : public WorkflowObserver {
private:
    std::vector<std::string> auditLog_;

public:
    AuditLogger() = default;
    virtual ~AuditLogger() = default;

    /**
     * @brief Appends the stringified event entry to the audit trail.
     */
    void update(const WorkflowEvent& event) override;

    const std::vector<std::string>& getAuditLog() const;
    void printLog() const;
    size_t getLogSize() const;
};
/**
 * @}
 */
#endif // AUDIT_LOGGER_H