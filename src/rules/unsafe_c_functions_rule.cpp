#include "rules/unsafe_c_functions_rule.h"
#include <clang/AST/Decl.h>

namespace cpp_review {

const std::unordered_map<std::string, UnsafeFunctionInfo>
UnsafeCFunctionsVisitor::unsafe_functions_ = {
    {"strcpy", {
        "std::string, strncpy, or strcpy_s",
        "No bounds checking - can cause buffer overflow"
    }},
    {"strcat", {
        "std::string, strncat, or strcat_s",
        "No bounds checking - can cause buffer overflow"
    }},
    {"sprintf", {
        "snprintf or std::stringstream",
        "No bounds checking - can cause buffer overflow"
    }},
    {"gets", {
        "std::getline, fgets, or std::cin",
        "No bounds checking - extremely dangerous, removed in C11"
    }},
    {"scanf", {
        "std::cin with width specifiers",
        "Can cause buffer overflow without width specifiers"
    }},
    {"vsprintf", {
        "vsnprintf",
        "No bounds checking - can cause buffer overflow"
    }},
    {"strncpy", {
        "std::string or ensure null-termination",
        "May not null-terminate the result"
    }},
    {"strncat", {
        "std::string",
        "Complex bounds checking required"
    }}
};

bool UnsafeCFunctionsVisitor::VisitCallExpr(clang::CallExpr* call) {
    if (!call) return true;

    // Get the called function
    const clang::FunctionDecl* func = call->getDirectCallee();
    if (!func) return true;

    std::string funcName = func->getNameAsString();

    // Check if this is an unsafe function
    auto it = unsafe_functions_.find(funcName);
    if (it != unsafe_functions_.end()) {
        Issue issue;
        issue.file_path = getFileName(call->getBeginLoc());
        issue.line = getLine(call->getBeginLoc());
        issue.column = getColumn(call->getBeginLoc());
        issue.severity = Severity::CRITICAL;
        issue.rule_id = "UNSAFE-C-FUNC-001";

        const UnsafeFunctionInfo& info = it->second;
        issue.description = "Use of unsafe C function '" + funcName + "': " + info.reason;
        issue.suggestion = "Replace '" + funcName + "' with " + info.safe_alternative +
                         ". In modern C++, prefer using std::string for string operations to avoid manual memory management";

        // Get code snippet
        clang::SourceRange range = call->getSourceRange();
        issue.code_snippet = getSourceText(range);

        reporter_.addIssue(issue);
    }

    return true;
}

void UnsafeCFunctionsRule::check(clang::ASTContext* context, Reporter& reporter) {
    UnsafeCFunctionsVisitor visitor(context, reporter);
    visitor.TraverseDecl(context->getTranslationUnitDecl());
}

} // namespace cpp_review
