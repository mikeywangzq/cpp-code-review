/*
 * 规则基类头文件
 * 定义所有分析规则必须实现的接口
 *
 * 设计模式:
 * - Rule: 策略模式 - 每个规则是一个策略
 * - RuleVisitor: 访问者模式 - 遍历 AST 并检查节点
 */

#pragma once

#include "report/reporter.h"
#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <string>
#include <memory>

namespace cpp_review {

/**
 * 规则基类
 * 所有分析规则都必须继承此类并实现虚函数
 *
 * 每个规则负责检测一种特定的代码问题
 * 例如: 空指针解引用、内存泄漏等
 */
class Rule {
public:
    virtual ~Rule() = default;

    // 获取规则 ID (如 "NULL-PTR-001")
    virtual std::string getRuleId() const = 0;

    // 获取规则名称 (如 "Null Pointer Dereference")
    virtual std::string getRuleName() const = 0;

    // 获取规则描述
    virtual std::string getDescription() const = 0;

    /**
     * 检查 AST 并报告问题
     * @param context Clang AST 上下文
     * @param reporter 用于收集问题的报告器
     */
    virtual void check(clang::ASTContext* context, Reporter& reporter) = 0;
};

/**
 * 规则访问者基类
 * 规则可以继承此类来遍历 AST
 *
 * 使用 Clang 的 RecursiveASTVisitor 递归访问所有 AST 节点
 * 子类可以重写 Visit* 方法来检查特定类型的节点
 */
class RuleVisitor : public clang::RecursiveASTVisitor<RuleVisitor> {
public:
    RuleVisitor(clang::ASTContext* context, Reporter& reporter)
        : context_(context), reporter_(reporter) {}

    virtual ~RuleVisitor() = default;

protected:
    clang::ASTContext* context_;  // AST 上下文
    Reporter& reporter_;          // 报告器

    // 辅助方法: 获取源码位置信息
    std::string getFileName(clang::SourceLocation loc) const;
    unsigned getLine(clang::SourceLocation loc) const;
    unsigned getColumn(clang::SourceLocation loc) const;
    std::string getSourceText(clang::SourceRange range) const;
};

} // namespace cpp_review
