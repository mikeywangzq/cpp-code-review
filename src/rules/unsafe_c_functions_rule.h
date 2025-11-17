#pragma once

#include "rules/rule.h"
#include <clang/AST/Expr.h>
#include <string>
#include <unordered_map>

namespace cpp_review {

struct UnsafeFunctionInfo {
    std::string safe_alternative;
    std::string reason;
};

class UnsafeCFunctionsVisitor : public RuleVisitor {
public:
    using RuleVisitor::RuleVisitor;

    bool VisitCallExpr(clang::CallExpr* call);

private:
    static const std::unordered_map<std::string, UnsafeFunctionInfo> unsafe_functions_;
};

class UnsafeCFunctionsRule : public Rule {
public:
    std::string getRuleId() const override { return "UNSAFE-C-FUNC-001"; }
    std::string getRuleName() const override { return "Unsafe C Functions"; }
    std::string getDescription() const override {
        return "Detects usage of unsafe C-style functions that can lead to buffer overflows and security vulnerabilities";
    }

    void check(clang::ASTContext* context, Reporter& reporter) override;
};

} // namespace cpp_review
