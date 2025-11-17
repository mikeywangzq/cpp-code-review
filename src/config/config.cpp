/*
 * 配置管理实现
 * 解析 YAML 格式的配置文件
 */

#include "config/config.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>

namespace cpp_review {

/**
 * 去除字符串首尾的空白字符
 * @param str 输入字符串
 * @return 去除空白后的字符串
 */
std::string ConfigManager::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";

    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

/**
 * 解析配置文件的单行
 * 支持简单的 YAML 格式: key: value
 * @param line 配置文件的一行
 * @param config 要更新的配置对象
 */
void ConfigManager::parseLine(const std::string& line, Config& config) {
    std::string trimmed = trim(line);

    // 跳过空行和注释行
    if (trimmed.empty() || trimmed[0] == '#') {
        return;
    }

    // 查找键值分隔符 (:)
    size_t colon_pos = trimmed.find(':');
    if (colon_pos == std::string::npos) {
        return;
    }

    std::string key = trim(trimmed.substr(0, colon_pos));
    std::string value = trim(trimmed.substr(colon_pos + 1));

    // 根据键名解析不同的配置项
    if (key == "disabled_rules") {
        // 解析禁用规则列表 (逗号或空格分隔)
        std::istringstream iss(value);
        std::string rule;

        // 移除方括号 (如果存在)
        if (!value.empty() && value[0] == '[') {
            value = value.substr(1);
        }
        if (!value.empty() && value.back() == ']') {
            value.pop_back();
        }

        // 解析逗号分隔的值
        std::istringstream stream(value);
        while (std::getline(stream, rule, ',')) {
            rule = trim(rule);
            if (!rule.empty()) {
                config.disabled_rules.insert(rule);
            }
        }
    }
    else if (key == "html_output") {
        std::string lower_value = value;
        std::transform(lower_value.begin(), lower_value.end(), lower_value.begin(), ::tolower);
        config.generate_html = (lower_value == "true" || lower_value == "yes" || lower_value == "1");
    }
    else if (key == "html_output_file") {
        config.html_output_file = value;
    }
    else if (key == "cpp_standard") {
        config.cpp_standard = value;
    }
    else if (key == "verbose") {
        std::string lower_value = value;
        std::transform(lower_value.begin(), lower_value.end(), lower_value.begin(), ::tolower);
        config.verbose = (lower_value == "true" || lower_value == "yes" || lower_value == "1");
    }
    else if (key.find("severity_") == 0) {
        // 规则严重性覆盖: severity_RULE-ID: HIGH
        std::string rule_id = key.substr(9); // 移除 "severity_" 前缀
        config.rule_severity[rule_id] = value;
    }
    // ===== V2.0 LLM 配置选项 =====
    else if (key == "enable_ai_suggestions" || key == "ai_suggestions") {
        std::string lower_value = value;
        std::transform(lower_value.begin(), lower_value.end(), lower_value.begin(), ::tolower);
        config.enable_ai_suggestions = (lower_value == "true" || lower_value == "yes" || lower_value == "1");
    }
    else if (key == "llm_provider") {
        config.llm_provider = value;
    }
    else if (key == "llm_api_key" || key == "openai_api_key") {
        config.llm_api_key = value;
    }
}

/**
 * 从文件加载配置
 * 如果文件不存在,返回默认配置
 * @param config_file 配置文件路径
 * @return 配置对象
 */
Config ConfigManager::loadConfig(const std::string& config_file) {
    Config config = getDefaultConfig();

    std::ifstream file(config_file);
    if (!file.is_open()) {
        // 配置文件不存在,使用默认配置
        return config;
    }

    // 逐行读取并解析配置文件
    std::string line;
    while (std::getline(file, line)) {
        parseLine(line, config);
    }

    return config;
}

/**
 * 获取默认配置
 * @return 包含所有默认值的配置对象
 */
Config ConfigManager::getDefaultConfig() {
    Config config;
    config.cpp_standard = "c++17";
    config.generate_html = false;
    config.html_output_file = "report.html";
    config.verbose = false;
    config.enable_ai_suggestions = false;  // 默认禁用 AI 建议
    config.llm_provider = "rule-based";    // 默认使用基于规则的提供者
    config.llm_api_key = "";
    return config;
}

} // namespace cpp_review
