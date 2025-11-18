/*
 * 数据流污点分析实现
 */

#include "analysis/taint_analysis.h"
#include <clang/AST/Expr.h>
#include <iostream>
#include <algorithm>

namespace cpp_review {

// ============================================================================
// TaintAnalyzer 实现
// ============================================================================

TaintAnalyzer::TaintAnalyzer(clang::ASTContext* context, Reporter& reporter)
    : context_(context), reporter_(reporter) {}

void TaintAnalyzer::analyzeFunction(const clang::FunctionDecl* function) {
    if (!function || !function->hasBody()) {
        return;
    }

    // 清空之前的污点信息
    tainted_vars_.clear();
    taint_sources_.clear();

    // 遍历函数体
    const clang::Stmt* body = function->getBody();
    propagateTaint(body);
}

bool TaintAnalyzer::isInputSource(const clang::CallExpr* call) {
    std::string func_name = getFunctionName(call);
    return TaintSourceDB::isUserInputFunction(func_name);
}

bool TaintAnalyzer::isNetworkSource(const clang::CallExpr* call) {
    std::string func_name = getFunctionName(call);
    return TaintSourceDB::isNetworkFunction(func_name);
}

bool TaintAnalyzer::isFileSource(const clang::CallExpr* call) {
    std::string func_name = getFunctionName(call);
    return TaintSourceDB::isFileFunction(func_name);
}

bool TaintAnalyzer::isEnvironmentSource(const clang::CallExpr* call) {
    std::string func_name = getFunctionName(call);
    return func_name == "getenv" || func_name == "std::getenv";
}

bool TaintAnalyzer::isSQLSink(const clang::CallExpr* call) {
    std::string func_name = getFunctionName(call);
    return TaintSinkDB::isSQLSinkFunction(func_name);
}

bool TaintAnalyzer::isCommandSink(const clang::CallExpr* call) {
    std::string func_name = getFunctionName(call);
    return TaintSinkDB::isCommandSinkFunction(func_name);
}

bool TaintAnalyzer::isFilePathSink(const clang::CallExpr* call) {
    std::string func_name = getFunctionName(call);
    return TaintSinkDB::isFilePathSinkFunction(func_name);
}

bool TaintAnalyzer::isFormatStringSink(const clang::CallExpr* call) {
    std::string func_name = getFunctionName(call);
    return func_name == "printf" || func_name == "sprintf" ||
           func_name == "fprintf" || func_name == "snprintf";
}

bool TaintAnalyzer::isSanitizationFunction(const clang::CallExpr* call) {
    std::string func_name = getFunctionName(call);

    // 常见的净化函数
    static const std::set<std::string> sanitize_funcs = {
        "escape", "sanitize", "validate", "filter",
        "htmlspecialchars", "mysql_real_escape_string",
        "pg_escape_string", "quote", "escapeshellarg"
    };

    for (const auto& sf : sanitize_funcs) {
        if (func_name.find(sf) != std::string::npos) {
            return true;
        }
    }

    return false;
}

void TaintAnalyzer::propagateTaint(const clang::Stmt* stmt) {
    if (!stmt) {
        return;
    }

    // 处理赋值表达式
    if (const auto* binOp = clang::dyn_cast<clang::BinaryOperator>(stmt)) {
        if (binOp->getOpcode() == clang::BO_Assign) {
            std::string lhs = getVariableName(binOp->getLHS());
            std::string rhs = getVariableName(binOp->getRHS());

            // 如果右侧被污染，传播到左侧
            if (isTainted(rhs)) {
                TaintSource source = taint_sources_[rhs];
                source.variable_name = lhs;
                markTainted(lhs, source);
            }

            // 检查右侧是否为污点源
            if (const auto* call = clang::dyn_cast<clang::CallExpr>(binOp->getRHS())) {
                if (isInputSource(call) || isNetworkSource(call) ||
                    isFileSource(call) || isEnvironmentSource(call)) {

                    TaintSource source;
                    source.variable_name = lhs;
                    source.type = TaintSourceDB::getTaintType(getFunctionName(call));
                    source.line = context_->getFullLoc(call->getBeginLoc()).getSpellingLineNumber();
                    source.column = context_->getFullLoc(call->getBeginLoc()).getSpellingColumnNumber();
                    source.description = "Tainted data from " + getFunctionName(call);

                    markTainted(lhs, source);
                }
            }
        }
    }

    // 处理函数调用
    if (const auto* call = clang::dyn_cast<clang::CallExpr>(stmt)) {
        // 检查是否为净化函数
        if (isSanitizationFunction(call)) {
            // 从污点集合中移除被净化的变量
            for (unsigned i = 0; i < call->getNumArgs(); ++i) {
                std::string arg_name = getVariableName(call->getArg(i));
                tainted_vars_.erase(arg_name);
                taint_sources_.erase(arg_name);
            }
        }
        // 检查是否为敏感操作（汇点）
        else if (isSQLSink(call) || isCommandSink(call) ||
                 isFilePathSink(call) || isFormatStringSink(call)) {

            // 检查参数是否被污染
            for (unsigned i = 0; i < call->getNumArgs(); ++i) {
                std::string arg_name = getVariableName(call->getArg(i));

                if (isTainted(arg_name)) {
                    // 发现污点流向敏感操作
                    TaintSource source = taint_sources_[arg_name];

                    TaintSink sink;
                    sink.function_name = getFunctionName(call);
                    sink.tainted_args.push_back(i);
                    sink.line = context_->getFullLoc(call->getBeginLoc()).getSpellingLineNumber();
                    sink.column = context_->getFullLoc(call->getBeginLoc()).getSpellingColumnNumber();
                    sink.risk_type = TaintSinkDB::getRiskType(sink.function_name);
                    sink.severity = TaintSinkDB::getSeverity(sink.function_name);

                    reportTaintFlow(source, sink);
                }
            }
        }
    }

    // 递归遍历子语句
    for (const auto* child : stmt->children()) {
        propagateTaint(child);
    }
}

void TaintAnalyzer::markTainted(const std::string& var_name, const TaintSource& source) {
    tainted_vars_.insert(var_name);
    taint_sources_[var_name] = source;
}

bool TaintAnalyzer::isTainted(const std::string& var_name) const {
    return tainted_vars_.find(var_name) != tainted_vars_.end();
}

void TaintAnalyzer::reportTaintFlow(const TaintSource& source, const TaintSink& sink) {
    // 记录污点路径
    TaintPath path;
    path.source = source;
    path.sink = sink;
    path.propagation.push_back(source.variable_name);
    taint_paths_.push_back(path);

    // 生成问题报告
    Issue issue;
    issue.file_path = context_->getSourceManager().getFilename(
        context_->getSourceManager().getLocForStartOfFile(
            context_->getSourceManager().getMainFileID())).str();
    issue.line = sink.line;
    issue.column = sink.column;
    issue.severity = sink.severity;
    issue.rule_id = "TAINT-ANALYSIS-001";

    std::stringstream desc;
    desc << "潜在的" << sink.risk_type << "漏洞: "
         << "不受信任的数据从 '" << source.variable_name
         << "' (第 " << source.line << " 行) "
         << "流向敏感函数 '" << sink.function_name << "'";
    issue.description = desc.str();

    std::stringstream sugg;
    sugg << "验证和净化输入数据:\n";
    sugg << "1. 在第 " << source.line << " 行后立即验证 '"
         << source.variable_name << "'\n";
    sugg << "2. 使用参数化查询或预编译语句\n";
    sugg << "3. 应用适当的转义函数\n";
    sugg << "4. 实施白名单验证\n\n";
    sugg << "示例修复:\n";

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

std::string TaintAnalyzer::getVariableName(const clang::Expr* expr) {
    if (!expr) {
        return "";
    }

    // 去除隐式转换
    expr = expr->IgnoreImpCasts();

    // 处理变量引用
    if (const auto* declRef = clang::dyn_cast<clang::DeclRefExpr>(expr)) {
        return declRef->getNameInfo().getAsString();
    }

    // 处理成员访问
    if (const auto* member = clang::dyn_cast<clang::MemberExpr>(expr)) {
        return member->getMemberNameInfo().getAsString();
    }

    return "";
}

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

bool TaintAnalyzer::checkArgumentTaint(const clang::CallExpr* call, int arg_index) {
    if (!call || arg_index >= static_cast<int>(call->getNumArgs())) {
        return false;
    }

    std::string arg_name = getVariableName(call->getArg(arg_index));
    return isTainted(arg_name);
}

// ============================================================================
// TaintAnalysisVisitor 实现
// ============================================================================

TaintAnalysisVisitor::TaintAnalysisVisitor(clang::ASTContext* context, Reporter& reporter)
    : context_(context), reporter_(reporter) {}

bool TaintAnalysisVisitor::VisitFunctionDecl(clang::FunctionDecl* function) {
    if (function && function->hasBody()) {
        TaintAnalyzer analyzer(context_, reporter_);
        analyzer.analyzeFunction(function);
    }
    return true;
}

// ============================================================================
// TaintAnalysisRule 实现
// ============================================================================

void TaintAnalysisRule::check(clang::ASTContext* context, Reporter& reporter) {
    TaintAnalysisVisitor visitor(context, reporter);
    visitor.TraverseDecl(context->getTranslationUnitDecl());
}

// ============================================================================
// TaintSourceDB 实现
// ============================================================================

bool TaintSourceDB::isUserInputFunction(const std::string& func_name) {
    static const std::set<std::string> input_funcs = {
        "gets", "fgets", "getline", "scanf", "fscanf", "sscanf",
        "cin", "getchar", "fgetc", "read", "recv", "recvfrom",
        "std::cin", "std::getline", "getopt", "getopt_long"
    };

    return input_funcs.find(func_name) != input_funcs.end() ||
           func_name.find("Input") != std::string::npos ||
           func_name.find("Read") != std::string::npos;
}

bool TaintSourceDB::isNetworkFunction(const std::string& func_name) {
    static const std::set<std::string> network_funcs = {
        "recv", "recvfrom", "recvmsg", "read", "readv",
        "SSL_read", "SSL_recv", "accept", "accept4"
    };

    return network_funcs.find(func_name) != network_funcs.end();
}

bool TaintSourceDB::isFileFunction(const std::string& func_name) {
    static const std::set<std::string> file_funcs = {
        "fread", "fgets", "fgetc", "fscanf", "read",
        "readfile", "file_get_contents"
    };

    return file_funcs.find(func_name) != file_funcs.end();
}

TaintType TaintSourceDB::getTaintType(const std::string& func_name) {
    if (isUserInputFunction(func_name)) return TaintType::USER_INPUT;
    if (isNetworkFunction(func_name)) return TaintType::NETWORK_DATA;
    if (isFileFunction(func_name)) return TaintType::FILE_DATA;
    if (func_name == "getenv") return TaintType::ENVIRONMENT;
    return TaintType::UNKNOWN;
}

// ============================================================================
// TaintSinkDB 实现
// ============================================================================

bool TaintSinkDB::isSQLSinkFunction(const std::string& func_name) {
    static const std::set<std::string> sql_funcs = {
        "mysql_query", "mysql_real_query", "PQexec", "PQexecParams",
        "sqlite3_exec", "sqlite3_prepare", "exec", "execute",
        "query", "executeQuery", "executeSql"
    };

    return sql_funcs.find(func_name) != sql_funcs.end() ||
           func_name.find("query") != std::string::npos ||
           func_name.find("Query") != std::string::npos ||
           func_name.find("exec") != std::string::npos;
}

bool TaintSinkDB::isCommandSinkFunction(const std::string& func_name) {
    static const std::set<std::string> cmd_funcs = {
        "system", "popen", "exec", "execl", "execlp", "execle",
        "execv", "execvp", "execvpe", "ShellExecute", "WinExec"
    };

    return cmd_funcs.find(func_name) != cmd_funcs.end();
}

bool TaintSinkDB::isFilePathSinkFunction(const std::string& func_name) {
    static const std::set<std::string> file_funcs = {
        "fopen", "open", "openat", "creat", "freopen",
        "remove", "unlink", "rmdir", "mkdir", "chmod"
    };

    return file_funcs.find(func_name) != file_funcs.end();
}

std::string TaintSinkDB::getRiskType(const std::string& func_name) {
    if (isSQLSinkFunction(func_name)) return "SQL注入";
    if (isCommandSinkFunction(func_name)) return "命令注入";
    if (isFilePathSinkFunction(func_name)) return "路径遍历";
    return "数据污染";
}

Severity TaintSinkDB::getSeverity(const std::string& func_name) {
    if (isSQLSinkFunction(func_name)) return Severity::CRITICAL;
    if (isCommandSinkFunction(func_name)) return Severity::CRITICAL;
    if (isFilePathSinkFunction(func_name)) return Severity::HIGH;
    return Severity::MEDIUM;
}

} // namespace cpp_review
