/*
 * Git 集成模块头文件
 * 支持增量分析和 PR 自动审查
 *
 * 功能:
 * - 检测 Git 仓库中的变更文件
 * - 支持多种增量模式 (工作区/分支/提交)
 * - PR 环境检测 (GitHub Actions, GitLab CI)
 * - 生成 PR 格式的审查报告
 */

#pragma once

#include <string>
#include <vector>
#include <optional>

namespace cpp_review {

/**
 * 增量分析模式
 */
enum class IncrementalMode {
    WORKSPACE,      // 工作区未提交的更改 (git diff)
    STAGED,         // 暂存区的更改 (git diff --cached)
    BRANCH,         // 与指定分支的差异
    COMMIT,         // 从指定提交以来的更改
    PR              // PR 模式 (自动检测 base 分支)
};

/**
 * PR 环境信息
 */
struct PREnvironment {
    std::string provider;        // "github" 或 "gitlab"
    std::string base_branch;     // PR 的基础分支
    std::string head_branch;     // PR 的头部分支
    std::string pr_number;       // PR 编号
    std::string repo_owner;      // 仓库所有者
    std::string repo_name;       // 仓库名称
    bool is_pr_environment;      // 是否在 PR 环境中
};

/**
 * Git 集成类
 * 提供 Git 相关的实用功能
 */
class GitIntegration {
public:
    /**
     * 检查当前目录是否为 Git 仓库
     * @return 如果是 Git 仓库返回 true
     */
    static bool isGitRepository();

    /**
     * 获取变更的 C++ 源文件列表
     * @param mode 增量分析模式
     * @param reference 参考分支/提交 (可选)
     * @return 变更文件的路径列表
     */
    static std::vector<std::string> getChangedFiles(
        IncrementalMode mode = IncrementalMode::WORKSPACE,
        const std::string& reference = ""
    );

    /**
     * 检测 PR 环境
     * 自动识别 GitHub Actions, GitLab CI 等
     * @return PR 环境信息 (如果在 PR 环境中)
     */
    static std::optional<PREnvironment> detectPREnvironment();

    /**
     * 获取当前分支名称
     * @return 分支名称
     */
    static std::string getCurrentBranch();

    /**
     * 获取默认主分支名称
     * 尝试检测 main 或 master
     * @return 主分支名称
     */
    static std::string getDefaultBranch();

    /**
     * 过滤出 C++ 源文件
     * @param files 文件路径列表
     * @return 仅包含 C++ 源文件的列表
     */
    static std::vector<std::string> filterCppFiles(const std::vector<std::string>& files);

    /**
     * 生成 PR 格式的审查报告
     * @param report_content 原始报告内容
     * @param pr_env PR 环境信息
     * @return PR 评论格式的报告
     */
    static std::string generatePRComment(
        const std::string& report_content,
        const PREnvironment& pr_env
    );

private:
    /**
     * 执行 Git 命令并返回输出
     * @param command Git 命令
     * @return 命令输出 (去除首尾空白)
     */
    static std::string executeGitCommand(const std::string& command);

    /**
     * 检查文件是否为 C++ 源文件
     * @param path 文件路径
     * @return 如果是 C++ 源文件返回 true
     */
    static bool isCppFile(const std::string& path);

    /**
     * 从环境变量获取值
     * @param var_name 环境变量名
     * @return 环境变量值 (如果存在)
     */
    static std::optional<std::string> getEnvVar(const std::string& var_name);
};

} // namespace cpp_review
