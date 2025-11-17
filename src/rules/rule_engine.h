#pragma once

#include "rules/rule.h"
#include "report/reporter.h"
#include <clang/AST/ASTContext.h>
#include <vector>
#include <memory>

namespace cpp_review {

class RuleEngine {
public:
    RuleEngine();

    void registerRule(std::unique_ptr<Rule> rule);
    void runAllRules(clang::ASTContext* context, Reporter& reporter);

    size_t getRuleCount() const { return rules_.size(); }

private:
    std::vector<std::unique_ptr<Rule>> rules_;
};

} // namespace cpp_review
