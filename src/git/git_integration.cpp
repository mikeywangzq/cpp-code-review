/*
 * Git 集成模块实现
 * 实现增量分析和 PR 审查功能
 */

#include "git/git_integration.h"
#include <cstdlib>
#include <array>
#include <memory>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <iostream>

namespace cpp_review {

namespace fs = std::filesystem;

/**
 * 执行 Git 命令并返回输出
 */
std::string GitIntegration::executeGitCommand(const std::string& command) {
    std::array<char, 128> buffer;
    std::string result;

    // 使用 popen 执行命令
    std::unique_ptr<FILE, decltype(&pclose)> pipe(
        popen(command.c_str(), "r"), pclose
    );

    if (!pipe) {
        return "";
    }

    // 读取命令输出
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    // 去除首尾空白
    result.erase(0, result.find_first_not_of(" \t\n\r"));
    result.erase(result.find_last_not_of(" \t\n\r") + 1);

    return result;
}

/**
 * 从环境变量获取值
 */
std::optional<std::string> GitIntegration::getEnvVar(const std::string& var_name) {
    const char* value = std::getenv(var_name.c_str());
    if (value != nullptr) {
        return std::string(value);
    }
    return std::nullopt;
}

/**
 * 检查当前目录是否为 Git 仓库
 */
bool GitIntegration::isGitRepository() {
    std::string result = executeGitCommand("git rev-parse --git-dir 2>/dev/null");
    return !result.empty();
}

/**
 * 获取当前分支名称
 */
std::string GitIntegration::getCurrentBranch() {
    return executeGitCommand("git rev-parse --abbrev-ref HEAD");
}

/**
 * 获取默认主分支名称
 */
std::string GitIntegration::getDefaultBranch() {
    // 尝试 main
    std::string result = executeGitCommand("git rev-parse --verify main 2>/dev/null");
    if (!result.empty()) {
        return "main";
    }

    // 尝试 master
    result = executeGitCommand("git rev-parse --verify master 2>/dev/null");
    if (!result.empty()) {
        return "master";
    }

    // 尝试从远程获取默认分支
    result = executeGitCommand("git symbolic-ref refs/remotes/origin/HEAD 2>/dev/null");
    if (!result.empty()) {
        // 提取分支名 (refs/remotes/origin/main -> main)
        size_t pos = result.find_last_of('/');
        if (pos != std::string::npos) {
            return result.substr(pos + 1);
        }
    }

    return "main";  // 默认返回 main
}

/**
 * 检查文件是否为 C++ 源文件
 */
bool GitIntegration::isCppFile(const std::string& path) {
    static const std::vector<std::string> extensions = {
        ".cpp", ".cc", ".cxx", ".c++",
        ".h", ".hpp", ".hxx", ".h++"
    };

    fs::path p(path);
    std::string ext = p.extension().string();

    return std::find(extensions.begin(), extensions.end(), ext) != extensions.end();
}

/**
 * 过滤出 C++ 源文件
 */
std::vector<std::string> GitIntegration::filterCppFiles(const std::vector<std::string>& files) {
    std::vector<std::string> cpp_files;

    for (const auto& file : files) {
        if (isCppFile(file) && fs::exists(file)) {
            cpp_files.push_back(file);
        }
    }

    return cpp_files;
}

/**
 * 获取变更的文件列表
 */
std::vector<std::string> GitIntegration::getChangedFiles(
    IncrementalMode mode,
    const std::string& reference
) {
    std::string git_command;

    switch (mode) {
        case IncrementalMode::WORKSPACE:
            // 工作区未提交的更改
            git_command = "git diff --name-only";
            break;

        case IncrementalMode::STAGED:
            // 暂存区的更改
            git_command = "git diff --cached --name-only";
            break;

        case IncrementalMode::BRANCH: {
            // 与指定分支的差异
            std::string base_branch = reference.empty() ? getDefaultBranch() : reference;
            git_command = "git diff --name-only " + base_branch + "...HEAD";
            break;
        }

        case IncrementalMode::COMMIT:
            // 从指定提交以来的更改
            git_command = "git diff --name-only " + reference + "..HEAD";
            break;

        case IncrementalMode::PR: {
            // PR 模式: 自动检测 base 分支
            auto pr_env = detectPREnvironment();
            if (pr_env) {
                git_command = "git diff --name-only " + pr_env->base_branch + "...HEAD";
            } else {
                // 如果不在 PR 环境,使用默认分支
                std::string base_branch = getDefaultBranch();
                git_command = "git diff --name-only " + base_branch + "...HEAD";
            }
            break;
        }
    }

    // 执行 Git 命令
    std::string output = executeGitCommand(git_command);

    // 解析输出为文件列表
    std::vector<std::string> files;
    std::istringstream iss(output);
    std::string file;

    while (std::getline(iss, file)) {
        if (!file.empty()) {
            files.push_back(file);
        }
    }

    // 过滤出 C++ 文件
    return filterCppFiles(files);
}

/**
 * 检测 PR 环境
 */
std::optional<PREnvironment> GitIntegration::detectPREnvironment() {
    PREnvironment env;
    env.is_pr_environment = false;

    // ===== GitHub Actions 检测 =====
    auto github_actions = getEnvVar("GITHUB_ACTIONS");
    auto github_event = getEnvVar("GITHUB_EVENT_NAME");

    if (github_actions && *github_actions == "true" &&
        github_event && *github_event == "pull_request") {

        env.provider = "github";
        env.is_pr_environment = true;

        // 获取 PR 信息
        if (auto base_ref = getEnvVar("GITHUB_BASE_REF")) {
            env.base_branch = *base_ref;
        }
        if (auto head_ref = getEnvVar("GITHUB_HEAD_REF")) {
            env.head_branch = *head_ref;
        }
        if (auto pr_number = getEnvVar("GITHUB_PR_NUMBER")) {
            env.pr_number = *pr_number;
        }
        if (auto repository = getEnvVar("GITHUB_REPOSITORY")) {
            // 解析 owner/repo
            size_t pos = repository->find('/');
            if (pos != std::string::npos) {
                env.repo_owner = repository->substr(0, pos);
                env.repo_name = repository->substr(pos + 1);
            }
        }

        return env;
    }

    // ===== GitLab CI 检测 =====
    auto gitlab_ci = getEnvVar("GITLAB_CI");
    auto merge_request_iid = getEnvVar("CI_MERGE_REQUEST_IID");

    if (gitlab_ci && merge_request_iid) {
        env.provider = "gitlab";
        env.is_pr_environment = true;

        // 获取 MR 信息
        if (auto target_branch = getEnvVar("CI_MERGE_REQUEST_TARGET_BRANCH_NAME")) {
            env.base_branch = *target_branch;
        }
        if (auto source_branch = getEnvVar("CI_MERGE_REQUEST_SOURCE_BRANCH_NAME")) {
            env.head_branch = *source_branch;
        }
        env.pr_number = *merge_request_iid;

        if (auto project_path = getEnvVar("CI_PROJECT_PATH")) {
            // 解析 owner/repo
            size_t pos = project_path->find('/');
            if (pos != std::string::npos) {
                env.repo_owner = project_path->substr(0, pos);
                env.repo_name = project_path->substr(pos + 1);
            }
        }

        return env;
    }

    // 不在 PR 环境中
    return std::nullopt;
}

/**
 * 生成 PR 格式的审查报告
 */
std::string GitIntegration::generatePRComment(
    const std::string& report_content,
    const PREnvironment& pr_env
) {
    std::ostringstream comment;

    // PR 评论头部
    comment << "## 🤖 C++ Code Review Agent - 自动审查报告\n\n";
    comment << "**分析范围**: `" << pr_env.base_branch << "` → `" << pr_env.head_branch << "`\n";
    comment << "**PR编号**: #" << pr_env.pr_number << "\n\n";
    comment << "---\n\n";

    // 添加报告内容
    comment << report_content;

    // PR 评论尾部
    comment << "\n\n---\n";
    comment << "*本报告由 C++ Code Review Agent V2.0 自动生成*  \n";
    comment << "*基于 Clang/LLVM AST 静态分析 · 零误报 · 精准定位*\n";

    return comment.str();
}

} // namespace cpp_review
