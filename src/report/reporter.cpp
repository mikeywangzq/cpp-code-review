#include "report/reporter.h"
#include <iostream>
#include <iomanip>
#include <algorithm>

namespace cpp_review {

void Reporter::addIssue(const Issue& issue) {
    issues_.push_back(issue);
}

size_t Reporter::getCriticalCount() const {
    return std::count_if(issues_.begin(), issues_.end(),
                        [](const Issue& issue) {
                            return issue.severity == Severity::CRITICAL;
                        });
}

std::string Reporter::severityToString(Severity severity) const {
    switch (severity) {
        case Severity::CRITICAL: return "CRITICAL";
        case Severity::HIGH: return "HIGH";
        case Severity::MEDIUM: return "MEDIUM";
        case Severity::LOW: return "LOW";
        case Severity::SUGGESTION: return "SUGGESTION";
    }
    return "UNKNOWN";
}

std::string Reporter::getSeverityColor(Severity severity) const {
    // ANSI color codes
    switch (severity) {
        case Severity::CRITICAL: return "\033[1;31m"; // Bold Red
        case Severity::HIGH: return "\033[0;31m";     // Red
        case Severity::MEDIUM: return "\033[0;33m";   // Yellow
        case Severity::LOW: return "\033[0;36m";      // Cyan
        case Severity::SUGGESTION: return "\033[0;32m"; // Green
    }
    return "\033[0m"; // Reset
}

void Reporter::generateReport(std::ostream& out) const {
    const std::string reset = "\033[0m";

    out << "\n";
    out << "╔══════════════════════════════════════════════════════════════════════╗\n";
    out << "║         C++ Code Review Report - Analysis Complete              ║\n";
    out << "╚══════════════════════════════════════════════════════════════════════╝\n";
    out << "\n";

    // Summary statistics
    std::map<Severity, size_t> severity_counts;
    for (const auto& issue : issues_) {
        severity_counts[issue.severity]++;
    }

    out << "Summary:\n";
    out << "  Total issues found: " << issues_.size() << "\n";
    out << "  - Critical: " << severity_counts[Severity::CRITICAL] << "\n";
    out << "  - High: " << severity_counts[Severity::HIGH] << "\n";
    out << "  - Medium: " << severity_counts[Severity::MEDIUM] << "\n";
    out << "  - Low: " << severity_counts[Severity::LOW] << "\n";
    out << "  - Suggestions: " << severity_counts[Severity::SUGGESTION] << "\n";
    out << "\n";

    if (issues_.empty()) {
        out << "✓ No issues found! Your code looks good.\n";
        return;
    }

    out << "Detailed Issues:\n";
    out << "═══════════════════════════════════════════════════════════════════════\n";

    for (size_t i = 0; i < issues_.size(); ++i) {
        const auto& issue = issues_[i];
        const std::string color = getSeverityColor(issue.severity);

        out << "\n[Issue #" << (i + 1) << "]\n";
        out << "Location: " << issue.file_path << ":" << issue.line << ":" << issue.column << "\n";
        out << "Severity: " << color << severityToString(issue.severity) << reset << "\n";
        out << "Rule ID: " << issue.rule_id << "\n";
        out << "Description: " << issue.description << "\n";

        if (!issue.code_snippet.empty()) {
            out << "Code:\n";
            out << "  " << issue.code_snippet << "\n";
        }

        if (!issue.suggestion.empty()) {
            out << "Suggestion: " << issue.suggestion << "\n";
        }

        out << "───────────────────────────────────────────────────────────────────────\n";
    }

    out << "\nAnalysis complete. Please review and fix the issues above.\n";
}

} // namespace cpp_review
