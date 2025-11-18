/*
 * LLM API 客户端 (V3.0)
 * 支持 OpenAI GPT 和 Anthropic Claude API
 *
 * 特性:
 * - HTTP/HTTPS 请求支持
 * - JSON 序列化/反序列化
 * - 错误处理和重试机制
 * - 流式响应支持
 */

#pragma once

#include <string>
#include <optional>
#include <vector>
#include <map>

namespace cpp_review {

/**
 * API 响应结构
 */
struct APIResponse {
    bool success;                  // 请求是否成功
    std::string content;           // 响应内容
    std::string error_message;     // 错误消息
    int status_code;               // HTTP 状态码
    std::map<std::string, std::string> metadata;  // 元数据
};

/**
 * HTTP 客户端
 * 封装底层 HTTP 请求
 */
class HTTPClient {
public:
    /**
     * 发送 POST 请求
     * @param url 请求 URL
     * @param headers HTTP 头
     * @param body 请求体
     * @return API 响应
     */
    static APIResponse post(const std::string& url,
                           const std::map<std::string, std::string>& headers,
                           const std::string& body);

    /**
     * 发送 GET 请求
     * @param url 请求 URL
     * @param headers HTTP 头
     * @return API 响应
     */
    static APIResponse get(const std::string& url,
                          const std::map<std::string, std::string>& headers);
};

/**
 * OpenAI API 客户端
 * 封装 OpenAI GPT API 调用
 */
class OpenAIClient {
public:
    /**
     * 构造函数
     * @param api_key OpenAI API 密钥
     * @param model 模型名称 (默认 gpt-4)
     */
    explicit OpenAIClient(const std::string& api_key, const std::string& model = "gpt-4");

    /**
     * 生成补全
     * @param prompt 提示词
     * @param max_tokens 最大令牌数
     * @param temperature 温度参数 (0.0-2.0)
     * @return API 响应
     */
    APIResponse complete(const std::string& prompt,
                        int max_tokens = 500,
                        double temperature = 0.3);

    /**
     * 聊天补全
     * @param messages 消息列表
     * @param max_tokens 最大令牌数
     * @param temperature 温度参数
     * @return API 响应
     */
    APIResponse chatComplete(const std::vector<std::pair<std::string, std::string>>& messages,
                            int max_tokens = 500,
                            double temperature = 0.3);

    /**
     * 检查 API 是否可用
     * @return 是否可用
     */
    bool isAvailable() const;

private:
    std::string api_key_;          // API 密钥
    std::string model_;            // 模型名称
    std::string base_url_;         // API 基础 URL

    std::string buildChatRequest(const std::vector<std::pair<std::string, std::string>>& messages,
                                 int max_tokens,
                                 double temperature);
};

/**
 * Anthropic Claude API 客户端
 * 封装 Anthropic Claude API 调用
 */
class AnthropicClient {
public:
    /**
     * 构造函数
     * @param api_key Anthropic API 密钥
     * @param model 模型名称 (默认 claude-3-5-sonnet-20241022)
     */
    explicit AnthropicClient(const std::string& api_key,
                            const std::string& model = "claude-3-5-sonnet-20241022");

    /**
     * 生成消息
     * @param prompt 提示词
     * @param max_tokens 最大令牌数
     * @param temperature 温度参数 (0.0-1.0)
     * @return API 响应
     */
    APIResponse message(const std::string& prompt,
                       int max_tokens = 500,
                       double temperature = 0.3);

    /**
     * 聊天消息
     * @param messages 消息列表
     * @param max_tokens 最大令牌数
     * @param temperature 温度参数
     * @return API 响应
     */
    APIResponse chatMessage(const std::vector<std::pair<std::string, std::string>>& messages,
                           int max_tokens = 500,
                           double temperature = 0.3);

    /**
     * 检查 API 是否可用
     * @return 是否可用
     */
    bool isAvailable() const;

private:
    std::string api_key_;          // API 密钥
    std::string model_;            // 模型名称
    std::string base_url_;         // API 基础 URL

    std::string buildMessageRequest(const std::string& prompt,
                                    int max_tokens,
                                    double temperature);

    std::string buildChatRequest(const std::vector<std::pair<std::string, std::string>>& messages,
                                int max_tokens,
                                double temperature);
};

/**
 * JSON 工具类
 * 简单的 JSON 序列化/反序列化
 */
class JSONUtil {
public:
    /**
     * 转义 JSON 字符串
     * @param str 原始字符串
     * @return 转义后的字符串
     */
    static std::string escape(const std::string& str);

    /**
     * 从 JSON 中提取字段
     * @param json JSON 字符串
     * @param field 字段名
     * @return 字段值
     */
    static std::optional<std::string> extractField(const std::string& json,
                                                   const std::string& field);

    /**
     * 从嵌套 JSON 中提取内容
     * @param json JSON 字符串
     * @param path 字段路径 (如 "choices[0].message.content")
     * @return 字段值
     */
    static std::optional<std::string> extractNested(const std::string& json,
                                                    const std::string& path);
};

} // namespace cpp_review
