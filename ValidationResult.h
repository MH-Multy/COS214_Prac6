#ifndef VALIDATION_RESULT_H
#define VALIDATION_RESULT_H

#include <string>
#include <vector>

enum class IssueSeverity {
    INFO,
    WARNING,
    CRITICAL_ERROR
};

struct ValidationIssue {
    std::string handlerName;
    std::string message;
    IssueSeverity severity;
};

class ValidationResult {
private:
    bool valid;
    std::vector<ValidationIssue> issues;

public:
    ValidationResult() : valid(true) {}

    void addIssue(const std::string& handler, const std::string& msg, IssueSeverity sev) {
        issues.push_back({handler, msg, sev});
        if (sev == IssueSeverity::CRITICAL_ERROR) {
            valid = false;
        }
    }

    void merge(const ValidationResult& other) {
        if (!other.valid) {
            valid = false;
        }
        issues.insert(issues.end(), other.issues.begin(), other.issues.end());
    }

    bool isValid() const { return valid; }
    const std::vector<ValidationIssue>& getIssues() const { return issues; }

    std::string str() const {
        if (valid && issues.empty()) {
            return "[Validation SUCCESS] All stage criteria satisfied.";
        }
        std::string report = valid ? "[Validation PASSED with Warnings]:\n" 
                                   : "[Validation FAILED]:\n";
        for (const auto& issue : issues) {
            std::string tag = (issue.severity == IssueSeverity::CRITICAL_ERROR) ? "[ERROR]" : "[WARN]";
            report += "  " + tag + " (" + issue.handlerName + "): " + issue.message + "\n";
        }
        return report;
    }
};

#endif // VALIDATION_RESULT_H