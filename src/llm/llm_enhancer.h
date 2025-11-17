/*
 * LLM 智能增强系统 (V2.0 核心功能)
 * 为代码问题提供 AI 驱动的智能修复建议
 *
 * 特性:
 * - 基于规则的内置智能系统 (无需外部 API)
 * - 7 种专门的规则处理器,提供详细的修复策略
 * - 支持未来扩展: OpenAI GPT, Anthropic Claude 等
 * - 插件化设计,易于添加新的 LLM 提供者
 */

#pragma once

#include "report/reporter.h"
#include <string>
#include <memory>

namespace cpp_review {

/**
 * LLM 提供者接口
 * 定义所有 LLM 提供者必须实现的接口
 * 支持多种后端: 规则引擎、OpenAI、Anthropic 等
 */
class LLMProvider {
public:
    virtual ~LLMProvider() = default;

    /**
     * 为问题生成增强的修复建议
     * @param issue 要增强的问题
     * @param code_context 代码上下文 (可选)
     * @return 生成的智能建议文本
     */
    virtual std::string generateSuggestion(const Issue& issue, const std::string& code_context) = 0;

    /**
     * 检查提供者是否可用/已配置
     * @return 如果提供者可用返回 true
     */
    virtual bool isAvailable() const = 0;

    /**
     * 获取提供者名称
     * @return 提供者的显示名称
     */
    virtual std::string getName() const = 0;
};

/**
 * 基于规则的智能提供者 (内置,无需外部 API)
 *
 * 特性:
 * - 7 种专门的规则处理器
 * - 每个处理器提供 4 级修复策略
 * - 包含完整的代码示例
 * - 零成本,零延迟,完全离线
 *
 * 规则覆盖:
 * 1. NULL-PTR-001: 空指针解引用
 * 2. MEMORY-LEAK-001: 内存泄漏
 * 3. BUFFER-OVERFLOW-001: 缓冲区溢出
 * 4. INTEGER-OVERFLOW-001: 整数溢出
 * 5. USE-AFTER-FREE-001: Use-After-Free
 * 6. SMART-PTR-001: 智能指针建议
 * 7. LOOP-COPY-001: 循环拷贝优化
 */
class RuleBasedProvider : public LLMProvider {
public:
    std::string generateSuggestion(const Issue& issue, const std::string& code_context) override;
    bool isAvailable() const override { return true; }
    std::string getName() const override { return "Rule-Based Intelligence"; }

private:
    // 7 个专门的建议生成器
    std::string generateNullPointerSuggestion(const Issue& issue);
    std::string generateMemoryLeakSuggestion(const Issue& issue);
    std::string generateBufferOverflowSuggestion(const Issue& issue);
    std::string generateIntegerOverflowSuggestion(const Issue& issue);
    std::string generateUseAfterFreeSuggestion(const Issue& issue);
    std::string generateSmartPointerSuggestion(const Issue& issue);
    std::string generateLoopCopySuggestion(const Issue& issue);
    std::string generateGenericSuggestion(const Issue& issue);
};

/**
 * OpenAI 提供者 (未来功能)
 * 占位符,用于未来集成 OpenAI GPT API
 *
 * 计划功能:
 * - HTTP 客户端集成
 * - GPT-4 API 调用
 * - 上下文感知的代码建议
 * - 自动代码修复
 */
class OpenAIProvider : public LLMProvider {
public:
    explicit OpenAIProvider(const std::string& api_key) : api_key_(api_key) {}

    std::string generateSuggestion(const Issue& issue, const std::string& code_context) override;
    bool isAvailable() const override;
    std::string getName() const override { return "OpenAI GPT"; }

private:
    std::string api_key_;
    std::string buildPrompt(const Issue& issue, const std::string& code_context);
};

/**
 * LLM 增强器
 * 协调 LLM 提供者,为问题添加智能建议
 */
class LLMEnhancer {
public:
    explicit LLMEnhancer(std::shared_ptr<LLMProvider> provider);

    /**
     * 使用 LLM 生成的建议增强单个问题
     * @param issue 原始问题
     * @param code_context 可选的代码上下文
     * @return 增强后的问题 (包含 AI 建议)
     */
    Issue enhanceIssue(const Issue& issue, const std::string& code_context = "");

    /**
     * 增强报告器中的所有问题
     * @param reporter 包含问题的报告器
     */
    void enhanceAllIssues(Reporter& reporter);

    /**
     * 检查增强功能是否启用
     * @return 如果提供者可用返回 true
     */
    bool isEnabled() const { return provider_ && provider_->isAvailable(); }

    /**
     * 获取当前提供者名称
     * @return 提供者名称
     */
    std::string getProviderName() const {
        return provider_ ? provider_->getName() : "None";
    }

private:
    std::shared_ptr<LLMProvider> provider_;  // LLM 提供者实例
};

/**
 * LLM 提供者工厂
 * 创建不同类型的 LLM 提供者实例
 */
class LLMProviderFactory {
public:
    /**
     * 提供者类型枚举
     */
    enum class ProviderType {
        RULE_BASED,  // 基于规则的内置系统
        OPENAI,      // OpenAI GPT (未来)
        NONE         // 禁用 LLM 功能
    };

    /**
     * 创建 LLM 提供者
     * @param type 提供者类型
     * @param config 配置字符串 (如 API 密钥)
     * @return LLM 提供者的共享指针
     */
    static std::shared_ptr<LLMProvider> create(ProviderType type, const std::string& config = "");
};

} // namespace cpp_review
