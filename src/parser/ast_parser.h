/*
 * AST 解析器头文件
 * 使用 Clang/LLVM LibTooling 解析 C++ 代码的抽象语法树
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <clang/Tooling/Tooling.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/ASTContext.h>

namespace cpp_review {

class Reporter;
class RuleEngine;

/**
 * AST 消费者类 - 在编译单元解析完成后执行分析规则
 * 这是 Clang AST 访问的核心接口
 */
class AnalysisConsumer : public clang::ASTConsumer {
public:
    AnalysisConsumer(RuleEngine& engine, Reporter& reporter);

    // 处理整个编译单元的 AST
    void HandleTranslationUnit(clang::ASTContext& context) override;

private:
    RuleEngine& rule_engine_;  // 规则引擎引用
    Reporter& reporter_;       // 报告生成器引用
};

/**
 * 前端操作类 - 创建 AST 消费者的工厂
 * 负责为每个源文件创建分析消费者
 */
class AnalysisAction : public clang::ASTFrontendAction {
public:
    AnalysisAction(RuleEngine& engine, Reporter& reporter);

    // 为每个源文件创建一个 AST 消费者
    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
        clang::CompilerInstance& compiler,
        llvm::StringRef file) override;

private:
    RuleEngine& rule_engine_;
    Reporter& reporter_;
};

/**
 * 分析操作工厂 - 为 ClangTool 创建前端操作
 * 实现 FrontendActionFactory 接口以支持多文件分析
 */
class AnalysisActionFactory : public clang::tooling::FrontendActionFactory {
public:
    AnalysisActionFactory(RuleEngine& engine, Reporter& reporter);

    // 创建新的前端操作实例
    std::unique_ptr<clang::FrontendAction> create() override;

private:
    RuleEngine& rule_engine_;
    Reporter& reporter_;
};

/**
 * AST 解析器主类
 * 协调整个 AST 解析流程,使用 Clang Tooling API
 */
class ASTParser {
public:
    /**
     * 构造函数
     * @param source_paths 要分析的源文件路径列表
     * @param cpp_standard C++ 标准版本 (默认 c++17)
     */
    ASTParser(const std::vector<std::string>& source_paths,
              const std::string& cpp_standard = "c++17");

    /**
     * 解析源文件并运行分析规则
     * @param engine 规则引擎
     * @param reporter 报告生成器
     * @return 成功返回 true,失败返回 false
     */
    bool parse(RuleEngine& engine, Reporter& reporter);

private:
    std::vector<std::string> source_paths_;  // 源文件路径列表
    std::string cpp_standard_;                // C++ 标准版本
};

} // namespace cpp_review
