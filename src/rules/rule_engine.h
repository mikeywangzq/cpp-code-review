/*
 * 规则引擎头文件
 * 管理和运行所有代码分析规则
 */

#pragma once

#include "rules/rule.h"
#include "report/reporter.h"
#include <clang/AST/ASTContext.h>
#include <vector>
#include <memory>

namespace cpp_review {

/**
 * 规则引擎类
 * 负责注册、管理和执行所有分析规则
 * 采用策略模式,支持动态添加新规则
 */
class RuleEngine {
public:
    RuleEngine();

    /**
     * 注册一个新的分析规则
     * @param rule 规则的唯一指针 (转移所有权)
     */
    void registerRule(std::unique_ptr<Rule> rule);

    /**
     * 在给定的 AST 上运行所有注册的规则
     * @param context Clang AST 上下文
     * @param reporter 用于收集问题的报告器
     */
    void runAllRules(clang::ASTContext* context, Reporter& reporter);

    /**
     * 获取已注册的规则数量
     * @return 规则总数
     */
    size_t getRuleCount() const { return rules_.size(); }

private:
    std::vector<std::unique_ptr<Rule>> rules_;  // 所有已注册的规则列表
};

} // namespace cpp_review
