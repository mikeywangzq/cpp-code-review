/*
 * 规则引擎实现
 * 管理规则的注册和执行流程
 */

#include "rules/rule_engine.h"
#include <iostream>

namespace cpp_review {

// 默认构造函数
RuleEngine::RuleEngine() = default;

/**
 * 注册新规则
 * 使用移动语义转移规则所有权
 */
void RuleEngine::registerRule(std::unique_ptr<Rule> rule) {
    rules_.push_back(std::move(rule));
}

/**
 * 依次运行所有已注册的规则
 * 每个规则独立运行,一个规则失败不影响其他规则
 */
void RuleEngine::runAllRules(clang::ASTContext* context, Reporter& reporter) {
    for (auto& rule : rules_) {
        try {
            // 运行单个规则的检查逻辑
            rule->check(context, reporter);
        } catch (const std::exception& e) {
            // 捕获异常,避免单个规则错误导致整个分析失败
            std::cerr << "Error running rule " << rule->getRuleId()
                     << ": " << e.what() << "\n";
        }
    }
}

} // namespace cpp_review
