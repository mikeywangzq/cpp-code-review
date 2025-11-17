/*
 * 配置管理头文件
 * 支持从 YAML 配置文件加载用户设置
 */

#pragma once

#include <string>
#include <set>
#include <map>

namespace cpp_review {

/**
 * 配置结构体
 * 存储所有用户可配置的选项
 */
struct Config {
    // ===== 规则配置 =====
    std::set<std::string> disabled_rules;             // 禁用的规则 ID 列表
    std::map<std::string, std::string> rule_severity; // 规则严重性覆盖

    // ===== 输出选项 =====
    bool generate_html = false;                       // 是否生成 HTML 报告
    std::string html_output_file = "report.html";     // HTML 报告输出文件名

    // ===== 分析选项 =====
    std::string cpp_standard = "c++17";               // C++ 标准版本
    bool verbose = false;                             // 详细输出模式

    // ===== LLM 智能增强选项 (V2.0) =====
    bool enable_ai_suggestions = false;               // 启用 AI 建议
    std::string llm_provider = "rule-based";          // LLM 提供者: "rule-based", "openai", "none"
    std::string llm_api_key = "";                     // OpenAI 等外部提供者的 API 密钥
};

/**
 * 配置管理器类
 * 负责加载和解析配置文件
 */
class ConfigManager {
public:
    /**
     * 从文件加载配置
     * @param config_file 配置文件路径 (YAML 格式)
     * @return 解析后的配置对象
     */
    static Config loadConfig(const std::string& config_file);

    /**
     * 获取默认配置
     * @return 包含默认值的配置对象
     */
    static Config getDefaultConfig();

private:
    // 去除字符串首尾空白
    static std::string trim(const std::string& str);

    // 解析配置文件的单行
    static void parseLine(const std::string& line, Config& config);
};

} // namespace cpp_review
