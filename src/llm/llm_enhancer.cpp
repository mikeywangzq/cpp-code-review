/*
 * LLM 智能增强系统实现 (V3.0)
 * LLM Intelligence Enhancement System Implementation (V3.0)
 *
 * 功能概述 / Overview:
 * - 为代码问题提供 AI 驱动的智能修复建议
 * - 支持多种后端: 规则引擎、OpenAI GPT-4、Anthropic Claude
 * - 自动降级和错误处理
 * - 插件化设计，易于扩展
 *
 * - Provide AI-driven intelligent fix suggestions for code issues
 * - Support multiple backends: rule engine, OpenAI GPT-4, Anthropic Claude
 * - Automatic fallback and error handling
 * - Pluggable design, easy to extend
 *
 * 实现说明 / Implementation Notes:
 * - 每个规则都有专门的处理器，提供 4 级修复策略
 * - API 调用失败时自动回退到基于规则的建议
 * - 完整的代码示例和最佳实践指导
 *
 * - Each rule has a dedicated handler providing 4-level fix strategies
 * - Automatic fallback to rule-based suggestions on API failures
 * - Complete code examples and best practice guidance
 */

#include "llm/llm_enhancer.h"
#include "llm/api_client.h"
#include <sstream>
#include <iostream>

namespace cpp_review {

// ============================================================================
// RuleBasedProvider 实现 / RuleBasedProvider Implementation
// 基于规则的智能提供者 - 内置，无需外部 API
// ============================================================================

/**
 * 生成智能建议的主入口
 * Main entry point for generating intelligent suggestions
 *
 * 功能 / Functionality:
 * - 根据规则 ID 路由到专门的处理器
 * - 为每种问题类型提供定制化的修复策略
 * - 包含完整的代码示例和最佳实践
 *
 * - Route to specialized handlers based on rule ID
 * - Provide customized fix strategies for each issue type
 * - Include complete code examples and best practices
 *
 * @param issue 要增强的问题 / Issue to enhance
 * @param code_context 代码上下文（可选）/ Code context (optional)
 * @return 生成的智能建议 / Generated intelligent suggestion
 */
std::string RuleBasedProvider::generateSuggestion(const Issue& issue, const std::string& code_context) {
    // 根据规则 ID 路由到专门的处理器
    // Route to specialized handler based on rule ID
    if (issue.rule_id == "NULL-PTR-001") {
        return generateNullPointerSuggestion(issue);
    } else if (issue.rule_id == "MEMORY-LEAK-001") {
        return generateMemoryLeakSuggestion(issue);
    } else if (issue.rule_id == "BUFFER-OVERFLOW-001") {
        return generateBufferOverflowSuggestion(issue);
    } else if (issue.rule_id == "INTEGER-OVERFLOW-001") {
        return generateIntegerOverflowSuggestion(issue);
    } else if (issue.rule_id == "USE-AFTER-FREE-001") {
        return generateUseAfterFreeSuggestion(issue);
    } else if (issue.rule_id == "SMART-PTR-001") {
        return generateSmartPointerSuggestion(issue);
    } else if (issue.rule_id == "LOOP-COPY-001") {
        return generateLoopCopySuggestion(issue);
    }

    // 对于未知规则，返回通用建议
    // For unknown rules, return generic suggestion
    return generateGenericSuggestion(issue);
}

/**
 * 生成空指针解引用的修复建议
 * Generate fix suggestion for null pointer dereference
 *
 * 提供 4 级修复策略 / Provide 4-level fix strategies:
 * 1. 立即修复 - 添加空指针检查
 * 2. 推荐方案 - 使用智能指针
 * 3. 最佳实践 - 使用引用
 * 4. 设计模式 - 使用 Optional
 */
std::string RuleBasedProvider::generateNullPointerSuggestion(const Issue& issue) {
    std::stringstream ss;
    ss << "🤖 AI-Enhanced Fix Strategy:\n\n";
    ss << "1. **Immediate Fix** - Add null check:\n";
    ss << "   ```cpp\n";
    ss << "   if (ptr != nullptr) {\n";
    ss << "       // Use ptr safely\n";
    ss << "       *ptr = value;\n";
    ss << "   }\n";
    ss << "   ```\n\n";
    ss << "2. **Better Approach** - Use smart pointers:\n";
    ss << "   ```cpp\n";
    ss << "   auto ptr = std::make_unique<Type>();\n";
    ss << "   *ptr = value;  // Always safe\n";
    ss << "   ```\n\n";
    ss << "3. **Best Practice** - Use references when possible:\n";
    ss << "   ```cpp\n";
    ss << "   Type& ref = *ptr;  // Will throw if null\n";
    ss << "   ref = value;\n";
    ss << "   ```\n\n";
    ss << "4. **Design Pattern** - Use Optional for nullable values:\n";
    ss << "   ```cpp\n";
    ss << "   std::optional<Type> maybeValue;\n";
    ss << "   if (maybeValue.has_value()) {\n";
    ss << "       *maybeValue = value;\n";
    ss << "   }\n";
    ss << "   ```";

    return ss.str();
}

/**
 * 生成内存泄漏的修复建议
 * Generate fix suggestion for memory leak
 */
std::string RuleBasedProvider::generateMemoryLeakSuggestion(const Issue& issue) {
    std::stringstream ss;
    ss << "🤖 AI-Enhanced Fix Strategy:\n\n";
    ss << "1. **Quick Fix** - Add delete statement:\n";
    ss << "   ```cpp\n";
    ss << "   Type* ptr = new Type();\n";
    ss << "   // Use ptr...\n";
    ss << "   delete ptr;  // Don't forget!\n";
    ss << "   ptr = nullptr;  // Prevent dangling pointer\n";
    ss << "   ```\n\n";
    ss << "2. **Recommended** - Use std::unique_ptr:\n";
    ss << "   ```cpp\n";
    ss << "   auto ptr = std::make_unique<Type>();\n";
    ss << "   // Automatic cleanup, exception-safe\n";
    ss << "   ```\n\n";
    ss << "3. **For Shared Ownership** - Use std::shared_ptr:\n";
    ss << "   ```cpp\n";
    ss << "   auto ptr = std::make_shared<Type>();\n";
    ss << "   // Reference counted, multiple owners OK\n";
    ss << "   ```\n\n";
    ss << "4. **RAII Pattern** - Wrap resource in class:\n";
    ss << "   ```cpp\n";
    ss << "   class ResourceWrapper {\n";
    ss << "       Type* ptr_;\n";
    ss << "   public:\n";
    ss << "       ResourceWrapper() : ptr_(new Type()) {}\n";
    ss << "       ~ResourceWrapper() { delete ptr_; }\n";
    ss << "       // Delete copy, allow move\n";
    ss << "   };\n";
    ss << "   ```";

    return ss.str();
}

/**
 * 生成缓冲区溢出的修复建议
 * Generate fix suggestion for buffer overflow
 */
std::string RuleBasedProvider::generateBufferOverflowSuggestion(const Issue& issue) {
    std::stringstream ss;
    ss << "🤖 AI-Enhanced Fix Strategy:\n\n";
    ss << "1. **Immediate Fix** - Add bounds checking:\n";
    ss << "   ```cpp\n";
    ss << "   if (index >= 0 && index < array_size) {\n";
    ss << "       array[index] = value;\n";
    ss << "   } else {\n";
    ss << "       // Handle error\n";
    ss << "       throw std::out_of_range(\"Invalid index\");\n";
    ss << "   }\n";
    ss << "   ```\n\n";
    ss << "2. **Use std::vector with at()** - Automatic bounds checking:\n";
    ss << "   ```cpp\n";
    ss << "   std::vector<int> vec(size);\n";
    ss << "   try {\n";
    ss << "       vec.at(index) = value;  // Throws if out of bounds\n";
    ss << "   } catch (const std::out_of_range& e) {\n";
    ss << "       // Handle error\n";
    ss << "   }\n";
    ss << "   ```\n\n";
    ss << "3. **Use std::span (C++20)** - Safe array views:\n";
    ss << "   ```cpp\n";
    ss << "   void process(std::span<int> data) {\n";
    ss << "       for (size_t i = 0; i < data.size(); ++i) {\n";
    ss << "           data[i] = value;  // Size known\n";
    ss << "       }\n";
    ss << "   }\n";
    ss << "   ```\n\n";
    ss << "4. **Debug Mode** - Use assertions:\n";
    ss << "   ```cpp\n";
    ss << "   #include <cassert>\n";
    ss << "   assert(index >= 0 && index < size && \"Index out of bounds\");\n";
    ss << "   array[index] = value;\n";
    ss << "   ```";

    return ss.str();
}

/**
 * 生成整数溢出的修复建议
 * Generate fix suggestion for integer overflow
 */
std::string RuleBasedProvider::generateIntegerOverflowSuggestion(const Issue& issue) {
    std::stringstream ss;
    ss << "🤖 AI-Enhanced Fix Strategy:\n\n";
    ss << "1. **Use Larger Types** - Prevent overflow:\n";
    ss << "   ```cpp\n";
    ss << "   int8_t a = 100, b = 100;\n";
    ss << "   int32_t result = static_cast<int32_t>(a) + static_cast<int32_t>(b);\n";
    ss << "   ```\n\n";
    ss << "2. **Check Before Operation** - Detect potential overflow:\n";
    ss << "   ```cpp\n";
    ss << "   #include <limits>\n";
    ss << "   if (a > std::numeric_limits<int>::max() - b) {\n";
    ss << "       // Would overflow\n";
    ss << "       throw std::overflow_error(\"Addition overflow\");\n";
    ss << "   }\n";
    ss << "   int result = a + b;\n";
    ss << "   ```\n\n";
    ss << "3. **Use Compiler Builtins** - Hardware-assisted checking:\n";
    ss << "   ```cpp\n";
    ss << "   int result;\n";
    ss << "   if (__builtin_add_overflow(a, b, &result)) {\n";
    ss << "       // Overflow occurred\n";
    ss << "       std::cerr << \"Overflow detected!\" << std::endl;\n";
    ss << "   }\n";
    ss << "   ```\n\n";
    ss << "4. **Safe Integer Libraries** - Use checked types:\n";
    ss << "   ```cpp\n";
    ss << "   // Boost.SafeNumerics or similar\n";
    ss << "   safe<int> a = 100;\n";
    ss << "   safe<int> b = 100;\n";
    ss << "   safe<int> result = a + b;  // Throws on overflow\n";
    ss << "   ```";

    return ss.str();
}

/**
 * 生成 Use-After-Free 的修复建议
 * Generate fix suggestion for use-after-free
 */
std::string RuleBasedProvider::generateUseAfterFreeSuggestion(const Issue& issue) {
    std::stringstream ss;
    ss << "🤖 AI-Enhanced Fix Strategy:\n\n";
    ss << "1. **Immediate Fix** - Set to nullptr after delete:\n";
    ss << "   ```cpp\n";
    ss << "   delete ptr;\n";
    ss << "   ptr = nullptr;  // Prevent use-after-free\n";
    ss << "   \n";
    ss << "   if (ptr != nullptr) {\n";
    ss << "       *ptr = value;  // Won't execute\n";
    ss << "   }\n";
    ss << "   ```\n\n";
    ss << "2. **Best Practice** - Use RAII with smart pointers:\n";
    ss << "   ```cpp\n";
    ss << "   {\n";
    ss << "       auto ptr = std::make_unique<Type>();\n";
    ss << "       *ptr = value;  // Safe\n";
    ss << "   }  // Automatically deleted, can't use after\n";
    ss << "   ```\n\n";
    ss << "3. **Scope Management** - Limit pointer lifetime:\n";
    ss << "   ```cpp\n";
    ss << "   void processData() {\n";
    ss << "       Type* ptr = new Type();\n";
    ss << "       try {\n";
    ss << "           // Use ptr\n";
    ss << "       } catch (...) {\n";
    ss << "           delete ptr;\n";
    ss << "           throw;\n";
    ss << "       }\n";
    ss << "       delete ptr;\n";
    ss << "   }\n";
    ss << "   // ptr no longer accessible\n";
    ss << "   ```\n\n";
    ss << "4. **Memory Sanitizers** - Debug detection:\n";
    ss << "   ```bash\n";
    ss << "   # Compile with AddressSanitizer\n";
    ss << "   g++ -fsanitize=address -g code.cpp\n";
    ss << "   # Will catch use-after-free at runtime\n";
    ss << "   ```";

    return ss.str();
}

/**
 * 生成智能指针的修复建议
 * Generate fix suggestion for smart pointer usage
 */
std::string RuleBasedProvider::generateSmartPointerSuggestion(const Issue& issue) {
    std::stringstream ss;
    ss << "🤖 AI-Enhanced Refactoring Guide:\n\n";
    ss << "1. **std::unique_ptr** - For exclusive ownership:\n";
    ss << "   ```cpp\n";
    ss << "   // Before\n";
    ss << "   Widget* widget = new Widget();\n";
    ss << "   widget->doSomething();\n";
    ss << "   delete widget;\n";
    ss << "   \n";
    ss << "   // After\n";
    ss << "   auto widget = std::make_unique<Widget>();\n";
    ss << "   widget->doSomething();\n";
    ss << "   // Automatic cleanup\n";
    ss << "   ```\n\n";
    ss << "2. **std::shared_ptr** - For shared ownership:\n";
    ss << "   ```cpp\n";
    ss << "   auto resource = std::make_shared<Resource>();\n";
    ss << "   \n";
    ss << "   // Share with other owners\n";
    ss << "   auto copy = resource;  // Reference count++\n";
    ss << "   \n";
    ss << "   // Last owner cleans up automatically\n";
    ss << "   ```\n\n";
    ss << "3. **Passing Smart Pointers** - Best practices:\n";
    ss << "   ```cpp\n";
    ss << "   // By value: Transfer ownership\n";
    ss << "   void takeOwnership(std::unique_ptr<T> ptr);\n";
    ss << "   \n";
    ss << "   // By reference: Borrow temporarily\n";
    ss << "   void useTemporarily(const std::unique_ptr<T>& ptr);\n";
    ss << "   \n";
    ss << "   // Raw pointer: No ownership semantics\n";
    ss << "   void observe(T* ptr);\n";
    ss << "   ```\n\n";
    ss << "4. **Custom Deleters** - For special cleanup:\n";
    ss << "   ```cpp\n";
    ss << "   auto fileDeleter = [](FILE* f) { if (f) fclose(f); };\n";
    ss << "   std::unique_ptr<FILE, decltype(fileDeleter)> \n";
    ss << "       file(fopen(\"data.txt\", \"r\"), fileDeleter);\n";
    ss << "   ```";

    return ss.str();
}

/**
 * 生成循环拷贝优化的修复建议
 * Generate fix suggestion for loop copy optimization
 */
std::string RuleBasedProvider::generateLoopCopySuggestion(const Issue& issue) {
    std::stringstream ss;
    ss << "🤖 AI-Enhanced Performance Optimization:\n\n";
    ss << "1. **Use const reference** - Zero-copy access:\n";
    ss << "   ```cpp\n";
    ss << "   // Before: Copies each element\n";
    ss << "   for (std::string str : container) {\n";
    ss << "       process(str);  // Expensive copy!\n";
    ss << "   }\n";
    ss << "   \n";
    ss << "   // After: No copies\n";
    ss << "   for (const auto& str : container) {\n";
    ss << "       process(str);  // Just a reference\n";
    ss << "   }\n";
    ss << "   ```\n\n";
    ss << "2. **Non-const reference** - For modifications:\n";
    ss << "   ```cpp\n";
    ss << "   for (auto& element : container) {\n";
    ss << "       element.modify();  // Modify in place\n";
    ss << "   }\n";
    ss << "   ```\n\n";
    ss << "3. **Move semantics** - For consuming elements:\n";
    ss << "   ```cpp\n";
    ss << "   std::vector<std::string> results;\n";
    ss << "   for (auto&& str : container) {\n";
    ss << "       results.push_back(std::move(str));  // Move, not copy\n";
    ss << "   }\n";
    ss << "   ```\n\n";
    ss << "4. **Performance analysis** - Measure impact:\n";
    ss << "   ```cpp\n";
    ss << "   #include <chrono>\n";
    ss << "   auto start = std::chrono::high_resolution_clock::now();\n";
    ss << "   for (const auto& item : container) { /* ... */ }\n";
    ss << "   auto end = std::chrono::high_resolution_clock::now();\n";
    ss << "   auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);\n";
    ss << "   ```";

    return ss.str();
}

/**
 * 生成通用修复建议
 * Generate generic fix suggestion
 *
 * 用于未知规则类型的后备建议
 * Fallback suggestion for unknown rule types
 */
std::string RuleBasedProvider::generateGenericSuggestion(const Issue& issue) {
    std::stringstream ss;
    ss << "🤖 AI-Enhanced Analysis:\n\n";
    ss << "Based on the detected issue, consider these general best practices:\n\n";
    ss << "1. **Code Review**: Have a colleague review this code section\n";
    ss << "2. **Unit Tests**: Add tests to verify the fix works correctly\n";
    ss << "3. **Documentation**: Update code comments if behavior changes\n";
    ss << "4. **Static Analysis**: Run additional tools to catch related issues\n\n";
    ss << "For more specific guidance, consult:\n";
    ss << "- C++ Core Guidelines: https://isocpp.github.io/CppCoreGuidelines/\n";
    ss << "- Your team's coding standards\n";
    ss << "- Language-specific best practices";

    return ss.str();
}

// ============================================================================
// OpenAIProvider 实现 / OpenAIProvider Implementation
// OpenAI GPT-4 API 集成
// ============================================================================

/**
 * 使用 OpenAI GPT-4 生成修复建议
 * Generate fix suggestion using OpenAI GPT-4
 *
 * 功能 / Functionality:
 * - 构建上下文感知的提示词
 * - 调用 GPT-4 API
 * - 处理错误并降级到基于规则的建议
 *
 * - Build context-aware prompt
 * - Call GPT-4 API
 * - Handle errors and fallback to rule-based suggestions
 *
 * @param issue 要增强的问题 / Issue to enhance
 * @param code_context 代码上下文 / Code context
 * @return 生成的智能建议 / Generated intelligent suggestion
 */
std::string OpenAIProvider::generateSuggestion(const Issue& issue, const std::string& code_context) {
    if (!isAvailable()) {
        return "⚠️  OpenAI API not configured. Set OPENAI_API_KEY to enable AI suggestions.";
    }

    try {
        // 创建 OpenAI 客户端 / Create OpenAI client
        OpenAIClient client(api_key_);

        // 构建提示词 / Build prompt
        std::string prompt = buildPrompt(issue, code_context);

        // BUG FIX: 检查提示词长度 / Check prompt length
        // GPT-4 上下文限制约为 8K tokens，1 token ≈ 4 chars
        // GPT-4 context limit is around 8K tokens, 1 token ≈ 4 chars
        const size_t MAX_PROMPT_CHARS = 20000;  // ~5K tokens, 留余地给响应
        if (prompt.length() > MAX_PROMPT_CHARS) {
            std::cerr << "⚠️  Prompt too long (" << prompt.length()
                      << " chars), truncating...\n";
            prompt = prompt.substr(0, MAX_PROMPT_CHARS);
            prompt += "\n... [truncated]";
        }

        // 调用 API / Call API
        APIResponse response = client.complete(prompt, 800, 0.3);

        if (response.success) {
            std::stringstream ss;
            ss << "🤖 OpenAI GPT-4 Analysis:\n\n";
            ss << response.content;
            return ss.str();
        } else {
            std::stringstream ss;
            ss << "❌ OpenAI API Error: " << response.error_message << "\n";
            ss << "Status Code: " << response.status_code << "\n\n";
            ss << "Falling back to rule-based suggestion:\n";
            // 回退到基于规则的建议 / Fallback to rule-based suggestion
            RuleBasedProvider fallback;
            return fallback.generateSuggestion(issue, code_context);
        }

    } catch (const std::exception& e) {
        std::stringstream ss;
        ss << "❌ Exception calling OpenAI API: " << e.what() << "\n\n";
        ss << "Falling back to rule-based suggestion:\n";
        RuleBasedProvider fallback;
        return fallback.generateSuggestion(issue, code_context);
    }
}

/**
 * 检查 OpenAI 提供者是否可用
 * Check if OpenAI provider is available
 *
 * @return true 如果 API 密钥已配置 / true if API key is configured
 */
bool OpenAIProvider::isAvailable() const {
    // BUG FIX: 更严格的验证 / More strict validation
    // 检查 API 密钥格式 / Check API key format
    if (api_key_.empty() || api_key_ == "none") {
        return false;
    }

    // OpenAI 密钥通常以 "sk-" 开头 / OpenAI keys usually start with "sk-"
    if (api_key_.length() < 10) {
        std::cerr << "⚠️  OpenAI API key seems too short\n";
        return false;
    }

    return true;
}

/**
 * 构建发送给 OpenAI 的提示词
 * Build prompt to send to OpenAI
 *
 * @param issue 问题详情 / Issue details
 * @param code_context 代码上下文 / Code context
 * @return 构建的提示词 / Built prompt
 */
std::string OpenAIProvider::buildPrompt(const Issue& issue, const std::string& code_context) {
    std::stringstream ss;
    ss << "You are a C++ code review expert. Analyze this issue and provide a detailed fix:\n\n";
    ss << "Issue Type: " << issue.rule_id << "\n";
    ss << "Severity: ";

    // BUG FIX: 处理所有严重性级别 / Handle all severity levels
    switch (issue.severity) {
        case Severity::CRITICAL: ss << "CRITICAL"; break;
        case Severity::HIGH: ss << "HIGH"; break;
        case Severity::MEDIUM: ss << "MEDIUM"; break;
        case Severity::LOW: ss << "LOW"; break;
        case Severity::SUGGESTION: ss << "SUGGESTION"; break;
        default: ss << "UNKNOWN"; break;
    }

    ss << "\n";
    ss << "Description: " << issue.description << "\n\n";

    if (!code_context.empty()) {
        ss << "Code Context:\n" << code_context << "\n\n";
    }

    ss << "Provide:\n";
    ss << "1. Detailed explanation of the issue\n";
    ss << "2. Step-by-step fix instructions\n";
    ss << "3. Complete corrected code example\n";
    ss << "4. Best practices to prevent similar issues\n";

    return ss.str();
}

// ============================================================================
// AnthropicProvider 实现 / AnthropicProvider Implementation
// Anthropic Claude API 集成
// ============================================================================

/**
 * 使用 Anthropic Claude 生成修复建议
 * Generate fix suggestion using Anthropic Claude
 */
std::string AnthropicProvider::generateSuggestion(const Issue& issue, const std::string& code_context) {
    if (!isAvailable()) {
        return "⚠️  Anthropic API not configured. Set ANTHROPIC_API_KEY to enable AI suggestions.";
    }

    try {
        // 创建 Anthropic 客户端 / Create Anthropic client
        AnthropicClient client(api_key_);

        // 构建提示词 / Build prompt
        std::string prompt = buildPrompt(issue, code_context);

        // BUG FIX: 检查提示词长度 / Check prompt length
        // Claude 支持 200K tokens，但我们还是限制一下
        // Claude supports 200K tokens, but we still limit it
        const size_t MAX_PROMPT_CHARS = 50000;  // ~12K tokens
        if (prompt.length() > MAX_PROMPT_CHARS) {
            std::cerr << "⚠️  Prompt too long (" << prompt.length()
                      << " chars), truncating...\n";
            prompt = prompt.substr(0, MAX_PROMPT_CHARS);
            prompt += "\n... [truncated]";
        }

        // 调用 API / Call API
        APIResponse response = client.message(prompt, 800, 0.3);

        if (response.success) {
            std::stringstream ss;
            ss << "🤖 Anthropic Claude Analysis:\n\n";
            ss << response.content;
            return ss.str();
        } else {
            std::stringstream ss;
            ss << "❌ Anthropic API Error: " << response.error_message << "\n";
            ss << "Status Code: " << response.status_code << "\n\n";
            ss << "Falling back to rule-based suggestion:\n";
            // 回退到基于规则的建议 / Fallback to rule-based suggestion
            RuleBasedProvider fallback;
            return fallback.generateSuggestion(issue, code_context);
        }

    } catch (const std::exception& e) {
        std::stringstream ss;
        ss << "❌ Exception calling Anthropic API: " << e.what() << "\n\n";
        ss << "Falling back to rule-based suggestion:\n";
        RuleBasedProvider fallback;
        return fallback.generateSuggestion(issue, code_context);
    }
}

/**
 * 检查 Anthropic 提供者是否可用
 * Check if Anthropic provider is available
 */
bool AnthropicProvider::isAvailable() const {
    // BUG FIX: 更严格的验证 / More strict validation
    if (api_key_.empty() || api_key_ == "none") {
        return false;
    }

    // Anthropic 密钥通常以 "sk-ant-" 开头 / Anthropic keys usually start with "sk-ant-"
    if (api_key_.length() < 10) {
        std::cerr << "⚠️  Anthropic API key seems too short\n";
        return false;
    }

    return true;
}

/**
 * 构建发送给 Anthropic 的提示词
 * Build prompt to send to Anthropic
 */
std::string AnthropicProvider::buildPrompt(const Issue& issue, const std::string& code_context) {
    std::stringstream ss;
    ss << "You are a C++ code review expert. Analyze this issue and provide a detailed, actionable fix.\n\n";
    ss << "Issue Type: " << issue.rule_id << "\n";
    ss << "Severity: ";

    // BUG FIX: 处理所有严重性级别 / Handle all severity levels
    switch (issue.severity) {
        case Severity::CRITICAL: ss << "CRITICAL"; break;
        case Severity::HIGH: ss << "HIGH"; break;
        case Severity::MEDIUM: ss << "MEDIUM"; break;
        case Severity::LOW: ss << "LOW"; break;
        case Severity::SUGGESTION: ss << "SUGGESTION"; break;
        default: ss << "UNKNOWN"; break;
    }

    ss << "\n";
    ss << "Location: " << issue.file_path << ":" << issue.line << ":" << issue.column << "\n";
    ss << "Description: " << issue.description << "\n\n";

    if (!code_context.empty()) {
        ss << "Code Context:\n```cpp\n" << code_context << "\n```\n\n";
    }

    ss << "Please provide:\n";
    ss << "1. Root cause analysis\n";
    ss << "2. Immediate fix with code example\n";
    ss << "3. Long-term best practices\n";
    ss << "4. Potential pitfalls to avoid\n\n";
    ss << "Be concise and practical.";

    return ss.str();
}

// ============================================================================
// LLMEnhancer 实现 / LLMEnhancer Implementation
// LLM 增强器 - 协调 LLM 提供者
// ============================================================================

/**
 * 构造 LLM 增强器
 * Construct LLM enhancer
 *
 * @param provider LLM 提供者 / LLM provider
 */
LLMEnhancer::LLMEnhancer(std::shared_ptr<LLMProvider> provider)
    : provider_(std::move(provider)) {
    // BUG FIX: 验证提供者 / Validate provider
    if (!provider_) {
        std::cerr << "⚠️  LLMEnhancer: 提供者为空，增强功能将被禁用\n";
        std::cerr << "⚠️  LLMEnhancer: Provider is null, enhancement disabled\n";
    }
}

/**
 * 使用 LLM 生成的建议增强单个问题
 * Enhance a single issue with LLM-generated suggestions
 *
 * 功能 / Functionality:
 * - 调用 LLM 提供者生成智能建议
 * - 将 AI 建议追加到原有建议后面
 * - 处理异常情况
 *
 * - Call LLM provider to generate intelligent suggestions
 * - Append AI suggestions to existing suggestions
 * - Handle exceptional cases
 *
 * @param issue 原始问题 / Original issue
 * @param code_context 代码上下文 / Code context
 * @return 增强后的问题 / Enhanced issue
 */
Issue LLMEnhancer::enhanceIssue(const Issue& issue, const std::string& code_context) {
    Issue enhanced = issue;

    // BUG FIX: 检查提供者是否可用 / Check if provider is available
    if (!isEnabled()) {
        std::cerr << "ℹ️  LLM enhancement disabled or provider unavailable\n";
        return enhanced;  // 返回原始问题 / Return original issue
    }

    try {
        // 生成 AI 增强的建议 / Generate AI-enhanced suggestion
        std::string ai_suggestion = provider_->generateSuggestion(issue, code_context);

        // 追加到现有建议后 / Append to existing suggestion
        if (!enhanced.suggestion.empty()) {
            enhanced.suggestion += "\n\n" + std::string(70, '=') + "\n";
        }
        enhanced.suggestion += ai_suggestion;

    } catch (const std::exception& e) {
        std::cerr << "❌ Exception in enhanceIssue: " << e.what() << "\n";
        // 返回原始问题，不中断程序 / Return original issue, don't interrupt
    }

    return enhanced;
}

/**
 * 增强报告器中的所有问题
 * Enhance all issues in a reporter
 *
 * 注意 / Note:
 * - 当前为占位符实现
 * - 需要 Reporter 提供修改问题的接口
 *
 * - Currently a placeholder implementation
 * - Requires Reporter to provide interface for modifying issues
 *
 * @param reporter 包含问题的报告器 / Reporter containing issues
 */
void LLMEnhancer::enhanceAllIssues(Reporter& reporter) {
    if (!isEnabled()) {
        return;
    }

    // TODO: 需要 Reporter 提供修改问题的接口
    // TODO: Need Reporter to provide interface for modifying issues
    // 当前为占位符 / Currently a placeholder

    std::cerr << "ℹ️  enhanceAllIssues: 功能尚未实现 / not yet implemented\n";
}

// ============================================================================
// LLMProviderFactory 实现 / LLMProviderFactory Implementation
// 提供者工厂 - 创建不同类型的 LLM 提供者
// ============================================================================

/**
 * 创建指定类型的 LLM 提供者
 * Create LLM provider of specified type
 *
 * @param type 提供者类型 / Provider type
 * @param config 配置字符串（如 API 密钥）/ Configuration string (e.g., API key)
 * @return LLM 提供者的共享指针 / Shared pointer to LLM provider
 */
std::shared_ptr<LLMProvider> LLMProviderFactory::create(ProviderType type, const std::string& config) {
    switch (type) {
        case ProviderType::RULE_BASED:
            // 创建基于规则的提供者（内置，无需配置）
            // Create rule-based provider (built-in, no configuration needed)
            return std::make_shared<RuleBasedProvider>();

        case ProviderType::OPENAI:
            // 创建 OpenAI 提供者（需要 API 密钥）
            // Create OpenAI provider (requires API key)
            if (config.empty()) {
                std::cerr << "⚠️  OpenAI provider requires API key\n";
                return nullptr;
            }
            return std::make_shared<OpenAIProvider>(config);

        case ProviderType::ANTHROPIC:
            // 创建 Anthropic 提供者（需要 API 密钥）
            // Create Anthropic provider (requires API key)
            if (config.empty()) {
                std::cerr << "⚠️  Anthropic provider requires API key\n";
                return nullptr;
            }
            return std::make_shared<AnthropicProvider>(config);

        case ProviderType::NONE:
            // 禁用 LLM 功能 / Disable LLM functionality
            return nullptr;

        default:
            // BUG FIX: 显式处理未知类型 / Explicitly handle unknown types
            std::cerr << "❌ Unknown provider type: " << static_cast<int>(type) << "\n";
            return nullptr;
    }
}

} // namespace cpp_review
