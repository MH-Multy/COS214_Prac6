#include "AuditLogger.h"
#include <iostream>

void AuditLogger::update(const WorkflowEvent& event) {
    auditLog_.push_back(event.toString());
}

const std::vector<std::string>& AuditLogger::getAuditLog() const {
    return auditLog_;
}

void AuditLogger::printLog() const {
    std::cout << "--- System Audit Log ---" << std::endl;
    for (const auto& entry : auditLog_) {
        std::cout << entry << std::endl;
    }
}

size_t AuditLogger::getLogSize() const {
    return auditLog_.size();
}