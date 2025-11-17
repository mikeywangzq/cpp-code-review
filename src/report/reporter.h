#pragma once

#include <string>
#include <vector>
#include <map>

namespace cpp_review {

enum class Severity {
    CRITICAL,
    HIGH,
    MEDIUM,
    LOW,
    SUGGESTION
};

struct Issue {
    std::string file_path;
    unsigned line;
    unsigned column;
    Severity severity;
    std::string rule_id;
    std::string description;
    std::string suggestion;
    std::string code_snippet;
};

class Reporter {
public:
    void addIssue(const Issue& issue);
    void generateReport(std::ostream& out) const;
    size_t getIssueCount() const { return issues_.size(); }
    size_t getCriticalCount() const;

private:
    std::vector<Issue> issues_;

    std::string severityToString(Severity severity) const;
    std::string getSeverityColor(Severity severity) const;
};

} // namespace cpp_review
