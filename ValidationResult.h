#ifndef VALIDATION_RESULT_H
#define VALIDATION_RESULT_H

#include <string>
#include <vector>
/**
 * @addtogroup ChainOfResponsibility
 * @{
 */
/**
 * @brief Severity classification for validation issues.
 */
enum class IssueSeverity {
    INFO,
    WARNING,
    CRITICAL_ERROR
};

/**
 * @brief Encapsulates an individual diagnostic issue discovered during validation.
 */
struct ValidationIssue {
    std::string handlerName;
    std::string message;
    IssueSeverity severity;
};

/**
 * @brief Aggregated outcome and diagnostic issue collector for the validation chain.
 */
class ValidationResult {
private:
    bool valid;
    std::vector<ValidationIssue> issues;

public:
    ValidationResult() : valid(true) {}

    /**
     * @brief Appends a diagnostic finding. CRITICAL_ERROR invalidates the result.
     * @param handler Name of the reporting handler.
     * @param msg Descriptive explanation of the issue.
     * @param sev Severity level (INFO, WARNING, CRITICAL_ERROR).
     */
    void addIssue(const std::string& handler, const std::string& msg, IssueSeverity sev) {
        issues.push_back({handler, msg, sev});
        if (sev == IssueSeverity::CRITICAL_ERROR) {
            valid = false;
        }
    }

    /**
     * @brief Merges another validation result into this one.
     * @param other Result object to aggregate.
     */
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
/**
 * @}
 */
#endif // VALIDATION_RESULT_H