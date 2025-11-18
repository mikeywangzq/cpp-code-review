/*
 * 自动代码修复系统实现
 */

#include "fixer/auto_fixer.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <regex>

namespace cpp_review {

AutoFixer::AutoFixer(bool create_backup)
    : create_backup_(create_backup), interactive_mode_(false) {}

std::optional<FixAction> AutoFixer::generateFix(const Issue& issue) {
    // 根据规则 ID 分发到不同的修复生成器
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

    // 不支持的规则
    return std::nullopt;
}

bool AutoFixer::applyFix(const FixAction& fix) {
    // 交互式模式：询问用户确认
    if (interactive_mode_ && !askUserConfirmation(fix)) {
        std::cout << "⏭️  跳过修复\n";
        return false;
    }

    // 创建备份
    if (create_backup_ && !createBackup(fix.file_path)) {
        std::cerr << "❌ 无法创建备份文件: " << fix.file_path << "\n";
        return false;
    }

    // 读取文件
    auto lines = readLines(fix.file_path);
    if (lines.empty()) {
        std::cerr << "❌ 无法读取文件: " << fix.file_path << "\n";
        return false;
    }

    // 应用修复
    try {
        switch (fix.type) {
            case FixType::REPLACE: {
                // 替换指定行的内容
                if (fix.line_start > 0 && fix.line_start <= lines.size()) {
                    std::string& line = lines[fix.line_start - 1];

                    // 如果指定了列范围，只替换部分内容
                    if (fix.column_start > 0 && fix.column_end > 0) {
                        line.replace(fix.column_start - 1,
                                   fix.column_end - fix.column_start,
                                   fix.new_code);
                    } else {
                        // 替换整行
                        line = fix.new_code;
                    }
                }
                break;
            }
            case FixType::INSERT: {
                // 在指定行插入新内容
                if (fix.line_start > 0 && fix.line_start <= lines.size() + 1) {
                    lines.insert(lines.begin() + fix.line_start - 1, fix.new_code);
                }
                break;
            }
            case FixType::DELETE: {
                // 删除指定范围的行
                if (fix.line_start > 0 && fix.line_end <= lines.size()) {
                    lines.erase(lines.begin() + fix.line_start - 1,
                               lines.begin() + fix.line_end);
                }
                break;
            }
            case FixType::ADD_INCLUDE: {
                // 在文件开头添加 include
                bool has_include = false;
                for (const auto& line : lines) {
                    if (line.find(fix.new_code) != std::string::npos) {
                        has_include = true;
                        break;
                    }
                }
                if (!has_include) {
                    // 找到第一个非注释行
                    size_t insert_pos = 0;
                    for (size_t i = 0; i < lines.size(); ++i) {
                        std::string trimmed = lines[i];
                        trimmed.erase(0, trimmed.find_first_not_of(" \t"));
                        if (!trimmed.empty() && trimmed[0] != '/' && trimmed[0] != '*') {
                            insert_pos = i;
                            break;
                        }
                    }
                    lines.insert(lines.begin() + insert_pos, fix.new_code);
                }
                break;
            }
            case FixType::REWRITE: {
                // 完全重写文件内容
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
        std::ostringstream oss;
        for (size_t i = 0; i < lines.size(); ++i) {
            oss << lines[i];
            if (i < lines.size() - 1) {
                oss << "\n";
            }
        }

        return writeFile(fix.file_path, oss.str());

    } catch (const std::exception& e) {
        std::cerr << "❌ 应用修复时出错: " << e.what() << "\n";
        return false;
    }
}

FixResult AutoFixer::fixAll(const Reporter& reporter, Severity filter_severity) {
    FixResult result{true, "", {}, 0, 0};

    std::cout << "\n🔧 开始自动修复...\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";

    const auto& issues = reporter.getIssues();

    for (const auto& issue : issues) {
        // 过滤严重性
        if (issue.severity > filter_severity) {
            continue;
        }

        std::cout << "🔍 处理问题: " << issue.rule_id << " at "
                  << issue.file_path << ":" << issue.line << "\n";

        // 生成修复
        auto fix_opt = generateFix(issue);
        if (!fix_opt) {
            std::cout << "⚠️  该问题暂不支持自动修复\n\n";
            continue;
        }

        // 应用修复
        if (applyFix(*fix_opt)) {
            std::cout << "✅ 修复成功: " << fix_opt->description << "\n\n";
            result.fixed_count++;

            // 记录修改的文件
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
// 规则特定的修复生成器
// ============================================================================

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
    std::regex var_regex("'([^']+)'");
    std::smatch match;
    std::string var_name = "ptr";

    if (std::regex_search(issue.description, match, var_regex) && match.size() > 1) {
        var_name = match[1].str();
    }

    // 生成修复代码：添加空指针检查
    fix.new_code = "    if (" + var_name + " != nullptr) {\n"
                   "        // 原始代码\n"
                   "    } else {\n"
                   "        // 处理空指针情况\n"
                   "    }";

    return fix;
}

std::optional<FixAction> AutoFixer::fixUninitializedVar(const Issue& issue) {
    FixAction fix;
    fix.type = FixType::REPLACE;
    fix.file_path = issue.file_path;
    fix.line_start = issue.line;
    fix.line_end = issue.line;
    fix.description = "初始化变量";

    // 从问题描述中提取变量信息
    std::regex var_regex("'([^']+)'.*'([^']+)'");
    std::smatch match;

    if (std::regex_search(issue.description, match, var_regex) && match.size() > 2) {
        std::string var_name = match[1].str();
        std::string var_type = match[2].str();

        // 生成初始化代码
        if (var_type.find("int") != std::string::npos ||
            var_type.find("long") != std::string::npos ||
            var_type.find("short") != std::string::npos) {
            fix.new_code = "    " + var_type + " " + var_name + " = 0;";
        } else if (var_type.find("double") != std::string::npos ||
                   var_type.find("float") != std::string::npos) {
            fix.new_code = "    " + var_type + " " + var_name + " = 0.0;";
        } else if (var_type.find("*") != std::string::npos) {
            fix.new_code = "    " + var_type + " " + var_name + " = nullptr;";
        } else {
            fix.new_code = "    " + var_type + " " + var_name + "{};";
        }

        return fix;
    }

    return std::nullopt;
}

std::optional<FixAction> AutoFixer::fixAssignmentInCondition(const Issue& issue) {
    FixAction fix;
    fix.type = FixType::REPLACE;
    fix.file_path = issue.file_path;
    fix.line_start = issue.line;
    fix.line_end = issue.line;
    fix.description = "修复赋值/比较混淆";

    // 读取该行代码
    auto lines = readLines(issue.file_path);
    if (issue.line > 0 && issue.line <= lines.size()) {
        std::string line = lines[issue.line - 1];

        // 将 = 替换为 ==
        std::regex assign_regex(R"(if\s*\(([^=]+)=([^=]))");
        fix.new_code = std::regex_replace(line, assign_regex, "if ($1==$2");

        return fix;
    }

    return std::nullopt;
}

std::optional<FixAction> AutoFixer::fixUnsafeCFunction(const Issue& issue) {
    FixAction fix;
    fix.type = FixType::REPLACE;
    fix.file_path = issue.file_path;
    fix.line_start = issue.line;
    fix.line_end = issue.line;
    fix.description = "替换为安全函数";

    // 读取该行代码
    auto lines = readLines(issue.file_path);
    if (issue.line > 0 && issue.line <= lines.size()) {
        std::string line = lines[issue.line - 1];

        // 替换不安全的 C 函数
        if (line.find("strcpy") != std::string::npos) {
            fix.new_code = std::regex_replace(line, std::regex("strcpy"), "strncpy");
        } else if (line.find("sprintf") != std::string::npos) {
            fix.new_code = std::regex_replace(line, std::regex("sprintf"), "snprintf");
        } else if (line.find("gets") != std::string::npos) {
            fix.new_code = "    // 使用 std::getline 或 fgets 替代 gets\n"
                          "    // std::string line;\n"
                          "    // std::getline(std::cin, line);";
        } else {
            return std::nullopt;
        }

        return fix;
    }

    return std::nullopt;
}

std::optional<FixAction> AutoFixer::fixMemoryLeak(const Issue& issue) {
    FixAction fix;
    fix.type = FixType::ADD_INCLUDE;
    fix.file_path = issue.file_path;
    fix.line_start = 1;
    fix.description = "建议使用智能指针";

    // 添加 memory 头文件
    fix.new_code = "#include <memory>";

    return fix;
}

std::optional<FixAction> AutoFixer::fixBufferOverflow(const Issue& issue) {
    // 缓冲区溢出通常需要更复杂的分析，这里提供基本建议
    return std::nullopt;
}

std::optional<FixAction> AutoFixer::fixIntegerOverflow(const Issue& issue) {
    // 整数溢出修复需要上下文分析
    return std::nullopt;
}

std::optional<FixAction> AutoFixer::fixUseAfterFree(const Issue& issue) {
    // Use-After-Free 修复需要数据流分析
    return std::nullopt;
}

// ============================================================================
// 辅助方法
// ============================================================================

bool AutoFixer::createBackup(const std::string& file_path) {
    try {
        std::string backup_path = file_path + ".backup";
        std::filesystem::copy_file(file_path, backup_path,
                                   std::filesystem::copy_options::overwrite_existing);
        backup_files_.push_back(backup_path);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "备份失败: " << e.what() << "\n";
        return false;
    }
}

bool AutoFixer::writeFile(const std::string& file_path, const std::string& content) {
    try {
        std::ofstream out(file_path);
        if (!out) {
            return false;
        }
        out << content;
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

std::string AutoFixer::readFile(const std::string& file_path) {
    std::ifstream in(file_path);
    if (!in) {
        return "";
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

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

    return (response == "y" || response == "Y" || response == "yes" || response == "Yes");
}

bool AutoFixer::rollback() {
    bool success = true;

    std::cout << "\n🔄 回滚修复...\n";

    for (const auto& backup_path : backup_files_) {
        try {
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
