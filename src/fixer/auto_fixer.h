/*
 * 自动代码修复系统 (V3.0)
 * 自动应用修复建议,直接修改源代码
 *
 * 特性:
 * - 支持多种修复策略
 * - 安全的代码重写机制
 * - 备份和回滚功能
 * - 批量修复支持
 */

#pragma once

#include "report/reporter.h"
#include <string>
#include <vector>
#include <functional>
#include <optional>

namespace cpp_review {

/**
 * 代码修复操作类型
 */
enum class FixType {
    REPLACE,        // 替换文本
    INSERT,         // 插入文本
    DELETE,         // 删除文本
    ADD_INCLUDE,    // 添加 include
    REWRITE         // 完全重写
};

/**
 * 单个修复操作
 */
struct FixAction {
    FixType type;              // 修复类型
    std::string file_path;     // 文件路径
    unsigned line_start;       // 开始行
    unsigned line_end;         // 结束行
    unsigned column_start;     // 开始列
    unsigned column_end;       // 结束列
    std::string old_code;      // 原始代码
    std::string new_code;      // 修复后的代码
    std::string description;   // 修复描述
};

/**
 * 修复结果
 */
struct FixResult {
    bool success;              // 是否成功
    std::string message;       // 结果消息
    std::vector<std::string> modified_files;  // 修改的文件列表
    int fixed_count;           // 修复的问题数量
    int failed_count;          // 失败的修复数量
};

/**
 * 自动修复器
 * 根据检测到的问题自动生成和应用修复
 */
class AutoFixer {
public:
    /**
     * 构造函数
     * @param create_backup 是否创建备份文件
     */
    explicit AutoFixer(bool create_backup = true);

    /**
     * 为问题生成修复操作
     * @param issue 要修复的问题
     * @return 修复操作的可选值
     */
    std::optional<FixAction> generateFix(const Issue& issue);

    /**
     * 应用单个修复操作
     * @param fix 要应用的修复操作
     * @return 是否成功
     */
    bool applyFix(const FixAction& fix);

    /**
     * 批量修复报告中的所有问题
     * @param reporter 包含问题的报告器
     * @param filter_severity 只修复此严重性级别及以上的问题
     * @return 修复结果
     */
    FixResult fixAll(const Reporter& reporter, Severity filter_severity = Severity::LOW);

    /**
     * 设置是否启用自动备份
     * @param enable 是否启用
     */
    void setBackupEnabled(bool enable) { create_backup_ = enable; }

    /**
     * 设置是否启用交互式模式
     * @param enable 是否启用
     */
    void setInteractiveMode(bool enable) { interactive_mode_ = enable; }

    /**
     * 回滚上次修复
     * @return 是否成功
     */
    bool rollback();

private:
    bool create_backup_;              // 是否创建备份
    bool interactive_mode_;           // 是否为交互式模式
    std::vector<std::string> backup_files_;  // 备份文件列表

    // 规则特定的修复生成器
    std::optional<FixAction> fixNullPointer(const Issue& issue);
    std::optional<FixAction> fixUninitializedVar(const Issue& issue);
    std::optional<FixAction> fixAssignmentInCondition(const Issue& issue);
    std::optional<FixAction> fixUnsafeCFunction(const Issue& issue);
    std::optional<FixAction> fixMemoryLeak(const Issue& issue);
    std::optional<FixAction> fixBufferOverflow(const Issue& issue);
    std::optional<FixAction> fixIntegerOverflow(const Issue& issue);
    std::optional<FixAction> fixUseAfterFree(const Issue& issue);

    // 辅助方法
    bool createBackup(const std::string& file_path);
    bool writeFile(const std::string& file_path, const std::string& content);
    std::string readFile(const std::string& file_path);
    std::vector<std::string> readLines(const std::string& file_path);
    bool askUserConfirmation(const FixAction& fix);
};

} // namespace cpp_review
