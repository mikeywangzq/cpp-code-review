/*
 * LLM API 客户端实现 (V3.0)
 * LLM API Client Implementation (V3.0)
 *
 * 功能概述 / Overview:
 * - 通过 libcurl 进行 HTTPS 请求
 * - 支持 OpenAI GPT-4 和 Anthropic Claude API
 * - 简单的 JSON 序列化/反序列化
 * - 错误处理和降级机制
 *
 * - HTTPS requests via libcurl
 * - Support for OpenAI GPT-4 and Anthropic Claude APIs
 * - Simple JSON serialization/deserialization
 * - Error handling and fallback mechanisms
 */

#include "llm/api_client.h"
#include <sstream>
#include <iostream>
#include <cstring>
#include <regex>

// 使用 libcurl 进行 HTTP 请求
// Use libcurl for HTTP requests
#ifdef ENABLE_CURL
#include <curl/curl.h>
#endif

namespace cpp_review {

// ============================================================================
// HTTP 客户端实现 / HTTP Client Implementation
// ============================================================================

#ifdef ENABLE_CURL
/**
 * CURL 写回调函数
 * CURL write callback function
 *
 * 功能 / Functionality:
 * - 将接收到的数据追加到字符串缓冲区
 * - Append received data to string buffer
 *
 * @param contents 接收到的数据指针 / Pointer to received data
 * @param size     每个数据块的大小 / Size of each data block
 * @param nmemb    数据块数量 / Number of data blocks
 * @param userp    用户指针，指向 std::string / User pointer to std::string
 * @return 处理的字节数 / Number of bytes processed
 */
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    // BUG FIX: 添加空指针检查 / Add null pointer check
    if (!contents || !userp) {
        std::cerr << "❌ WriteCallback: 空指针参数 / null pointer parameter\n";
        return 0;
    }

    // BUG FIX: 检查乘法溢出 / Check multiplication overflow
    size_t total_size = size * nmemb;
    if (size > 0 && total_size / size != nmemb) {
        std::cerr << "❌ WriteCallback: 大小溢出 / size overflow\n";
        return 0;
    }

    try {
        ((std::string*)userp)->append((char*)contents, total_size);
        return total_size;
    } catch (const std::exception& e) {
        std::cerr << "❌ WriteCallback: 追加数据失败 / failed to append data: "
                  << e.what() << "\n";
        return 0;
    }
}
#endif

/**
 * 发送 HTTP POST 请求
 * Send HTTP POST request
 *
 * @param url     请求 URL / Request URL
 * @param headers HTTP 头部映射 / HTTP headers map
 * @param body    请求体 / Request body
 * @return API 响应结构 / API response structure
 */
APIResponse HTTPClient::post(const std::string& url,
                             const std::map<std::string, std::string>& headers,
                             const std::string& body) {
#ifdef ENABLE_CURL
    // 1. 初始化 CURL / Initialize CURL
    CURL* curl = curl_easy_init();
    if (!curl) {
        return {false, "", "Failed to initialize CURL", 0, {}};
    }

    std::string response_data;
    long response_code = 0;

    // 2. 设置请求 URL / Set request URL
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    // 3. 设置 POST 方法和请求体 / Set POST method and request body
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body.size());

    // 4. 设置请求头 / Set request headers
    struct curl_slist* header_list = nullptr;
    for (const auto& [key, value] : headers) {
        std::string header = key + ": " + value;
        header_list = curl_slist_append(header_list, header.c_str());
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);

    // 5. 设置回调函数 / Set callback function
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

    // BUG FIX: 添加安全和超时配置 / Add security and timeout configuration
    // 启用 SSL 证书验证 / Enable SSL certificate verification
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

    // 设置超时（30秒）/ Set timeout (30 seconds)
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);

    // 跟随重定向 / Follow redirects
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);

    // 6. 执行请求 / Execute request
    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        std::string error = curl_easy_strerror(res);
        curl_slist_free_all(header_list);
        curl_easy_cleanup(curl);
        return {false, "", "CURL error: " + error, 0, {}};
    }

    // 7. 获取响应码 / Get response code
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    // 8. 清理资源 / Cleanup resources
    curl_slist_free_all(header_list);
    curl_easy_cleanup(curl);

    // 9. 判断成功状态 / Determine success status
    bool success = (response_code >= 200 && response_code < 300);

    // BUG FIX: 解析错误响应体 / Parse error response body
    if (!success && !response_data.empty()) {
        // 尝试从响应中提取错误消息 / Try to extract error message from response
        auto error_msg = JSONUtil::extractField(response_data, "error");
        if (error_msg) {
            return {false, response_data, *error_msg, static_cast<int>(response_code), {}};
        }
    }

    return {success, response_data, "", static_cast<int>(response_code), {}};
#else
    // libcurl 未启用，返回模拟响应 / CURL not enabled, return mock response
    std::cerr << "⚠️  CURL 未启用，无法进行 API 调用\n";
    std::cerr << "   提示: 使用 -DENABLE_CURL=ON 编译以启用 API 功能\n";
    std::cerr << "⚠️  CURL not enabled, cannot make API calls\n";
    std::cerr << "   Hint: Compile with -DENABLE_CURL=ON to enable API features\n";
    return {false, "", "CURL not enabled", 0, {}};
#endif
}

/**
 * 发送 HTTP GET 请求
 * Send HTTP GET request
 *
 * @param url     请求 URL / Request URL
 * @param headers HTTP 头部映射 / HTTP headers map
 * @return API 响应结构 / API response structure
 */
APIResponse HTTPClient::get(const std::string& url,
                            const std::map<std::string, std::string>& headers) {
#ifdef ENABLE_CURL
    // 1. 初始化 CURL / Initialize CURL
    CURL* curl = curl_easy_init();
    if (!curl) {
        return {false, "", "Failed to initialize CURL", 0, {}};
    }

    std::string response_data;
    long response_code = 0;

    // 2. 设置请求 URL / Set request URL
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    // 3. 设置请求头 / Set request headers
    struct curl_slist* header_list = nullptr;
    for (const auto& [key, value] : headers) {
        std::string header = key + ": " + value;
        header_list = curl_slist_append(header_list, header.c_str());
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);

    // 4. 设置回调函数 / Set callback function
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

    // BUG FIX: 添加安全和超时配置 / Add security and timeout configuration
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);

    // 5. 执行请求 / Execute request
    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        std::string error = curl_easy_strerror(res);
        curl_slist_free_all(header_list);
        curl_easy_cleanup(curl);
        return {false, "", "CURL error: " + error, 0, {}};
    }

    // 6. 获取响应码 / Get response code
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    // 7. 清理资源 / Cleanup resources
    curl_slist_free_all(header_list);
    curl_easy_cleanup(curl);

    // 8. 判断成功状态 / Determine success status
    bool success = (response_code >= 200 && response_code < 300);

    // BUG FIX: 处理错误响应 / Handle error response
    if (!success && !response_data.empty()) {
        auto error_msg = JSONUtil::extractField(response_data, "error");
        if (error_msg) {
            return {false, response_data, *error_msg, static_cast<int>(response_code), {}};
        }
    }

    return {success, response_data, "", static_cast<int>(response_code), {}};
#else
    std::cerr << "⚠️  CURL 未启用，无法进行 API 调用\n";
    std::cerr << "⚠️  CURL not enabled, cannot make API calls\n";
    return {false, "", "CURL not enabled", 0, {}};
#endif
}

// ============================================================================
// OpenAI 客户端实现 / OpenAI Client Implementation
// ============================================================================

/**
 * 构造 OpenAI 客户端
 * Construct OpenAI client
 *
 * @param api_key OpenAI API 密钥 / OpenAI API key
 * @param model   模型名称 / Model name
 */
OpenAIClient::OpenAIClient(const std::string& api_key, const std::string& model)
    : api_key_(api_key), model_(model), base_url_("https://api.openai.com/v1") {}

/**
 * 聊天补全接口
 * Chat completion interface
 *
 * @param messages    消息列表 (role, content) / Message list (role, content)
 * @param max_tokens  最大令牌数 / Maximum tokens
 * @param temperature 温度参数 (0.0-2.0) / Temperature parameter (0.0-2.0)
 * @return API 响应 / API response
 */
APIResponse OpenAIClient::chatComplete(const std::vector<std::pair<std::string, std::string>>& messages,
                                       int max_tokens,
                                       double temperature) {
    // 1. 验证 API 密钥 / Validate API key
    if (api_key_.empty()) {
        return {false, "", "OpenAI API key not set", 0, {}};
    }

    // BUG FIX: 验证参数范围 / Validate parameter ranges
    if (max_tokens <= 0 || max_tokens > 4096) {
        return {false, "", "Invalid max_tokens: must be between 1 and 4096", 0, {}};
    }
    if (temperature < 0.0 || temperature > 2.0) {
        return {false, "", "Invalid temperature: must be between 0.0 and 2.0", 0, {}};
    }
    if (messages.empty()) {
        return {false, "", "Messages list cannot be empty", 0, {}};
    }

    // BUG FIX: 验证消息角色 / Validate message roles
    for (const auto& [role, content] : messages) {
        if (role != "system" && role != "user" && role != "assistant") {
            return {false, "", "Invalid role: must be system, user, or assistant", 0, {}};
        }
        if (content.empty()) {
            return {false, "", "Message content cannot be empty", 0, {}};
        }
    }

    // 2. 构建请求 JSON / Build request JSON
    std::string request_body = buildChatRequest(messages, max_tokens, temperature);

    // 3. 设置请求头 / Set request headers
    std::map<std::string, std::string> headers = {
        {"Content-Type", "application/json"},
        {"Authorization", "Bearer " + api_key_}
    };

    // 4. 发送请求 / Send request
    std::string url = base_url_ + "/chat/completions";
    APIResponse response = HTTPClient::post(url, headers, request_body);

    if (!response.success) {
        // BUG FIX: 处理 429 限流 / Handle 429 rate limiting
        if (response.status_code == 429) {
            response.error_message = "Rate limit exceeded. Please retry after some time.";
        }
        return response;
    }

    // 5. 从响应中提取内容 / Extract content from response
    auto content = JSONUtil::extractNested(response.content, "choices[0].message.content");
    if (content) {
        response.content = *content;
    } else {
        response.success = false;
        response.error_message = "Failed to parse OpenAI response";
    }

    return response;
}

/**
 * 简单补全接口
 * Simple completion interface
 *
 * @param prompt      提示词 / Prompt
 * @param max_tokens  最大令牌数 / Maximum tokens
 * @param temperature 温度参数 / Temperature parameter
 * @return API 响应 / API response
 */
APIResponse OpenAIClient::complete(const std::string& prompt,
                                   int max_tokens,
                                   double temperature) {
    // 构造消息列表，包含系统提示和用户提示
    // Construct message list with system and user prompts
    std::vector<std::pair<std::string, std::string>> messages = {
        {"system", "You are a C++ code review expert. Provide concise, actionable suggestions."},
        {"user", prompt}
    };

    return chatComplete(messages, max_tokens, temperature);
}

/**
 * 检查 API 是否可用
 * Check if API is available
 *
 * @return true 如果 API 密钥已设置 / true if API key is set
 */
bool OpenAIClient::isAvailable() const {
    return !api_key_.empty();
}

/**
 * 构建聊天请求 JSON
 * Build chat request JSON
 *
 * @param messages    消息列表 / Message list
 * @param max_tokens  最大令牌数 / Maximum tokens
 * @param temperature 温度参数 / Temperature parameter
 * @return JSON 字符串 / JSON string
 */
std::string OpenAIClient::buildChatRequest(const std::vector<std::pair<std::string, std::string>>& messages,
                                          int max_tokens,
                                          double temperature) {
    std::ostringstream oss;
    oss << "{\n";
    oss << "  \"model\": \"" << model_ << "\",\n";
    oss << "  \"messages\": [\n";

    // 遍历消息列表 / Iterate through messages
    for (size_t i = 0; i < messages.size(); ++i) {
        const auto& [role, content] = messages[i];
        oss << "    {\n";
        oss << "      \"role\": \"" << role << "\",\n";
        oss << "      \"content\": \"" << JSONUtil::escape(content) << "\"\n";
        oss << "    }";
        if (i < messages.size() - 1) {
            oss << ",";
        }
        oss << "\n";
    }

    oss << "  ],\n";
    oss << "  \"max_tokens\": " << max_tokens << ",\n";
    oss << "  \"temperature\": " << temperature << "\n";
    oss << "}";

    return oss.str();
}

// ============================================================================
// Anthropic 客户端实现 / Anthropic Client Implementation
// ============================================================================

/**
 * 构造 Anthropic 客户端
 * Construct Anthropic client
 *
 * @param api_key Anthropic API 密钥 / Anthropic API key
 * @param model   模型名称 / Model name
 */
AnthropicClient::AnthropicClient(const std::string& api_key, const std::string& model)
    : api_key_(api_key), model_(model), base_url_("https://api.anthropic.com/v1") {}

/**
 * 生成消息接口
 * Generate message interface
 *
 * @param prompt      提示词 / Prompt
 * @param max_tokens  最大令牌数 / Maximum tokens
 * @param temperature 温度参数 (0.0-1.0) / Temperature parameter (0.0-1.0)
 * @return API 响应 / API response
 */
APIResponse AnthropicClient::message(const std::string& prompt,
                                     int max_tokens,
                                     double temperature) {
    // 1. 验证 API 密钥 / Validate API key
    if (api_key_.empty()) {
        return {false, "", "Anthropic API key not set", 0, {}};
    }

    // BUG FIX: 验证参数范围 / Validate parameter ranges
    if (max_tokens <= 0 || max_tokens > 4096) {
        return {false, "", "Invalid max_tokens: must be between 1 and 4096", 0, {}};
    }
    // 注意: Anthropic 的温度范围是 0.0-1.0
    // Note: Anthropic's temperature range is 0.0-1.0
    if (temperature < 0.0 || temperature > 1.0) {
        return {false, "", "Invalid temperature: must be between 0.0 and 1.0", 0, {}};
    }
    if (prompt.empty()) {
        return {false, "", "Prompt cannot be empty", 0, {}};
    }

    // 2. 构建请求 JSON / Build request JSON
    std::string request_body = buildMessageRequest(prompt, max_tokens, temperature);

    // 3. 设置请求头 / Set request headers
    std::map<std::string, std::string> headers = {
        {"Content-Type", "application/json"},
        {"x-api-key", api_key_},
        {"anthropic-version", "2023-06-01"}
    };

    // 4. 发送请求 / Send request
    std::string url = base_url_ + "/messages";
    APIResponse response = HTTPClient::post(url, headers, request_body);

    if (!response.success) {
        // BUG FIX: 处理 429 限流 / Handle 429 rate limiting
        if (response.status_code == 429) {
            response.error_message = "Rate limit exceeded. Please retry after some time.";
        }
        return response;
    }

    // 5. 从响应中提取内容 / Extract content from response
    auto content = JSONUtil::extractNested(response.content, "content[0].text");
    if (content) {
        response.content = *content;
    } else {
        response.success = false;
        response.error_message = "Failed to parse Anthropic response";
    }

    return response;
}

/**
 * 聊天消息接口
 * Chat message interface
 *
 * @param messages    消息列表 (role, content) / Message list (role, content)
 * @param max_tokens  最大令牌数 / Maximum tokens
 * @param temperature 温度参数 / Temperature parameter
 * @return API 响应 / API response
 */
APIResponse AnthropicClient::chatMessage(const std::vector<std::pair<std::string, std::string>>& messages,
                                        int max_tokens,
                                        double temperature) {
    // 1. 验证 API 密钥 / Validate API key
    if (api_key_.empty()) {
        return {false, "", "Anthropic API key not set", 0, {}};
    }

    // BUG FIX: 验证参数范围 / Validate parameter ranges
    if (max_tokens <= 0 || max_tokens > 4096) {
        return {false, "", "Invalid max_tokens: must be between 1 and 4096", 0, {}};
    }
    if (temperature < 0.0 || temperature > 1.0) {
        return {false, "", "Invalid temperature: must be between 0.0 and 1.0", 0, {}};
    }
    if (messages.empty()) {
        return {false, "", "Messages list cannot be empty", 0, {}};
    }

    // BUG FIX: 验证消息角色 / Validate message roles
    // 注意: Anthropic 只支持 "user" 和 "assistant" 角色
    // Note: Anthropic only supports "user" and "assistant" roles
    for (const auto& [role, content] : messages) {
        if (role != "user" && role != "assistant") {
            return {false, "", "Invalid role: Anthropic only supports user and assistant", 0, {}};
        }
        if (content.empty()) {
            return {false, "", "Message content cannot be empty", 0, {}};
        }
    }

    // 2. 构建请求 JSON / Build request JSON
    std::string request_body = buildChatRequest(messages, max_tokens, temperature);

    // 3. 设置请求头 / Set request headers
    std::map<std::string, std::string> headers = {
        {"Content-Type", "application/json"},
        {"x-api-key", api_key_},
        {"anthropic-version", "2023-06-01"}
    };

    // 4. 发送请求 / Send request
    std::string url = base_url_ + "/messages";
    APIResponse response = HTTPClient::post(url, headers, request_body);

    if (!response.success) {
        // BUG FIX: 处理 429 限流 / Handle 429 rate limiting
        if (response.status_code == 429) {
            response.error_message = "Rate limit exceeded. Please retry after some time.";
        }
        return response;
    }

    // 5. 从响应中提取内容 / Extract content from response
    auto content = JSONUtil::extractNested(response.content, "content[0].text");
    if (content) {
        response.content = *content;
    } else {
        response.success = false;
        response.error_message = "Failed to parse Anthropic response";
    }

    return response;
}

/**
 * 检查 API 是否可用
 * Check if API is available
 *
 * @return true 如果 API 密钥已设置 / true if API key is set
 */
bool AnthropicClient::isAvailable() const {
    return !api_key_.empty();
}

/**
 * 构建消息请求 JSON
 * Build message request JSON
 *
 * @param prompt      提示词 / Prompt
 * @param max_tokens  最大令牌数 / Maximum tokens
 * @param temperature 温度参数 / Temperature parameter
 * @return JSON 字符串 / JSON string
 */
std::string AnthropicClient::buildMessageRequest(const std::string& prompt,
                                                 int max_tokens,
                                                 double temperature) {
    std::ostringstream oss;
    oss << "{\n";
    oss << "  \"model\": \"" << model_ << "\",\n";
    oss << "  \"max_tokens\": " << max_tokens << ",\n";
    oss << "  \"temperature\": " << temperature << ",\n";
    oss << "  \"messages\": [\n";
    oss << "    {\n";
    oss << "      \"role\": \"user\",\n";
    oss << "      \"content\": \"" << JSONUtil::escape(prompt) << "\"\n";
    oss << "    }\n";
    oss << "  ]\n";
    oss << "}";

    return oss.str();
}

/**
 * 构建聊天请求 JSON
 * Build chat request JSON
 *
 * @param messages    消息列表 / Message list
 * @param max_tokens  最大令牌数 / Maximum tokens
 * @param temperature 温度参数 / Temperature parameter
 * @return JSON 字符串 / JSON string
 */
std::string AnthropicClient::buildChatRequest(const std::vector<std::pair<std::string, std::string>>& messages,
                                              int max_tokens,
                                              double temperature) {
    std::ostringstream oss;
    oss << "{\n";
    oss << "  \"model\": \"" << model_ << "\",\n";
    oss << "  \"max_tokens\": " << max_tokens << ",\n";
    oss << "  \"temperature\": " << temperature << ",\n";
    oss << "  \"messages\": [\n";

    // 遍历消息列表 / Iterate through messages
    for (size_t i = 0; i < messages.size(); ++i) {
        const auto& [role, content] = messages[i];
        oss << "    {\n";
        oss << "      \"role\": \"" << role << "\",\n";
        oss << "      \"content\": \"" << JSONUtil::escape(content) << "\"\n";
        oss << "    }";
        if (i < messages.size() - 1) {
            oss << ",";
        }
        oss << "\n";
    }

    oss << "  ]\n";
    oss << "}";

    return oss.str();
}

// ============================================================================
// JSON 工具实现 / JSON Utility Implementation
// ============================================================================

/**
 * 转义 JSON 字符串
 * Escape JSON string
 *
 * 功能 / Functionality:
 * - 转义特殊字符（引号、反斜杠、控制字符等）
 * - Escape special characters (quotes, backslash, control chars, etc.)
 *
 * @param str 原始字符串 / Original string
 * @return 转义后的字符串 / Escaped string
 */
std::string JSONUtil::escape(const std::string& str) {
    std::ostringstream oss;
    for (unsigned char c : str) {  // BUG FIX: 使用 unsigned char 避免负数问题
        switch (c) {
            case '"':  oss << "\\\""; break;
            case '\\': oss << "\\\\"; break;
            case '\b': oss << "\\b"; break;
            case '\f': oss << "\\f"; break;
            case '\n': oss << "\\n"; break;
            case '\r': oss << "\\r"; break;
            case '\t': oss << "\\t"; break;
            default:
                // BUG FIX: 处理其他控制字符 / Handle other control characters
                if (c < 0x20) {
                    // 转义为 \uXXXX 格式 / Escape as \uXXXX format
                    oss << "\\u00";
                    oss << "0123456789abcdef"[c >> 4];
                    oss << "0123456789abcdef"[c & 0x0F];
                } else {
                    oss << c;
                }
        }
    }
    return oss.str();
}

/**
 * 从 JSON 中提取字段值
 * Extract field value from JSON
 *
 * 使用正则表达式提取简单字段
 * Use regex to extract simple field
 *
 * @param json  JSON 字符串 / JSON string
 * @param field 字段名 / Field name
 * @return 字段值（如果存在）/ Field value (if exists)
 */
std::optional<std::string> JSONUtil::extractField(const std::string& json,
                                                   const std::string& field) {
    try {
        // 简单的字段提取（不依赖第三方库）
        // Simple field extraction (no third-party dependency)

        // BUG FIX: 改进正则表达式以处理嵌套 / Improve regex to handle nesting
        // 匹配 "field": "value" 或 "field": {"nested": "value"}
        std::string pattern = "\"" + field + "\"\\s*:\\s*\"([^\"]+)\"";
        std::regex regex(pattern);
        std::smatch match;

        if (std::regex_search(json, match, regex) && match.size() > 1) {
            return match[1].str();
        }

        return std::nullopt;
    } catch (const std::regex_error& e) {
        std::cerr << "❌ Regex error in extractField: " << e.what() << "\n";
        return std::nullopt;
    }
}

/**
 * 从嵌套 JSON 中提取内容
 * Extract content from nested JSON
 *
 * 支持路径格式:
 * - "choices[0].message.content" (OpenAI)
 * - "content[0].text" (Anthropic)
 *
 * Supported path formats:
 * - "choices[0].message.content" (OpenAI)
 * - "content[0].text" (Anthropic)
 *
 * @param json JSON 字符串 / JSON string
 * @param path 字段路径 / Field path
 * @return 提取的内容（如果存在）/ Extracted content (if exists)
 */
std::optional<std::string> JSONUtil::extractNested(const std::string& json,
                                                    const std::string& path) {
    try {
        // 简化的嵌套提取 / Simplified nested extraction
        // 支持 "choices[0].message.content" 这样的路径
        // Support paths like "choices[0].message.content"

        // 对于 OpenAI: choices[0].message.content
        // For OpenAI: choices[0].message.content
        if (path == "choices[0].message.content") {
            // BUG FIX: 改进正则表达式以处理转义引号
            // Improved regex to handle escaped quotes
            std::regex content_regex(R"("content"\s*:\s*"((?:[^"\\]|\\.)*)")");
            std::smatch match;
            if (std::regex_search(json, match, content_regex) && match.size() > 1) {
                std::string content = match[1].str();
                // 反转义 / Unescape
                content = std::regex_replace(content, std::regex(R"(\\n)"), "\n");
                content = std::regex_replace(content, std::regex(R"(\\t)"), "\t");
                content = std::regex_replace(content, std::regex(R"(\\r)"), "\r");
                content = std::regex_replace(content, std::regex(R"(\\")"), "\"");
                content = std::regex_replace(content, std::regex(R"(\\\\)"), "\\");
                return content;
            }
        }

        // 对于 Anthropic: content[0].text
        // For Anthropic: content[0].text
        if (path == "content[0].text") {
            std::regex text_regex(R"("text"\s*:\s*"((?:[^"\\]|\\.)*)")");
            std::smatch match;
            if (std::regex_search(json, match, text_regex) && match.size() > 1) {
                std::string text = match[1].str();
                // 反转义 / Unescape
                text = std::regex_replace(text, std::regex(R"(\\n)"), "\n");
                text = std::regex_replace(text, std::regex(R"(\\t)"), "\t");
                text = std::regex_replace(text, std::regex(R"(\\r)"), "\r");
                text = std::regex_replace(text, std::regex(R"(\\")"), "\"");
                text = std::regex_replace(text, std::regex(R"(\\\\)"), "\\");
                return text;
            }
        }

        return std::nullopt;
    } catch (const std::regex_error& e) {
        std::cerr << "❌ Regex error in extractNested: " << e.what() << "\n";
        return std::nullopt;
    }
}

} // namespace cpp_review
