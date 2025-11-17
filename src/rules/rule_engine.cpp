#include "rules/rule_engine.h"
#include <iostream>

namespace cpp_review {

RuleEngine::RuleEngine() = default;

void RuleEngine::registerRule(std::unique_ptr<Rule> rule) {
    rules_.push_back(std::move(rule));
}

void RuleEngine::runAllRules(clang::ASTContext* context, Reporter& reporter) {
    for (auto& rule : rules_) {
        try {
            rule->check(context, reporter);
        } catch (const std::exception& e) {
            std::cerr << "Error running rule " << rule->getRuleId()
                     << ": " << e.what() << "\n";
        }
    }
}

} // namespace cpp_review
