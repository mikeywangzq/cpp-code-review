/*
 * 数据流污点分析实现 (V3.0)
 * Data Flow Taint Analysis Implementation (V3.0)
 *
 * 功能概述 / Overview:
 * - 跟踪不受信任数据的流向 (源点 -> 传播 -> 汇点)
 * - 检测安全漏洞 (SQL注入、命令注入、路径遍历等)
 * - 识别净化函数和安全检查
 *
 * - Track untrusted data flow (source -> propagation -> sink)
 * - Detect security vulnerabilities (SQL injection, command injection, path traversal, etc.)
 * - Identify sanitization functions and security checks
 */

#include "analysis/taint_analysis.h"
#include <clang/AST/Expr.h>
#include <iostream>
#include <algorithm>

namespace cpp_review {

// ============================================================================
// TaintAnalyzer 实现 / TaintAnalyzer Implementation
// ============================================================================

/**
 * 构造污点分析器
 * Construct taint analyzer
 *
 * @param context AST 上下文 / AST context
 * @param reporter 问题报告器 / Issue reporter
 */
TaintAnalyzer::TaintAnalyzer(clang::ASTContext* context, Reporter& reporter)
    : context_(context), reporter_(reporter) {
    // BUG FIX: 验证上下文指针 / Validate context pointer
    if (!context_) {
        std::cerr << "❌ TaintAnalyzer: 空 AST 上下文 / null AST context\n";
    }
}

/**
 * 分析函数中的污点流
 * Analyze taint flow in function
 *
 * 功能 / Functionality:
 * 1. 遍历函数体中的所有语句
 * 2. 识别污点源（用户输入、网络数据等）
 * 3. 跟踪污点传播（赋值、函数调用等）
 * 4. 检测污点流向敏感操作（SQL查询、系统命令等）
 *
 * 1. Traverse all statements in function body
 * 2. Identify taint sources (user input, network data, etc.)
 * 3. Track taint propagation (assignments, function calls, etc.)
 * 4. Detect taint flowing to sensitive operations (SQL queries, system commands, etc.)
 *
 * @param function 要分析的函数声明 / Function declaration to analyze
 */
void TaintAnalyzer::analyzeFunction(const clang::FunctionDecl* function) {
    // BUG FIX: 添加更全面的验证 / Add more comprehensive validation
    if (!function) {
        std::cerr << "⚠️  TaintAnalyzer: 空函数指针 / null function pointer\n";
        return;
    }

    if (!context_) {
        std::cerr << "⚠️  TaintAnalyzer: AST 上下文未初始化 / AST context not initialized\n";
        return;
    }

    if (!function->hasBody()) {
        return;  // 没有函数体，无需分析 / No body to analyze
    }

    // 清空之前的污点信息 / Clear previous taint information
    tainted_vars_.clear();
    taint_sources_.clear();

    // 遍历函数体 / Traverse function body
    const clang::Stmt* body = function->getBody();
    propagateTaint(body);
}

/**
 * 检查函数调用是否为用户输入源
 * Check if function call is a user input source
 *
 * @param call 函数调用表达式 / Function call expression
 * @return true 如果是用户输入源 / true if user input source
 */
bool TaintAnalyzer::isInputSource(const clang::CallExpr* call) {
    std::string func_name = getFunctionName(call);
    return TaintSourceDB::isUserInputFunction(func_name);
}

/**
 * 检查函数调用是否为网络数据源
 * Check if function call is a network data source
 */
bool TaintAnalyzer::isNetworkSource(const clang::CallExpr* call) {
    std::string func_name = getFunctionName(call);
    return TaintSourceDB::isNetworkFunction(func_name);
}

/**
 * 检查函数调用是否为文件数据源
 * Check if function call is a file data source
 */
bool TaintAnalyzer::isFileSource(const clang::CallExpr* call) {
    std::string func_name = getFunctionName(call);
    return TaintSourceDB::isFileFunction(func_name);
}

/**
 * 检查函数调用是否为环境变量源
 * Check if function call is an environment variable source
 */
bool TaintAnalyzer::isEnvironmentSource(const clang::CallExpr* call) {
    std::string func_name = getFunctionName(call);
    return func_name == "getenv" || func_name == "std::getenv";
}

/**
 * 检查函数调用是否为 SQL 汇点
 * Check if function call is a SQL sink
 */
bool TaintAnalyzer::isSQLSink(const clang::CallExpr* call) {
    std::string func_name = getFunctionName(call);
    return TaintSinkDB::isSQLSinkFunction(func_name);
}

/**
 * 检查函数调用是否为命令执行汇点
 * Check if function call is a command execution sink
 */
bool TaintAnalyzer::isCommandSink(const clang::CallExpr* call) {
    std::string func_name = getFunctionName(call);
    return TaintSinkDB::isCommandSinkFunction(func_name);
}

/**
 * 检查函数调用是否为文件路径汇点
 * Check if function call is a file path sink
 */
bool TaintAnalyzer::isFilePathSink(const clang::CallExpr* call) {
    std::string func_name = getFunctionName(call);
    return TaintSinkDB::isFilePathSinkFunction(func_name);
}

/**
 * 检查函数调用是否为格式字符串汇点
 * Check if function call is a format string sink
 */
bool TaintAnalyzer::isFormatStringSink(const clang::CallExpr* call) {
    std::string func_name = getFunctionName(call);
    return func_name == "printf" || func_name == "sprintf" ||
           func_name == "fprintf" || func_name == "snprintf";
}

/**
 * 检查函数调用是否为净化函数
 * Check if function call is a sanitization function
 *
 * @param call 函数调用表达式 / Function call expression
 * @return true 如果是净化函数 / true if sanitization function
 */
bool TaintAnalyzer::isSanitizationFunction(const clang::CallExpr* call) {
    std::string func_name = getFunctionName(call);

    // 常见的净化函数 / Common sanitization functions
    static const std::set<std::string> sanitize_funcs = {
        "escape", "sanitize", "validate", "filter",
        "htmlspecialchars", "mysql_real_escape_string",
        "pg_escape_string", "quote", "escapeshellarg",
        "trim", "strip", "clean", "purify"
    };

    // BUG FIX: 更严格的匹配，避免误判
    // More strict matching to avoid false positives
    // 首先检查完全匹配 / First check exact match
    if (sanitize_funcs.find(func_name) != sanitize_funcs.end()) {
        return true;
    }

    // 然后检查函数名是否包含这些关键词（但要确保是完整单词）
    // Then check if function name contains these keywords (as whole words)
    for (const auto& sf : sanitize_funcs) {
        // 检查是否在单词边界处匹配 / Check for word boundary matches
        size_t pos = func_name.find(sf);
        if (pos != std::string::npos) {
            // 确保前后是单词边界 / Ensure word boundaries
            bool start_ok = (pos == 0 || !std::isalnum(func_name[pos - 1]));
            bool end_ok = (pos + sf.length() >= func_name.length() ||
                          !std::isalnum(func_name[pos + sf.length()]));
            if (start_ok && end_ok) {
                return true;
            }
        }
    }

    return false;
}

/**
 * 传播污点信息
 * Propagate taint information
 *
 * 功能 / Functionality:
 * - 处理赋值表达式（污点从右侧传播到左侧）
 * - 处理函数调用（识别源点、汇点、净化函数）
 * - 递归遍历子语句
 *
 * - Handle assignment expressions (taint propagates from RHS to LHS)
 * - Handle function calls (identify sources, sinks, sanitization)
 * - Recursively traverse child statements
 *
 * @param stmt 要分析的语句 / Statement to analyze
 */
void TaintAnalyzer::propagateTaint(const clang::Stmt* stmt) {
    if (!stmt) {
        return;
    }

    // 处理赋值表达式 / Handle assignment expressions
    if (const auto* binOp = clang::dyn_cast<clang::BinaryOperator>(stmt)) {
        if (binOp->getOpcode() == clang::BO_Assign) {
            std::string lhs = getVariableName(binOp->getLHS());
            std::string rhs = getVariableName(binOp->getRHS());

            // BUG FIX: 检查变量名是否为空 / Check if variable names are empty
            if (lhs.empty()) {
                // 可能是复杂表达式，跳过 / Might be complex expression, skip
                // 继续处理子语句 / Continue processing children
            } else {
                // 如果右侧被污染，传播到左侧
                // If RHS is tainted, propagate to LHS
                if (!rhs.empty() && isTainted(rhs)) {
                    // BUG FIX: 检查映射中是否存在 / Check if exists in map
                    auto it = taint_sources_.find(rhs);
                    if (it != taint_sources_.end()) {
                        TaintSource source = it->second;
                        source.variable_name = lhs;
                        markTainted(lhs, source);
                    }
                }

                // 检查右侧是否为污点源 / Check if RHS is a taint source
                if (const auto* call = clang::dyn_cast<clang::CallExpr>(binOp->getRHS())) {
                    if (isInputSource(call) || isNetworkSource(call) ||
                        isFileSource(call) || isEnvironmentSource(call)) {

                        TaintSource source;
                        source.variable_name = lhs;
                        source.type = TaintSourceDB::getTaintType(getFunctionName(call));

                        // BUG FIX: 验证位置信息有效性 / Validate location info
                        clang::SourceLocation loc = call->getBeginLoc();
                        if (loc.isValid()) {
                            clang::FullSourceLoc full_loc = context_->getFullLoc(loc);
                            if (full_loc.isValid()) {
                                source.line = full_loc.getSpellingLineNumber();
                                source.column = full_loc.getSpellingColumnNumber();
                            } else {
                                source.line = 0;
                                source.column = 0;
                            }
                        } else {
                            source.line = 0;
                            source.column = 0;
                        }

                        source.description = "Tainted data from " + getFunctionName(call);
                        markTainted(lhs, source);
                    }
                }
            }
        }
    }

    // 处理函数调用 / Handle function calls
    if (const auto* call = clang::dyn_cast<clang::CallExpr>(stmt)) {
        // 检查是否为净化函数 / Check if sanitization function
        if (isSanitizationFunction(call)) {
            // 从污点集合中移除被净化的变量
            // Remove sanitized variables from taint set
            for (unsigned i = 0; i < call->getNumArgs(); ++i) {
                std::string arg_name = getVariableName(call->getArg(i));
                if (!arg_name.empty()) {
                    tainted_vars_.erase(arg_name);
                    taint_sources_.erase(arg_name);
                }
            }
        }
        // 检查是否为敏感操作（汇点）/ Check if sensitive operation (sink)
        else if (isSQLSink(call) || isCommandSink(call) ||
                 isFilePathSink(call) || isFormatStringSink(call)) {

            // 检查参数是否被污染 / Check if arguments are tainted
            for (unsigned i = 0; i < call->getNumArgs(); ++i) {
                std::string arg_name = getVariableName(call->getArg(i));

                if (!arg_name.empty() && isTainted(arg_name)) {
                    // 发现污点流向敏感操作 / Taint flows to sensitive operation
                    // BUG FIX: 检查映射中是否存在 / Check if exists in map
                    auto it = taint_sources_.find(arg_name);
                    if (it != taint_sources_.end()) {
                        TaintSource source = it->second;

                        TaintSink sink;
                        sink.function_name = getFunctionName(call);
                        sink.tainted_args.push_back(i);

                        // BUG FIX: 验证位置信息 / Validate location info
                        clang::SourceLocation loc = call->getBeginLoc();
                        if (loc.isValid()) {
                            clang::FullSourceLoc full_loc = context_->getFullLoc(loc);
                            if (full_loc.isValid()) {
                                sink.line = full_loc.getSpellingLineNumber();
                                sink.column = full_loc.getSpellingColumnNumber();
                            } else {
                                sink.line = 0;
                                sink.column = 0;
                            }
                        } else {
                            sink.line = 0;
                            sink.column = 0;
                        }

                        sink.risk_type = TaintSinkDB::getRiskType(sink.function_name);
                        sink.severity = TaintSinkDB::getSeverity(sink.function_name);

                        reportTaintFlow(source, sink);
                    }
                }
            }
        }
    }

    // 递归遍历子语句 / Recursively traverse child statements
    // BUG FIX: 添加深度限制以防止栈溢出 / Add depth limit to prevent stack overflow
    static thread_local int depth = 0;
    const int MAX_DEPTH = 1000;

    if (depth < MAX_DEPTH) {
        depth++;
        for (const auto* child : stmt->children()) {
            propagateTaint(child);
        }
        depth--;
    } else {
        std::cerr << "⚠️  TaintAnalyzer: 达到最大递归深度 / reached max recursion depth\n";
    }
}

/**
 * 标记变量为被污染
 * Mark variable as tainted
 *
 * @param var_name 变量名 / Variable name
 * @param source 污点源信息 / Taint source information
 */
void TaintAnalyzer::markTainted(const std::string& var_name, const TaintSource& source) {
    if (var_name.empty()) {
        return;  // BUG FIX: 忽略空变量名 / Ignore empty variable names
    }
    tainted_vars_.insert(var_name);
    taint_sources_[var_name] = source;
}

/**
 * 检查变量是否被污染
 * Check if variable is tainted
 *
 * @param var_name 变量名 / Variable name
 * @return true 如果变量被污染 / true if variable is tainted
 */
bool TaintAnalyzer::isTainted(const std::string& var_name) const {
    if (var_name.empty()) {
        return false;  // BUG FIX: 空变量名不被视为污染 / Empty names not considered tainted
    }
    return tainted_vars_.find(var_name) != tainted_vars_.end();
}

/**
 * 报告污点流
 * Report taint flow
 *
 * 功能 / Functionality:
 * - 记录污点传播路径
 * - 生成安全漏洞报告
 * - 提供修复建议
 *
 * - Record taint propagation path
 * - Generate security vulnerability report
 * - Provide fix suggestions
 *
 * @param source 污点源 / Taint source
 * @param sink 污点汇 / Taint sink
 */
void TaintAnalyzer::reportTaintFlow(const TaintSource& source, const TaintSink& sink) {
    // 记录污点路径 / Record taint path
    TaintPath path;
    path.source = source;
    path.sink = sink;
    path.propagation.push_back(source.variable_name);
    taint_paths_.push_back(path);

    // 生成问题报告 / Generate issue report
    Issue issue;

    // BUG FIX: 安全地获取文件路径 / Safely get file path
    if (!context_) {
        std::cerr << "❌ reportTaintFlow: 空上下文 / null context\n";
        return;
    }

    clang::SourceManager& sm = context_->getSourceManager();
    clang::FileID main_file = sm.getMainFileID();
    if (main_file.isValid()) {
        auto filename = sm.getFilename(sm.getLocForStartOfFile(main_file));
        issue.file_path = filename.str();
    } else {
        issue.file_path = "<unknown>";
    }

    issue.line = sink.line;
    issue.column = sink.column;
    issue.severity = sink.severity;
    issue.rule_id = "TAINT-ANALYSIS-001";

    // 构建问题描述 / Build issue description
    std::stringstream desc;
    desc << "潜在的" << sink.risk_type << "漏洞: "
         << "不受信任的数据从 '" << source.variable_name
         << "' (第 " << source.line << " 行) "
         << "流向敏感函数 '" << sink.function_name << "'";
    issue.description = desc.str();

    // 构建修复建议 / Build fix suggestions
    std::stringstream sugg;
    sugg << "验证和净化输入数据:\n";
    sugg << "1. 在第 " << source.line << " 行后立即验证 '"
         << source.variable_name << "'\n";
    sugg << "2. 使用参数化查询或预编译语句\n";
    sugg << "3. 应用适当的转义函数\n";
    sugg << "4. 实施白名单验证\n\n";
    sugg << "示例修复:\n";

    // 根据风险类型提供具体修复建议 / Provide specific fix based on risk type
    if (sink.risk_type == "SQL注入") {
        sugg << "// 使用参数化查询\n";
        sugg << "PreparedStatement stmt = conn.prepareStatement(\"SELECT * FROM users WHERE id = ?\");\n";
        sugg << "stmt.setInt(1, userId);\n";
    } else if (sink.risk_type == "命令注入") {
        sugg << "// 验证输入并使用白名单\n";
        sugg << "if (!isValidCommand(" << source.variable_name << ")) {\n";
        sugg << "    throw std::invalid_argument(\"Invalid command\");\n";
        sugg << "}\n";
    } else if (sink.risk_type == "路径遍历") {
        sugg << "// 规范化路径并验证\n";
        sugg << "std::filesystem::path safe_path = std::filesystem::canonical(" << source.variable_name << ");\n";
        sugg << "if (!safe_path.string().starts_with(\"/safe/directory/\")) {\n";
        sugg << "    throw std::invalid_argument(\"Invalid path\");\n";
        sugg << "}\n";
    }

    issue.suggestion = sugg.str();

    reporter_.addIssue(issue);
}

/**
 * 获取变量名
 * Get variable name
 *
 * 功能 / Functionality:
 * - 从表达式中提取变量名
 * - 处理隐式类型转换
 * - 支持变量引用和成员访问
 *
 * - Extract variable name from expression
 * - Handle implicit type conversions
 * - Support variable references and member access
 *
 * @param expr 表达式 / Expression
 * @return 变量名（如果无法提取则返回空字符串）/ Variable name (empty if cannot extract)
 */
std::string TaintAnalyzer::getVariableName(const clang::Expr* expr) {
    if (!expr) {
        return "";
    }

    // 去除隐式转换 / Strip implicit conversions
    expr = expr->IgnoreImpCasts();

    // 处理变量引用 / Handle variable references
    if (const auto* declRef = clang::dyn_cast<clang::DeclRefExpr>(expr)) {
        return declRef->getNameInfo().getAsString();
    }

    // 处理成员访问 / Handle member access
    if (const auto* member = clang::dyn_cast<clang::MemberExpr>(expr)) {
        return member->getMemberNameInfo().getAsString();
    }

    // BUG FIX: 处理数组下标访问 / Handle array subscript access
    if (const auto* arraySubscript = clang::dyn_cast<clang::ArraySubscriptExpr>(expr)) {
        // 返回数组基础变量名 / Return base array variable name
        return getVariableName(arraySubscript->getBase());
    }

    // BUG FIX: 处理指针解引用 / Handle pointer dereference
    if (const auto* unaryOp = clang::dyn_cast<clang::UnaryOperator>(expr)) {
        if (unaryOp->getOpcode() == clang::UO_Deref) {
            return getVariableName(unaryOp->getSubExpr());
        }
    }

    // BUG FIX: 处理函数调用返回值 / Handle function call return values
    // 对于函数调用，我们无法直接获取变量名，返回空字符串
    // For function calls, we cannot get variable name directly, return empty
    if (clang::dyn_cast<clang::CallExpr>(expr)) {
        return "";
    }

    return "";
}

/**
 * 获取函数名
 * Get function name
 *
 * @param call 函数调用表达式 / Function call expression
 * @return 函数名 / Function name
 */
std::string TaintAnalyzer::getFunctionName(const clang::CallExpr* call) {
    if (!call) {
        return "";
    }

    const clang::FunctionDecl* func = call->getDirectCallee();
    if (func) {
        return func->getNameAsString();
    }

    return "";
}

/**
 * 检查函数参数是否被污染
 * Check if function argument is tainted
 *
 * @param call 函数调用表达式 / Function call expression
 * @param arg_index 参数索引 / Argument index
 * @return true 如果参数被污染 / true if argument is tainted
 */
bool TaintAnalyzer::checkArgumentTaint(const clang::CallExpr* call, int arg_index) {
    // BUG FIX: 检查负索引 / Check for negative index
    if (!call || arg_index < 0 || arg_index >= static_cast<int>(call->getNumArgs())) {
        return false;
    }

    std::string arg_name = getVariableName(call->getArg(arg_index));
    return isTainted(arg_name);
}

// ============================================================================
// TaintAnalysisVisitor 实现 / TaintAnalysisVisitor Implementation
// ============================================================================

/**
 * 构造污点分析访问器
 * Construct taint analysis visitor
 */
TaintAnalysisVisitor::TaintAnalysisVisitor(clang::ASTContext* context, Reporter& reporter)
    : context_(context), reporter_(reporter) {}

/**
 * 访问函数声明
 * Visit function declaration
 *
 * @param function 函数声明 / Function declaration
 * @return true 继续遍历 / true to continue traversal
 */
bool TaintAnalysisVisitor::VisitFunctionDecl(clang::FunctionDecl* function) {
    if (function && function->hasBody()) {
        TaintAnalyzer analyzer(context_, reporter_);
        analyzer.analyzeFunction(function);
    }
    return true;
}

// ============================================================================
// TaintAnalysisRule 实现 / TaintAnalysisRule Implementation
// ============================================================================

/**
 * 执行污点分析规则检查
 * Execute taint analysis rule check
 *
 * @param context AST 上下文 / AST context
 * @param reporter 问题报告器 / Issue reporter
 */
void TaintAnalysisRule::check(clang::ASTContext* context, Reporter& reporter) {
    if (!context) {
        std::cerr << "❌ TaintAnalysisRule: 空上下文 / null context\n";
        return;
    }

    TaintAnalysisVisitor visitor(context, reporter);
    visitor.TraverseDecl(context->getTranslationUnitDecl());
}

// ============================================================================
// TaintSourceDB 实现 / TaintSourceDB Implementation
// ============================================================================

/**
 * 检查函数是否为用户输入源
 * Check if function is a user input source
 */
bool TaintSourceDB::isUserInputFunction(const std::string& func_name) {
    static const std::set<std::string> input_funcs = {
        "gets", "fgets", "getline", "scanf", "fscanf", "sscanf",
        "cin", "getchar", "fgetc", "read", "recv", "recvfrom",
        "std::cin", "std::getline", "getopt", "getopt_long",
        // BUG FIX: 添加更多输入函数 / Add more input functions
        "fread", "gets_s", "fgets_s", "std::cin.getline",
        "readlink", "recvmsg", "input", "readline"
    };

    // 完全匹配 / Exact match
    if (input_funcs.find(func_name) != input_funcs.end()) {
        return true;
    }

    // BUG FIX: 改进模糊匹配，使用完整单词 / Improve fuzzy matching, use whole words
    // 检查是否包含 "Input" 或 "Read" (但要确保是完整单词)
    // Check if contains "Input" or "Read" (ensure whole words)
    for (const std::string& keyword : {"Input", "Read", "Receive", "Get"}) {
        size_t pos = func_name.find(keyword);
        if (pos != std::string::npos) {
            bool start_ok = (pos == 0 || !std::isalnum(func_name[pos - 1]));
            bool end_ok = (pos + keyword.length() >= func_name.length() ||
                          !std::isalnum(func_name[pos + keyword.length()]));
            if (start_ok && end_ok) {
                return true;
            }
        }
    }

    return false;
}

/**
 * 检查函数是否为网络数据源
 * Check if function is a network data source
 */
bool TaintSourceDB::isNetworkFunction(const std::string& func_name) {
    static const std::set<std::string> network_funcs = {
        "recv", "recvfrom", "recvmsg", "read", "readv",
        "SSL_read", "SSL_recv", "accept", "accept4",
        // BUG FIX: 添加更多网络函数 / Add more network functions
        "BIO_read", "gnutls_record_recv", "mbedtls_ssl_read"
    };

    return network_funcs.find(func_name) != network_funcs.end();
}

/**
 * 检查函数是否为文件数据源
 * Check if function is a file data source
 */
bool TaintSourceDB::isFileFunction(const std::string& func_name) {
    static const std::set<std::string> file_funcs = {
        "fread", "fgets", "fgetc", "fscanf", "read",
        "readfile", "file_get_contents",
        // BUG FIX: 添加更多文件函数 / Add more file functions
        "pread", "readv", "mmap", "std::ifstream::read"
    };

    return file_funcs.find(func_name) != file_funcs.end();
}

/**
 * 获取污点类型
 * Get taint type
 */
TaintType TaintSourceDB::getTaintType(const std::string& func_name) {
    if (isUserInputFunction(func_name)) return TaintType::USER_INPUT;
    if (isNetworkFunction(func_name)) return TaintType::NETWORK_DATA;
    if (isFileFunction(func_name)) return TaintType::FILE_DATA;
    if (func_name == "getenv" || func_name == "std::getenv") return TaintType::ENVIRONMENT;
    return TaintType::UNKNOWN;
}

// ============================================================================
// TaintSinkDB 实现 / TaintSinkDB Implementation
// ============================================================================

/**
 * 检查函数是否为 SQL 汇点
 * Check if function is a SQL sink
 */
bool TaintSinkDB::isSQLSinkFunction(const std::string& func_name) {
    static const std::set<std::string> sql_funcs = {
        "mysql_query", "mysql_real_query", "PQexec", "PQexecParams",
        "sqlite3_exec", "sqlite3_prepare", "exec", "execute",
        "query", "executeQuery", "executeSql",
        // BUG FIX: 添加更多 SQL 函数 / Add more SQL functions
        "PQexecPrepared", "sqlite3_prepare_v2", "sqlite3_prepare_v3",
        "MYSQL_STMT_EXECUTE", "OCIStmtExecute"
    };

    // 完全匹配 / Exact match
    if (sql_funcs.find(func_name) != sql_funcs.end()) {
        return true;
    }

    // BUG FIX: 改进模糊匹配 / Improve fuzzy matching
    for (const std::string& keyword : {"query", "Query", "exec", "Exec", "sql", "Sql", "SQL"}) {
        size_t pos = func_name.find(keyword);
        if (pos != std::string::npos) {
            bool start_ok = (pos == 0 || !std::isalnum(func_name[pos - 1]));
            bool end_ok = (pos + keyword.length() >= func_name.length() ||
                          !std::isalnum(func_name[pos + keyword.length()]));
            if (start_ok && end_ok) {
                return true;
            }
        }
    }

    return false;
}

/**
 * 检查函数是否为命令执行汇点
 * Check if function is a command execution sink
 */
bool TaintSinkDB::isCommandSinkFunction(const std::string& func_name) {
    static const std::set<std::string> cmd_funcs = {
        "system", "popen", "exec", "execl", "execlp", "execle",
        "execv", "execvp", "execvpe", "ShellExecute", "WinExec",
        // BUG FIX: 添加更多命令执行函数 / Add more command execution functions
        "CreateProcess", "fork", "posix_spawn", "wordexp"
    };

    return cmd_funcs.find(func_name) != cmd_funcs.end();
}

/**
 * 检查函数是否为文件路径汇点
 * Check if function is a file path sink
 */
bool TaintSinkDB::isFilePathSinkFunction(const std::string& func_name) {
    static const std::set<std::string> file_funcs = {
        "fopen", "open", "openat", "creat", "freopen",
        "remove", "unlink", "rmdir", "mkdir", "chmod",
        // BUG FIX: 添加更多文件路径函数 / Add more file path functions
        "chown", "link", "symlink", "rename", "stat", "lstat"
    };

    return file_funcs.find(func_name) != file_funcs.end();
}

/**
 * 获取风险类型描述
 * Get risk type description
 */
std::string TaintSinkDB::getRiskType(const std::string& func_name) {
    if (isSQLSinkFunction(func_name)) return "SQL注入";
    if (isCommandSinkFunction(func_name)) return "命令注入";
    if (isFilePathSinkFunction(func_name)) return "路径遍历";
    return "数据污染";
}

/**
 * 获取严重性级别
 * Get severity level
 */
Severity TaintSinkDB::getSeverity(const std::string& func_name) {
    if (isSQLSinkFunction(func_name)) return Severity::CRITICAL;
    if (isCommandSinkFunction(func_name)) return Severity::CRITICAL;
    if (isFilePathSinkFunction(func_name)) return Severity::HIGH;
    return Severity::MEDIUM;
}

} // namespace cpp_review
