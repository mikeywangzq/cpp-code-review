/*
 * 命令行接口头文件
 * 定义命令行选项结构和解析接口
 */

#pragma once

#include <string>
#include <vector>

namespace cpp_review {

/**
 * 命令行选项结构
 * 存储所有从命令行解析的选项
 */
struct CLIOptions {
    std::vector<std::string> source_paths;  // 要分析的源文件路径列表
    std::string cpp_standard = "c++17";      // C++ 标准版本
    bool help = false;                       // 是否显示帮助信息
    bool version = false;                    // 是否显示版本信息
    bool generate_html = false;              // 是否生成 HTML 报告
    std::string html_output = "report.html"; // HTML 输出文件名
    bool enable_ai = false;                  // 启用 AI 建议 (V2.0)
    std::string llm_provider = "rule-based"; // LLM 提供者 (V2.0)
};

/**
 * 命令行接口类
 * 提供参数解析和帮助信息显示功能
 */
class CLI {
public:
    // 解析命令行参数
    static CLIOptions parseArguments(int argc, char* argv[]);

    // 打印帮助信息
    static void printHelp();

    // 打印版本信息
    static void printVersion();

private:
    // 判断文件是否为 C++ 源文件
    static bool isSourceFile(const std::string& path);
};

} // namespace cpp_review
