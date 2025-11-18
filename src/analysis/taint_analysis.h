/*
 * 数据流污点分析 (V3.0)
 * 跟踪不受信任的数据流向敏感操作
 *
 * 特性:
 * - 源点识别 (用户输入、网络数据、文件读取)
 * - 污点传播跟踪 (赋值、函数调用、容器操作)
 * - 汇点检测 (SQL 查询、系统命令、文件操作)
 * - 净化函数识别 (验证、转义、编码)
 *
 * 安全问题检测:
 * - SQL 注入
 * - 命令注入
 * - 路径遍历
 * - XSS (跨站脚本)
 * - 未验证的重定向
 */

#pragma once

#include "report/reporter.h"
#include <clang/AST/AST.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <string>
#include <set>
#include <map>
#include <vector>

namespace cpp_review {

/**
 * 污点类型
 */
enum class TaintType {
    USER_INPUT,      // 用户输入
    NETWORK_DATA,    // 网络数据
    FILE_DATA,       // 文件数据
    ENVIRONMENT,     // 环境变量
    DATABASE,        // 数据库数据
    UNKNOWN          // 未知来源
};

/**
 * 污点源
 */
struct TaintSource {
    std::string variable_name;      // 变量名
    TaintType type;                 // 污点类型
    unsigned line;                  // 源行号
    unsigned column;                // 源列号
    std::string description;        // 描述
};

/**
 * 污点汇点 (敏感操作)
 */
struct TaintSink {
    std::string function_name;      // 函数名
    std::vector<int> tainted_args;  // 被污染的参数索引
    unsigned line;                  // 行号
    unsigned column;                // 列号
    std::string risk_type;          // 风险类型 (SQL注入、命令注入等)
    Severity severity;              // 严重性
};

/**
 * 污点传播路径
 */
struct TaintPath {
    TaintSource source;             // 污点源
    std::vector<std::string> propagation;  // 传播路径
    TaintSink sink;                 // 污点汇
};

/**
 * 污点分析器
 * 使用数据流分析跟踪污点传播
 */
class TaintAnalyzer {
public:
    /**
     * 构造函数
     * @param context AST 上下文
     * @param reporter 报告器
     */
    explicit TaintAnalyzer(clang::ASTContext* context, Reporter& reporter);

    /**
     * 分析函数中的污点流
     * @param function 要分析的函数
     */
    void analyzeFunction(const clang::FunctionDecl* function);

    /**
     * 获取检测到的污点路径
     * @return 污点路径列表
     */
    const std::vector<TaintPath>& getTaintPaths() const { return taint_paths_; }

private:
    clang::ASTContext* context_;           // AST 上下文
    Reporter& reporter_;                   // 报告器
    std::set<std::string> tainted_vars_;   // 被污染的变量集合
    std::map<std::string, TaintSource> taint_sources_;  // 污点源映射
    std::vector<TaintPath> taint_paths_;   // 检测到的污点路径

    // 源点识别
    bool isInputSource(const clang::CallExpr* call);
    bool isNetworkSource(const clang::CallExpr* call);
    bool isFileSource(const clang::CallExpr* call);
    bool isEnvironmentSource(const clang::CallExpr* call);

    // 汇点识别
    bool isSQLSink(const clang::CallExpr* call);
    bool isCommandSink(const clang::CallExpr* call);
    bool isFilePathSink(const clang::CallExpr* call);
    bool isFormatStringSink(const clang::CallExpr* call);

    // 净化函数识别
    bool isSanitizationFunction(const clang::CallExpr* call);

    // 污点传播
    void propagateTaint(const clang::Stmt* stmt);
    void markTainted(const std::string& var_name, const TaintSource& source);
    bool isTainted(const std::string& var_name) const;

    // 路径构建
    void reportTaintFlow(const TaintSource& source, const TaintSink& sink);

    // 辅助方法
    std::string getVariableName(const clang::Expr* expr);
    std::string getFunctionName(const clang::CallExpr* call);
    bool checkArgumentTaint(const clang::CallExpr* call, int arg_index);
};

/**
 * 污点分析访问器
 * 遍历 AST 并执行污点分析
 */
class TaintAnalysisVisitor : public clang::RecursiveASTVisitor<TaintAnalysisVisitor> {
public:
    explicit TaintAnalysisVisitor(clang::ASTContext* context, Reporter& reporter);

    bool VisitFunctionDecl(clang::FunctionDecl* function);

private:
    clang::ASTContext* context_;
    Reporter& reporter_;
};

/**
 * 污点分析规则
 * 基于 Rule 接口的污点分析规则
 */
class TaintAnalysisRule {
public:
    /**
     * 执行污点分析
     * @param context AST 上下文
     * @param reporter 报告器
     */
    static void check(clang::ASTContext* context, Reporter& reporter);

    /**
     * 获取规则 ID
     */
    static std::string getRuleId() { return "TAINT-ANALYSIS-001"; }

    /**
     * 获取规则名称
     */
    static std::string getRuleName() { return "Data Flow Taint Analysis"; }
};

/**
 * 预定义的源点函数列表
 */
class TaintSourceDB {
public:
    /**
     * 检查函数是否为用户输入源
     */
    static bool isUserInputFunction(const std::string& func_name);

    /**
     * 检查函数是否为网络数据源
     */
    static bool isNetworkFunction(const std::string& func_name);

    /**
     * 检查函数是否为文件数据源
     */
    static bool isFileFunction(const std::string& func_name);

    /**
     * 获取污点类型
     */
    static TaintType getTaintType(const std::string& func_name);
};

/**
 * 预定义的汇点函数列表
 */
class TaintSinkDB {
public:
    /**
     * 检查函数是否为 SQL 汇点
     */
    static bool isSQLSinkFunction(const std::string& func_name);

    /**
     * 检查函数是否为命令执行汇点
     */
    static bool isCommandSinkFunction(const std::string& func_name);

    /**
     * 检查函数是否为文件路径汇点
     */
    static bool isFilePathSinkFunction(const std::string& func_name);

    /**
     * 获取风险类型描述
     */
    static std::string getRiskType(const std::string& func_name);

    /**
     * 获取严重性级别
     */
    static Severity getSeverity(const std::string& func_name);
};

} // namespace cpp_review
