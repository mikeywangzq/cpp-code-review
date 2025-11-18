/*
 * LLM API 客户端实现
 */

#include "llm/api_client.h"
#include <sstream>
#include <iostream>
#include <cstring>
#include <regex>

// 使用 libcurl 进行 HTTP 请求
#ifdef ENABLE_CURL
#include <curl/curl.h>
#endif

namespace cpp_review {

// ============================================================================
// HTTP 客户端实现
// ============================================================================

#ifdef ENABLE_CURL
// CURL 写回调函数
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}
#endif

APIResponse HTTPClient::post(const std::string& url,
                             const std::map<std::string, std::string>& headers,
                             const std::string& body) {
#ifdef ENABLE_CURL
    CURL* curl = curl_easy_init();
    if (!curl) {
        return {false, "", "Failed to initialize CURL", 0, {}};
    }

    std::string response_data;
    long response_code = 0;

    // 设置 URL
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    // 设置 POST 方法
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());

    // 设置头部
    struct curl_slist* header_list = nullptr;
    for (const auto& [key, value] : headers) {
        std::string header = key + ": " + value;
        header_list = curl_slist_append(header_list, header.c_str());
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);

    // 设置回调
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

    // 执行请求
    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        std::string error = curl_easy_strerror(res);
        curl_slist_free_all(header_list);
        curl_easy_cleanup(curl);
        return {false, "", "CURL error: " + error, 0, {}};
    }

    // 获取响应码
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    // 清理
    curl_slist_free_all(header_list);
    curl_easy_cleanup(curl);

    bool success = (response_code >= 200 && response_code < 300);
    return {success, response_data, "", static_cast<int>(response_code), {}};
#else
    // libcurl 未启用，返回模拟响应
    std::cerr << "⚠️  CURL 未启用，无法进行 API 调用\n";
    std::cerr << "   提示: 使用 -DENABLE_CURL=ON 编译以启用 API 功能\n";
    return {false, "", "CURL not enabled", 0, {}};
#endif
}

APIResponse HTTPClient::get(const std::string& url,
                            const std::map<std::string, std::string>& headers) {
#ifdef ENABLE_CURL
    CURL* curl = curl_easy_init();
    if (!curl) {
        return {false, "", "Failed to initialize CURL", 0, {}};
    }

    std::string response_data;
    long response_code = 0;

    // 设置 URL
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    // 设置头部
    struct curl_slist* header_list = nullptr;
    for (const auto& [key, value] : headers) {
        std::string header = key + ": " + value;
        header_list = curl_slist_append(header_list, header.c_str());
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);

    // 设置回调
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

    // 执行请求
    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        std::string error = curl_easy_strerror(res);
        curl_slist_free_all(header_list);
        curl_easy_cleanup(curl);
        return {false, "", "CURL error: " + error, 0, {}};
    }

    // 获取响应码
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    // 清理
    curl_slist_free_all(header_list);
    curl_easy_cleanup(curl);

    bool success = (response_code >= 200 && response_code < 300);
    return {success, response_data, "", static_cast<int>(response_code), {}};
#else
    std::cerr << "⚠️  CURL 未启用，无法进行 API 调用\n";
    return {false, "", "CURL not enabled", 0, {}};
#endif
}

// ============================================================================
// OpenAI 客户端实现
// ============================================================================

OpenAIClient::OpenAIClient(const std::string& api_key, const std::string& model)
    : api_key_(api_key), model_(model), base_url_("https://api.openai.com/v1") {}

APIResponse OpenAIClient::chatComplete(const std::vector<std::pair<std::string, std::string>>& messages,
                                       int max_tokens,
                                       double temperature) {
    if (api_key_.empty()) {
        return {false, "", "OpenAI API key not set", 0, {}};
    }

    // 构建请求 JSON
    std::string request_body = buildChatRequest(messages, max_tokens, temperature);

    // 设置请求头
    std::map<std::string, std::string> headers = {
        {"Content-Type", "application/json"},
        {"Authorization", "Bearer " + api_key_}
    };

    // 发送请求
    std::string url = base_url_ + "/chat/completions";
    APIResponse response = HTTPClient::post(url, headers, request_body);

    if (!response.success) {
        return response;
    }

    // 从响应中提取内容
    auto content = JSONUtil::extractNested(response.content, "choices[0].message.content");
    if (content) {
        response.content = *content;
    } else {
        response.success = false;
        response.error_message = "Failed to parse OpenAI response";
    }

    return response;
}

APIResponse OpenAIClient::complete(const std::string& prompt,
                                   int max_tokens,
                                   double temperature) {
    std::vector<std::pair<std::string, std::string>> messages = {
        {"system", "You are a C++ code review expert. Provide concise, actionable suggestions."},
        {"user", prompt}
    };

    return chatComplete(messages, max_tokens, temperature);
}

bool OpenAIClient::isAvailable() const {
    return !api_key_.empty();
}

std::string OpenAIClient::buildChatRequest(const std::vector<std::pair<std::string, std::string>>& messages,
                                          int max_tokens,
                                          double temperature) {
    std::ostringstream oss;
    oss << "{\n";
    oss << "  \"model\": \"" << model_ << "\",\n";
    oss << "  \"messages\": [\n";

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
// Anthropic 客户端实现
// ============================================================================

AnthropicClient::AnthropicClient(const std::string& api_key, const std::string& model)
    : api_key_(api_key), model_(model), base_url_("https://api.anthropic.com/v1") {}

APIResponse AnthropicClient::message(const std::string& prompt,
                                     int max_tokens,
                                     double temperature) {
    if (api_key_.empty()) {
        return {false, "", "Anthropic API key not set", 0, {}};
    }

    // 构建请求 JSON
    std::string request_body = buildMessageRequest(prompt, max_tokens, temperature);

    // 设置请求头
    std::map<std::string, std::string> headers = {
        {"Content-Type", "application/json"},
        {"x-api-key", api_key_},
        {"anthropic-version", "2023-06-01"}
    };

    // 发送请求
    std::string url = base_url_ + "/messages";
    APIResponse response = HTTPClient::post(url, headers, request_body);

    if (!response.success) {
        return response;
    }

    // 从响应中提取内容
    auto content = JSONUtil::extractNested(response.content, "content[0].text");
    if (content) {
        response.content = *content;
    } else {
        response.success = false;
        response.error_message = "Failed to parse Anthropic response";
    }

    return response;
}

APIResponse AnthropicClient::chatMessage(const std::vector<std::pair<std::string, std::string>>& messages,
                                        int max_tokens,
                                        double temperature) {
    if (api_key_.empty()) {
        return {false, "", "Anthropic API key not set", 0, {}};
    }

    // 构建请求 JSON
    std::string request_body = buildChatRequest(messages, max_tokens, temperature);

    // 设置请求头
    std::map<std::string, std::string> headers = {
        {"Content-Type", "application/json"},
        {"x-api-key", api_key_},
        {"anthropic-version", "2023-06-01"}
    };

    // 发送请求
    std::string url = base_url_ + "/messages";
    APIResponse response = HTTPClient::post(url, headers, request_body);

    if (!response.success) {
        return response;
    }

    // 从响应中提取内容
    auto content = JSONUtil::extractNested(response.content, "content[0].text");
    if (content) {
        response.content = *content;
    } else {
        response.success = false;
        response.error_message = "Failed to parse Anthropic response";
    }

    return response;
}

bool AnthropicClient::isAvailable() const {
    return !api_key_.empty();
}

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

std::string AnthropicClient::buildChatRequest(const std::vector<std::pair<std::string, std::string>>& messages,
                                              int max_tokens,
                                              double temperature) {
    std::ostringstream oss;
    oss << "{\n";
    oss << "  \"model\": \"" << model_ << "\",\n";
    oss << "  \"max_tokens\": " << max_tokens << ",\n";
    oss << "  \"temperature\": " << temperature << ",\n";
    oss << "  \"messages\": [\n";

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
// JSON 工具实现
// ============================================================================

std::string JSONUtil::escape(const std::string& str) {
    std::ostringstream oss;
    for (char c : str) {
        switch (c) {
            case '"':  oss << "\\\""; break;
            case '\\': oss << "\\\\"; break;
            case '\b': oss << "\\b"; break;
            case '\f': oss << "\\f"; break;
            case '\n': oss << "\\n"; break;
            case '\r': oss << "\\r"; break;
            case '\t': oss << "\\t"; break;
            default:
                oss << c;
        }
    }
    return oss.str();
}

std::optional<std::string> JSONUtil::extractField(const std::string& json,
                                                   const std::string& field) {
    // 简单的字段提取 (不依赖第三方库)
    std::string pattern = "\"" + field + "\"\\s*:\\s*\"([^\"]+)\"";
    std::regex regex(pattern);
    std::smatch match;

    if (std::regex_search(json, match, regex) && match.size() > 1) {
        return match[1].str();
    }

    return std::nullopt;
}

std::optional<std::string> JSONUtil::extractNested(const std::string& json,
                                                    const std::string& path) {
    // 简化的嵌套提取
    // 支持 "choices[0].message.content" 这样的路径

    // 对于 OpenAI: choices[0].message.content
    if (path == "choices[0].message.content") {
        std::regex content_regex(R"("content"\s*:\s*"((?:[^"\\]|\\.)*)")");
        std::smatch match;
        if (std::regex_search(json, match, content_regex) && match.size() > 1) {
            std::string content = match[1].str();
            // 反转义
            content = std::regex_replace(content, std::regex(R"(\\n)"), "\n");
            content = std::regex_replace(content, std::regex(R"(\\t)"), "\t");
            content = std::regex_replace(content, std::regex(R"(\\")"), "\"");
            return content;
        }
    }

    // 对于 Anthropic: content[0].text
    if (path == "content[0].text") {
        std::regex text_regex(R"("text"\s*:\s*"((?:[^"\\]|\\.)*)")");
        std::smatch match;
        if (std::regex_search(json, match, text_regex) && match.size() > 1) {
            std::string text = match[1].str();
            // 反转义
            text = std::regex_replace(text, std::regex(R"(\\n)"), "\n");
            text = std::regex_replace(text, std::regex(R"(\\t)"), "\t");
            text = std::regex_replace(text, std::regex(R"(\\")"), "\"");
            return text;
        }
    }

    return std::nullopt;
}

} // namespace cpp_review
