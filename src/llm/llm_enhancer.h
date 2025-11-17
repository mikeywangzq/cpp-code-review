#pragma once

#include "report/reporter.h"
#include <string>
#include <memory>

namespace cpp_review {

// LLM Provider interface for different backends
class LLMProvider {
public:
    virtual ~LLMProvider() = default;

    // Generate enhanced suggestion for an issue
    virtual std::string generateSuggestion(const Issue& issue, const std::string& code_context) = 0;

    // Check if the provider is available/configured
    virtual bool isAvailable() const = 0;

    // Get provider name
    virtual std::string getName() const = 0;
};

// Built-in rule-based intelligent suggestion system (no external LLM needed)
class RuleBasedProvider : public LLMProvider {
public:
    std::string generateSuggestion(const Issue& issue, const std::string& code_context) override;
    bool isAvailable() const override { return true; }
    std::string getName() const override { return "Rule-Based Intelligence"; }

private:
    std::string generateNullPointerSuggestion(const Issue& issue);
    std::string generateMemoryLeakSuggestion(const Issue& issue);
    std::string generateBufferOverflowSuggestion(const Issue& issue);
    std::string generateIntegerOverflowSuggestion(const Issue& issue);
    std::string generateUseAfterFreeSuggestion(const Issue& issue);
    std::string generateSmartPointerSuggestion(const Issue& issue);
    std::string generateLoopCopySuggestion(const Issue& issue);
    std::string generateGenericSuggestion(const Issue& issue);
};

// Placeholder for future OpenAI integration
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

// LLM Enhancer orchestrates LLM-powered suggestions
class LLMEnhancer {
public:
    explicit LLMEnhancer(std::shared_ptr<LLMProvider> provider);

    // Enhance an issue with LLM-generated suggestions
    Issue enhanceIssue(const Issue& issue, const std::string& code_context = "");

    // Enhance all issues in a reporter
    void enhanceAllIssues(Reporter& reporter);

    // Check if enhancement is enabled
    bool isEnabled() const { return provider_ && provider_->isAvailable(); }

    // Get current provider name
    std::string getProviderName() const {
        return provider_ ? provider_->getName() : "None";
    }

private:
    std::shared_ptr<LLMProvider> provider_;
};

// Factory for creating LLM providers
class LLMProviderFactory {
public:
    enum class ProviderType {
        RULE_BASED,
        OPENAI,
        NONE
    };

    static std::shared_ptr<LLMProvider> create(ProviderType type, const std::string& config = "");
};

} // namespace cpp_review
