/*
 * AST 解析器实现
 * 使用 Clang LibTooling 框架解析和分析 C++ 代码
 */

#include "parser/ast_parser.h"
#include "rules/rule_engine.h"
#include "report/reporter.h"
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>
#include <iostream>

namespace cpp_review {

// ===== AnalysisConsumer 实现 =====

AnalysisConsumer::AnalysisConsumer(RuleEngine& engine, Reporter& reporter)
    : rule_engine_(engine), reporter_(reporter) {}

/**
 * 当 AST 构建完成时被调用
 * 在这里运行所有注册的分析规则
 */
void AnalysisConsumer::HandleTranslationUnit(clang::ASTContext& context) {
    // 在整个编译单元上运行所有已注册的规则
    rule_engine_.runAllRules(&context, reporter_);
}

// ===== AnalysisAction 实现 =====

AnalysisAction::AnalysisAction(RuleEngine& engine, Reporter& reporter)
    : rule_engine_(engine), reporter_(reporter) {}

/**
 * 为每个源文件创建 AST 消费者
 * Clang 为每个文件调用此方法
 */
std::unique_ptr<clang::ASTConsumer> AnalysisAction::CreateASTConsumer(
    clang::CompilerInstance& compiler,
    llvm::StringRef file) {
    return std::make_unique<AnalysisConsumer>(rule_engine_, reporter_);
}

// ===== AnalysisActionFactory 实现 =====

AnalysisActionFactory::AnalysisActionFactory(RuleEngine& engine, Reporter& reporter)
    : rule_engine_(engine), reporter_(reporter) {}

/**
 * 创建新的前端操作实例
 * 工厂模式允许为每个文件创建独立的操作
 */
std::unique_ptr<clang::FrontendAction> AnalysisActionFactory::create() {
    return std::make_unique<AnalysisAction>(rule_engine_, reporter_);
}

// ===== ASTParser 实现 =====

ASTParser::ASTParser(const std::vector<std::string>& source_paths,
                     const std::string& cpp_standard)
    : source_paths_(source_paths), cpp_standard_(cpp_standard) {}

/**
 * 解析源文件并运行分析
 * 创建 Clang 工具实例并执行分析规则
 */
bool ASTParser::parse(RuleEngine& engine, Reporter& reporter) {
    // 构建命令行参数列表
    std::vector<std::string> args;
    args.push_back("cpp-agent");  // 程序名称

    // 添加所有源文件路径
    for (const auto& path : source_paths_) {
        args.push_back(path);
    }

    // 添加编译器选项
    args.push_back("--");
    args.push_back("-std=" + cpp_standard_);  // 指定 C++ 标准
    args.push_back("-fsyntax-only");           // 只检查语法,不生成代码
    args.push_back("-w");                      // 抑制编译器警告

    // 转换为 C 风格字符串数组 (Clang 工具接口要求)
    std::vector<const char*> c_args;
    for (const auto& arg : args) {
        c_args.push_back(arg.c_str());
    }

    int argc = static_cast<int>(c_args.size());

    // 解析命令行选项
    llvm::cl::OptionCategory category("cpp-agent options");
    auto expected_parser = clang::tooling::CommonOptionsParser::create(
        argc, c_args.data(), category);

    if (!expected_parser) {
        std::cerr << "Error creating parser: "
                 << llvm::toString(expected_parser.takeError()) << "\n";
        return false;
    }

    // 创建 Clang 工具实例
    clang::tooling::CommonOptionsParser& options_parser = expected_parser.get();
    clang::tooling::ClangTool tool(options_parser.getCompilations(),
                                   options_parser.getSourcePathList());

    // 使用我们的分析操作工厂运行工具
    AnalysisActionFactory factory(engine, reporter);
    int result = tool.run(&factory);

    // 返回分析是否成功 (0 表示成功)
    return result == 0;
}

} // namespace cpp_review
