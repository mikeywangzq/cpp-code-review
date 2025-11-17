/*
 * 报告生成器头文件
 * 定义问题结构和报告生成接口
 */

#pragma once

#include <string>
#include <vector>
#include <map>

namespace cpp_review {

/**
 * 严重性级别枚举
 * 从高到低: CRITICAL -> HIGH -> MEDIUM -> LOW -> SUGGESTION
 */
enum class Severity {
    CRITICAL,    // 严重: 会导致程序崩溃或安全漏洞
    HIGH,        // 高: 可能导致未定义行为
    MEDIUM,      // 中: 性能或代码质量问题
    LOW,         // 低: 次要问题
    SUGGESTION   // 建议: 改进建议
};

/**
 * 问题结构
 * 存储单个检测到的代码问题的所有信息
 */
struct Issue {
    std::string file_path;      // 文件路径
    unsigned line;              // 行号
    unsigned column;            // 列号
    Severity severity;          // 严重性级别
    std::string rule_id;        // 规则 ID (如 NULL-PTR-001)
    std::string description;    // 问题描述
    std::string suggestion;     // 修复建议
    std::string code_snippet;   // 代码片段 (可选)
};

/**
 * 报告生成器类
 * 收集所有问题并生成格式化的报告
 */
class Reporter {
public:
    // 添加一个问题到报告
    void addIssue(const Issue& issue);

    // 生成控制台报告
    void generateReport(std::ostream& out) const;

    // 获取问题总数
    size_t getIssueCount() const { return issues_.size(); }

    // 获取严重问题数量
    size_t getCriticalCount() const;

    // 获取所有问题的只读访问
    const std::vector<Issue>& getIssues() const { return issues_; }

    // 严重性转字符串
    std::string severityToString(Severity severity) const;

    // 获取严重性对应的颜色代码
    std::string getSeverityColor(Severity severity) const;

private:
    std::vector<Issue> issues_;  // 所有检测到的问题列表
};

} // namespace cpp_review
