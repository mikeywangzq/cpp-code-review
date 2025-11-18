/*
 * 自动代码修复系统实现 (Auto Code Fixer Implementation)
 *
 * 功能说明 (Features):
 * - 自动生成和应用代码修复 (Automatic code fix generation and application)
 * - 支持8种规则的智能修复 (Support for 8 rule types)
 * - 安全的文件操作和备份机制 (Safe file operations with backup)
 * - 交互式和自动化两种模式 (Interactive and automated modes)
 *
 * 潜在问题修复 (Bug Fixes):
 * - 添加完整的边界检查 (Complete boundary checks)
 * - 改进错误处理和资源管理 (Better error handling)
 * - 修复文件末尾换行符问题 (Fix trailing newline issue)
 * - 添加文件存在性检查 (Add file existence checks)
 */

#include "fixer/auto_fixer.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <regex>

namespace cpp_review {

/**
 * 构造函数 (Constructor)
 * @param create_backup 是否创建备份文件 (Whether to create backup files)
 */
AutoFixer::AutoFixer(bool create_backup)
    : create_backup_(create_backup), interactive_mode_(false) {}

/**
 * 生成修复操作 (Generate fix action for an issue)
 * @param issue 待修复的问题 (Issue to fix)
 * @return 修复操作，如果不支持则返回 nullopt (Fix action or nullopt if unsupported)
 */
std::optional<FixAction> AutoFixer::generateFix(const Issue& issue) {
    // 根据规则 ID 分发到不同的修复生成器
    // Dispatch to different fix generators based on rule ID
    if (issue.rule_id == "NULL-PTR-001") {
        return fixNullPointer(issue);
    } else if (issue.rule_id == "UNINIT-VAR-001") {
        return fixUninitializedVar(issue);
    } else if (issue.rule_id == "ASSIGN-COND-001") {
        return fixAssignmentInCondition(issue);
    } else if (issue.rule_id == "UNSAFE-C-FUNC-001") {
        return fixUnsafeCFunction(issue);
    } else if (issue.rule_id == "MEMORY-LEAK-001") {
        return fixMemoryLeak(issue);
    } else if (issue.rule_id == "BUFFER-OVERFLOW-001") {
        return fixBufferOverflow(issue);
    } else if (issue.rule_id == "INTEGER-OVERFLOW-001") {
        return fixIntegerOverflow(issue);
    } else if (issue.rule_id == "USE-AFTER-FREE-001") {
        return fixUseAfterFree(issue);
    }

    // 不支持的规则 (Unsupported rule)
    return std::nullopt;
}

/**
 * 应用修复操作 (Apply fix action to file)
 * @param fix 要应用的修复操作 (Fix action to apply)
 * @return 成功返回true (Returns true on success)
 *
 * Bug修复 (Bug Fixes):
 * - 添加文件存在性检查 (Add file existence check)
 * - 改进边界检查 (Improve boundary checks)
 * - 修复换行符处理 (Fix newline handling)
 */
bool AutoFixer::applyFix(const FixAction& fix) {
    // 交互式模式：询问用户确认
    // Interactive mode: ask for user confirmation
    if (interactive_mode_ && !askUserConfirmation(fix)) {
        std::cout << "⏭️  跳过修复\n";
        return false;
    }

    // BUG FIX: 检查文件是否存在
    // Check if file exists before proceeding
    if (!std::filesystem::exists(fix.file_path)) {
        std::cerr << "❌ 文件不存在: " << fix.file_path << "\n";
        return false;
    }

    // 创建备份
    // Create backup if enabled
    if (create_backup_ && !createBackup(fix.file_path)) {
        std::cerr << "❌ 无法创建备份文件: " << fix.file_path << "\n";
        return false;
    }

    // 读取文件
    // Read file content
    auto lines = readLines(fix.file_path);
    if (lines.empty()) {
        // BUG FIX: 区分空文件和读取失败
        // Distinguish between empty file and read failure
        std::ifstream test(fix.file_path);
        if (!test.good()) {
            std::cerr << "❌ 无法读取文件: " << fix.file_path << "\n";
            return false;
        }
        // 空文件是合法的，继续处理
        // Empty file is valid, continue
    }

    // 应用修复
    // Apply the fix
    try {
        switch (fix.type) {
            case FixType::REPLACE: {
                // 替换指定行的内容
                // Replace content at specified line

                // BUG FIX: 添加更严格的边界检查
                // Add stricter boundary checks
                if (fix.line_start == 0 || fix.line_start > lines.size()) {
                    std::cerr << "❌ 行号越界: " << fix.line_start << " (文件共 "
                              << lines.size() << " 行)\n";
                    return false;
                }

                std::string& line = lines[fix.line_start - 1];

                // 如果指定了列范围，只替换部分内容
                // If column range specified, replace partial content
                if (fix.column_start > 0 && fix.column_end > 0) {
                    // BUG FIX: 检查列号有效性
                    // Check column validity
                    if (fix.column_start > line.length() + 1) {
                        std::cerr << "❌ 起始列号越界: " << fix.column_start
                                  << " (行长度: " << line.length() << ")\n";
                        return false;
                    }

                    // BUG FIX: 确保 column_end >= column_start
                    // Ensure column_end >= column_start
                    if (fix.column_end < fix.column_start) {
                        std::cerr << "❌ 列号范围无效: " << fix.column_start
                                  << " > " << fix.column_end << "\n";
                        return false;
                    }

                    size_t start_pos = fix.column_start - 1;
                    size_t length = fix.column_end - fix.column_start;

                    // BUG FIX: 确保不会替换超出行长度的内容
                    // Ensure we don't replace beyond line length
                    if (start_pos + length > line.length()) {
                        length = line.length() - start_pos;
                    }

                    line.replace(start_pos, length, fix.new_code);
                } else {
                    // 替换整行
                    // Replace entire line
                    line = fix.new_code;
                }
                break;
            }

            case FixType::INSERT: {
                // 在指定行插入新内容
                // Insert new content at specified line

                // BUG FIX: 允许在文件末尾插入（line_start = lines.size() + 1）
                // Allow insertion at end of file
                if (fix.line_start == 0 || fix.line_start > lines.size() + 1) {
                    std::cerr << "❌ 插入位置无效: " << fix.line_start << "\n";
                    return false;
                }

                lines.insert(lines.begin() + fix.line_start - 1, fix.new_code);
                break;
            }

            case FixType::DELETE: {
                // 删除指定范围的行
                // Delete lines in specified range

                // BUG FIX: 添加范围有效性检查
                // Add range validity checks
                if (fix.line_start == 0 || fix.line_end == 0 ||
                    fix.line_start > lines.size() || fix.line_end > lines.size()) {
                    std::cerr << "❌ 删除范围越界: [" << fix.line_start
                              << ", " << fix.line_end << "]\n";
                    return false;
                }

                // BUG FIX: 确保 line_end >= line_start
                // Ensure line_end >= line_start
                if (fix.line_end < fix.line_start) {
                    std::cerr << "❌ 删除范围无效: " << fix.line_start
                              << " > " << fix.line_end << "\n";
                    return false;
                }

                lines.erase(lines.begin() + fix.line_start - 1,
                           lines.begin() + fix.line_end);
                break;
            }

            case FixType::ADD_INCLUDE: {
                // 在文件开头添加 include
                // Add include at file beginning

                // 检查是否已存在该 include
                // Check if include already exists
                bool has_include = false;
                for (const auto& line : lines) {
                    if (line.find(fix.new_code) != std::string::npos) {
                        has_include = true;
                        break;
                    }
                }

                if (!has_include) {
                    // 找到第一个非注释、非空行
                    // Find first non-comment, non-empty line
                    size_t insert_pos = 0;
                    for (size_t i = 0; i < lines.size(); ++i) {
                        std::string trimmed = lines[i];

                        // BUG FIX: 检查 find_first_not_of 返回值
                        // Check find_first_not_of return value
                        size_t first_non_space = trimmed.find_first_not_of(" \t");
                        if (first_non_space != std::string::npos) {
                            trimmed = trimmed.substr(first_non_space);

                            // 跳过注释行
                            // Skip comment lines
                            if (!trimmed.empty() &&
                                trimmed[0] != '/' &&
                                trimmed[0] != '*' &&
                                trimmed[0] != '#') {  // BUG FIX: 也跳过预处理指令之外的#
                                insert_pos = i;
                                break;
                            }
                        }
                    }

                    lines.insert(lines.begin() + insert_pos, fix.new_code);
                }
                break;
            }

            case FixType::REWRITE: {
                // 完全重写文件内容
                // Completely rewrite file content
                lines.clear();
                std::istringstream iss(fix.new_code);
                std::string line;
                while (std::getline(iss, line)) {
                    lines.push_back(line);
                }
                break;
            }
        }

        // 写回文件
        // Write back to file
        std::ostringstream oss;
        for (size_t i = 0; i < lines.size(); ++i) {
            oss << lines[i];
            // BUG FIX: 所有行都添加换行符，包括最后一行
            // Add newline to all lines including the last one
            // 这样可以保持文件格式的一致性
            // This maintains file format consistency
            if (i < lines.size() - 1 || !lines.empty()) {
                oss << "\n";
            }
        }

        bool write_success = writeFile(fix.file_path, oss.str());

        if (!write_success) {
            std::cerr << "❌ 写入文件失败: " << fix.file_path << "\n";

            // BUG FIX: 写入失败时尝试恢复备份
            // Try to restore backup on write failure
            if (create_backup_) {
                std::string backup_path = fix.file_path + ".backup";
                if (std::filesystem::exists(backup_path)) {
                    std::cerr << "🔄 尝试从备份恢复...\n";
                    try {
                        std::filesystem::copy_file(
                            backup_path,
                            fix.file_path,
                            std::filesystem::copy_options::overwrite_existing
                        );
                        std::cerr << "✅ 已从备份恢复\n";
                    } catch (const std::exception& e) {
                        std::cerr << "❌ 恢复备份失败: " << e.what() << "\n";
                    }
                }
            }
            return false;
        }

        return true;

    } catch (const std::exception& e) {
        std::cerr << "❌ 应用修复时出错: " << e.what() << "\n";
        return false;
    } catch (...) {
        // BUG FIX: 捕获所有异常
        // Catch all exceptions
        std::cerr << "❌ 应用修复时发生未知错误\n";
        return false;
    }
}

/**
 * 批量修复所有问题 (Fix all issues in batch)
 * @param reporter 包含问题列表的报告器 (Reporter containing issues)
 * @param filter_severity 只修复此严重性级别及以上的问题 (Only fix issues at or above this severity)
 * @return 修复结果统计 (Fix result statistics)
 */
FixResult AutoFixer::fixAll(const Reporter& reporter, Severity filter_severity) {
    FixResult result{true, "", {}, 0, 0};

    std::cout << "\n🔧 开始自动修复...\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

    const auto& issues = reporter.getIssues();

    // BUG FIX: 添加空问题列表检查
    // Check for empty issue list
    if (issues.empty()) {
        std::cout << "✅ 没有需要修复的问题\n";
        result.message = "无问题需要修复";
        return result;
    }

    for (const auto& issue : issues) {
        // 过滤严重性
        // Filter by severity
        if (issue.severity > filter_severity) {
            continue;
        }

        std::cout << "🔍 处理问题: " << issue.rule_id << " at "
                  << issue.file_path << ":" << issue.line << "\n";

        // 生成修复
        // Generate fix
        auto fix_opt = generateFix(issue);
        if (!fix_opt) {
            std::cout << "⚠️  该问题暂不支持自动修复\n\n";
            continue;
        }

        // 应用修复
        // Apply fix
        if (applyFix(*fix_opt)) {
            std::cout << "✅ 修复成功: " << fix_opt->description << "\n\n";
            result.fixed_count++;

            // 记录修改的文件（避免重复）
            // Record modified files (avoid duplicates)
            if (std::find(result.modified_files.begin(),
                         result.modified_files.end(),
                         fix_opt->file_path) == result.modified_files.end()) {
                result.modified_files.push_back(fix_opt->file_path);
            }
        } else {
            std::cout << "❌ 修复失败\n\n";
            result.failed_count++;
        }
    }

    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "📊 修复统计:\n";
    std::cout << "  ✅ 成功: " << result.fixed_count << "\n";
    std::cout << "  ❌ 失败: " << result.failed_count << "\n";
    std::cout << "  📁 修改文件数: " << result.modified_files.size() << "\n";

    result.message = "修复完成";
    result.success = (result.failed_count == 0);

    return result;
}

// ============================================================================
// 规则特定的修复生成器 (Rule-specific fix generators)
// ============================================================================

/**
 * 修复空指针解引用 (Fix null pointer dereference)
 * @param issue 问题信息 (Issue information)
 * @return 修复操作 (Fix action)
 */
std::optional<FixAction> AutoFixer::fixNullPointer(const Issue& issue) {
    FixAction fix;
    fix.type = FixType::REPLACE;
    fix.file_path = issue.file_path;
    fix.line_start = issue.line;
    fix.line_end = issue.line;
    fix.column_start = 0;
    fix.column_end = 0;
    fix.description = "添加空指针检查";

    // 从问题描述中提取变量名
    // Extract variable name from issue description
    std::regex var_regex("'([^']+)'");
    std::smatch match;
    std::string var_name = "ptr";  // 默认变量名 (default variable name)

    if (std::regex_search(issue.description, match, var_regex) && match.size() > 1) {
        var_name = match[1].str();
    }

    // 生成修复代码：添加空指针检查
    // Generate fix code: add null pointer check
    fix.new_code = "    if (" + var_name + " != nullptr) {\n"
                   "        // 原始代码 (original code)\n"
                   "    } else {\n"
                   "        // 处理空指针情况 (handle null pointer case)\n"
                   "    }";

    return fix;
}

/**
 * 修复未初始化变量 (Fix uninitialized variable)
 * @param issue 问题信息 (Issue information)
 * @return 修复操作 (Fix action)
 */
std::optional<FixAction> AutoFixer::fixUninitializedVar(const Issue& issue) {
    FixAction fix;
    fix.type = FixType::REPLACE;
    fix.file_path = issue.file_path;
    fix.line_start = issue.line;
    fix.line_end = issue.line;
    fix.description = "初始化变量";

    // 从问题描述中提取变量信息
    // Extract variable information from issue description
    std::regex var_regex("'([^']+)'.*'([^']+)'");
    std::smatch match;

    if (std::regex_search(issue.description, match, var_regex) && match.size() > 2) {
        std::string var_name = match[1].str();
        std::string var_type = match[2].str();

        // 根据类型生成合适的初始化代码
        // Generate appropriate initialization code based on type
        if (var_type.find("int") != std::string::npos ||
            var_type.find("long") != std::string::npos ||
            var_type.find("short") != std::string::npos) {
            fix.new_code = "    " + var_type + " " + var_name + " = 0;";
        } else if (var_type.find("double") != std::string::npos ||
                   var_type.find("float") != std::string::npos) {
            fix.new_code = "    " + var_type + " " + var_name + " = 0.0;";
        } else if (var_type.find("*") != std::string::npos) {
            fix.new_code = "    " + var_type + " " + var_name + " = nullptr;";
        } else if (var_type.find("bool") != std::string::npos) {
            // BUG FIX: 添加bool类型支持
            // Add bool type support
            fix.new_code = "    " + var_type + " " + var_name + " = false;";
        } else {
            // 使用统一初始化 (use uniform initialization)
            fix.new_code = "    " + var_type + " " + var_name + "{};";
        }

        return fix;
    }

    // BUG FIX: 无法提取变量信息时返回nullopt
    // Return nullopt if unable to extract variable info
    return std::nullopt;
}

/**
 * 修复赋值/比较混淆 (Fix assignment in condition)
 * @param issue 问题信息 (Issue information)
 * @return 修复操作 (Fix action)
 */
std::optional<FixAction> AutoFixer::fixAssignmentInCondition(const Issue& issue) {
    FixAction fix;
    fix.type = FixType::REPLACE;
    fix.file_path = issue.file_path;
    fix.line_start = issue.line;
    fix.line_end = issue.line;
    fix.description = "修复赋值/比较混淆";

    // 读取该行代码
    // Read the line
    auto lines = readLines(issue.file_path);
    if (issue.line > 0 && issue.line <= lines.size()) {
        std::string line = lines[issue.line - 1];

        // 将 = 替换为 ==
        // Replace = with ==
        std::regex assign_regex(R"(if\s*\(([^=]+)=([^=]))");
        std::string new_line = std::regex_replace(line, assign_regex, "if ($1==$2");

        // BUG FIX: 检查替换是否成功
        // Check if replacement succeeded
        if (new_line == line) {
            // 可能是 while 语句
            // Might be a while statement
            assign_regex = std::regex(R"(while\s*\(([^=]+)=([^=]))");
            new_line = std::regex_replace(line, assign_regex, "while ($1==$2");
        }

        if (new_line != line) {
            fix.new_code = new_line;
            return fix;
        }
    }

    return std::nullopt;
}

/**
 * 修复不安全的C函数 (Fix unsafe C function)
 * @param issue 问题信息 (Issue information)
 * @return 修复操作 (Fix action)
 */
std::optional<FixAction> AutoFixer::fixUnsafeCFunction(const Issue& issue) {
    FixAction fix;
    fix.type = FixType::REPLACE;
    fix.file_path = issue.file_path;
    fix.line_start = issue.line;
    fix.line_end = issue.line;
    fix.description = "替换为安全函数";

    // 读取该行代码
    // Read the line
    auto lines = readLines(issue.file_path);
    if (issue.line > 0 && issue.line <= lines.size()) {
        std::string line = lines[issue.line - 1];

        // 替换不安全的 C 函数
        // Replace unsafe C functions
        if (line.find("strcpy") != std::string::npos) {
            fix.new_code = std::regex_replace(line, std::regex(R"(\bstrcpy\b)"), "strncpy");
            return fix;
        } else if (line.find("sprintf") != std::string::npos) {
            fix.new_code = std::regex_replace(line, std::regex(R"(\bsprintf\b)"), "snprintf");
            return fix;
        } else if (line.find("strcat") != std::string::npos) {
            // BUG FIX: 添加strcat支持
            // Add strcat support
            fix.new_code = std::regex_replace(line, std::regex(R"(\bstrcat\b)"), "strncat");
            return fix;
        } else if (line.find("gets") != std::string::npos) {
            fix.new_code = "    // BUG: gets() 已废弃，请使用 std::getline 或 fgets\n"
                          "    // std::string line;\n"
                          "    // std::getline(std::cin, line);";
            return fix;
        }
    }

    return std::nullopt;
}

/**
 * 修复内存泄漏 (Fix memory leak)
 * @param issue 问题信息 (Issue information)
 * @return 修复操作 (Fix action)
 */
std::optional<FixAction> AutoFixer::fixMemoryLeak(const Issue& issue) {
    FixAction fix;
    fix.type = FixType::ADD_INCLUDE;
    fix.file_path = issue.file_path;
    fix.line_start = 1;
    fix.description = "添加智能指针头文件";

    // 添加 memory 头文件
    // Add memory header
    fix.new_code = "#include <memory>";

    return fix;
}

/**
 * 修复缓冲区溢出 (Fix buffer overflow)
 * 注意：缓冲区溢出修复需要更复杂的分析，当前仅返回建议
 * Note: Buffer overflow fixing requires complex analysis, currently only returns suggestion
 */
std::optional<FixAction> AutoFixer::fixBufferOverflow(const Issue& issue) {
    // 缓冲区溢出通常需要更复杂的分析
    // Buffer overflow usually requires more complex analysis
    // 这里暂不提供自动修复
    // No automatic fix provided here
    return std::nullopt;
}

/**
 * 修复整数溢出 (Fix integer overflow)
 */
std::optional<FixAction> AutoFixer::fixIntegerOverflow(const Issue& issue) {
    // 整数溢出修复需要上下文分析
    // Integer overflow fixing requires context analysis
    return std::nullopt;
}

/**
 * 修复Use-After-Free (Fix use-after-free)
 */
std::optional<FixAction> AutoFixer::fixUseAfterFree(const Issue& issue) {
    // Use-After-Free 修复需要数据流分析
    // Use-after-free fixing requires data flow analysis
    return std::nullopt;
}

// ============================================================================
// 辅助方法 (Helper methods)
// ============================================================================

/**
 * 创建文件备份 (Create file backup)
 * @param file_path 文件路径 (File path)
 * @return 成功返回true (Returns true on success)
 */
bool AutoFixer::createBackup(const std::string& file_path) {
    try {
        std::string backup_path = file_path + ".backup";

        // BUG FIX: 如果备份已存在，先删除旧备份
        // If backup exists, remove old backup first
        if (std::filesystem::exists(backup_path)) {
            std::filesystem::remove(backup_path);
        }

        std::filesystem::copy_file(file_path, backup_path,
                                   std::filesystem::copy_options::overwrite_existing);
        backup_files_.push_back(backup_path);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "备份失败: " << e.what() << "\n";
        return false;
    }
}

/**
 * 写入文件 (Write to file)
 * @param file_path 文件路径 (File path)
 * @param content 文件内容 (File content)
 * @return 成功返回true (Returns true on success)
 */
bool AutoFixer::writeFile(const std::string& file_path, const std::string& content) {
    try {
        // BUG FIX: 使用二进制模式防止换行符转换问题
        // Use binary mode to prevent newline conversion issues
        std::ofstream out(file_path, std::ios::binary);
        if (!out) {
            return false;
        }
        out << content;
        out.close();

        // BUG FIX: 检查写入是否成功
        // Check if write succeeded
        if (out.fail()) {
            return false;
        }

        return true;
    } catch (const std::exception&) {
        return false;
    }
}

/**
 * 读取文件内容 (Read file content)
 * @param file_path 文件路径 (File path)
 * @return 文件内容字符串 (File content string)
 */
std::string AutoFixer::readFile(const std::string& file_path) {
    // BUG FIX: 使用二进制模式保持原始内容
    // Use binary mode to preserve original content
    std::ifstream in(file_path, std::ios::binary);
    if (!in) {
        return "";
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

/**
 * 按行读取文件 (Read file lines)
 * @param file_path 文件路径 (File path)
 * @return 行的vector (Vector of lines)
 */
std::vector<std::string> AutoFixer::readLines(const std::string& file_path) {
    std::vector<std::string> lines;
    std::ifstream in(file_path);
    if (!in) {
        return lines;
    }

    std::string line;
    while (std::getline(in, line)) {
        lines.push_back(line);
    }

    return lines;
}

/**
 * 询问用户确认 (Ask user for confirmation)
 * @param fix 待确认的修复操作 (Fix action to confirm)
 * @return 用户同意返回true (Returns true if user agrees)
 */
bool AutoFixer::askUserConfirmation(const FixAction& fix) {
    std::cout << "\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "🔧 建议的修复:\n";
    std::cout << "  📁 文件: " << fix.file_path << ":" << fix.line_start << "\n";
    std::cout << "  📝 描述: " << fix.description << "\n";
    std::cout << "  ⚙️  类型: ";

    switch (fix.type) {
        case FixType::REPLACE: std::cout << "替换"; break;
        case FixType::INSERT: std::cout << "插入"; break;
        case FixType::DELETE: std::cout << "删除"; break;
        case FixType::ADD_INCLUDE: std::cout << "添加头文件"; break;
        case FixType::REWRITE: std::cout << "重写"; break;
    }

    std::cout << "\n\n新代码:\n" << fix.new_code << "\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "应用此修复? [y/N]: ";

    std::string response;
    std::getline(std::cin, response);

    // BUG FIX: 改进输入验证
    // Improve input validation
    // 去除首尾空格 (trim whitespace)
    response.erase(0, response.find_first_not_of(" \t\n\r"));
    response.erase(response.find_last_not_of(" \t\n\r") + 1);

    return (response == "y" || response == "Y" ||
            response == "yes" || response == "Yes" ||
            response == "YES");
}

/**
 * 回滚所有修复 (Rollback all fixes)
 * @return 成功返回true (Returns true on success)
 */
bool AutoFixer::rollback() {
    bool success = true;

    std::cout << "\n🔄 回滚修复...\n";

    // BUG FIX: 检查是否有备份文件
    // Check if there are backup files
    if (backup_files_.empty()) {
        std::cout << "⚠️  没有备份文件需要回滚\n";
        return true;
    }

    for (const auto& backup_path : backup_files_) {
        try {
            // BUG FIX: 检查备份文件是否存在
            // Check if backup file exists
            if (!std::filesystem::exists(backup_path)) {
                std::cerr << "❌ 备份文件不存在: " << backup_path << "\n";
                success = false;
                continue;
            }

            std::string original_path = backup_path.substr(0, backup_path.length() - 7);  // 移除 .backup
            std::filesystem::copy_file(backup_path, original_path,
                                      std::filesystem::copy_options::overwrite_existing);
            std::filesystem::remove(backup_path);
            std::cout << "✅ 已恢复: " << original_path << "\n";
        } catch (const std::exception& e) {
            std::cerr << "❌ 回滚失败: " << backup_path << " - " << e.what() << "\n";
            success = false;
        }
    }

    backup_files_.clear();
    return success;
}

} // namespace cpp_review
