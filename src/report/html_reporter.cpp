#include "report/html_reporter.h"
#include <sstream>
#include <map>
#include <algorithm>

namespace cpp_review {

std::string HTMLReporter::escapeHTML(const std::string& input) {
    std::string output;
    output.reserve(input.size());

    for (char c : input) {
        switch (c) {
            case '&':  output.append("&amp;");   break;
            case '<':  output.append("&lt;");    break;
            case '>':  output.append("&gt;");    break;
            case '"':  output.append("&quot;");  break;
            case '\'': output.append("&#39;");   break;
            default:   output.push_back(c);      break;
        }
    }

    return output;
}

std::string HTMLReporter::severityToHTML(Severity severity) {
    switch (severity) {
        case Severity::CRITICAL: return "CRITICAL";
        case Severity::HIGH: return "HIGH";
        case Severity::MEDIUM: return "MEDIUM";
        case Severity::LOW: return "LOW";
        case Severity::SUGGESTION: return "SUGGESTION";
    }
    return "UNKNOWN";
}

std::string HTMLReporter::severityToColor(Severity severity) {
    switch (severity) {
        case Severity::CRITICAL: return "#dc3545";  // Red
        case Severity::HIGH: return "#fd7e14";      // Orange
        case Severity::MEDIUM: return "#ffc107";    // Yellow
        case Severity::LOW: return "#17a2b8";       // Cyan
        case Severity::SUGGESTION: return "#28a745"; // Green
    }
    return "#6c757d"; // Gray
}

std::string HTMLReporter::getHTMLHeader() {
    return R"(<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>C++ 代码审查报告 | C++ Code Review Report</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, 'Microsoft YaHei', sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            padding: 20px;
            line-height: 1.6;
        }

        .container {
            max-width: 1200px;
            margin: 0 auto;
            background: white;
            border-radius: 15px;
            box-shadow: 0 20px 60px rgba(0,0,0,0.3);
            overflow: hidden;
        }

        .header {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 40px;
            text-align: center;
        }

        .header h1 {
            font-size: 2.5em;
            margin-bottom: 10px;
        }

        .header p {
            font-size: 1.1em;
            opacity: 0.9;
        }

        .summary {
            padding: 30px 40px;
            background: #f8f9fa;
            border-bottom: 3px solid #667eea;
        }

        .summary h2 {
            color: #333;
            margin-bottom: 20px;
        }

        .stats {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
            gap: 15px;
            margin-top: 20px;
        }

        .stat-card {
            background: white;
            padding: 20px;
            border-radius: 10px;
            text-align: center;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
            transition: transform 0.3s;
        }

        .stat-card:hover {
            transform: translateY(-5px);
        }

        .stat-number {
            font-size: 2.5em;
            font-weight: bold;
            margin-bottom: 5px;
        }

        .stat-label {
            color: #666;
            font-size: 0.9em;
        }

        .issues {
            padding: 40px;
        }

        .issues h2 {
            color: #333;
            margin-bottom: 30px;
            font-size: 2em;
        }

        .issue-card {
            background: white;
            border-left: 4px solid;
            margin-bottom: 20px;
            padding: 25px;
            border-radius: 8px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
            transition: all 0.3s;
        }

        .issue-card:hover {
            box-shadow: 0 5px 20px rgba(0,0,0,0.15);
            transform: translateX(5px);
        }

        .issue-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 15px;
            flex-wrap: wrap;
        }

        .issue-number {
            font-size: 1.3em;
            font-weight: bold;
            color: #333;
        }

        .severity-badge {
            padding: 5px 15px;
            border-radius: 20px;
            color: white;
            font-weight: bold;
            font-size: 0.9em;
        }

        .location {
            color: #666;
            margin: 10px 0;
            font-family: 'Courier New', monospace;
            background: #f8f9fa;
            padding: 10px;
            border-radius: 5px;
        }

        .rule-id {
            display: inline-block;
            background: #e9ecef;
            padding: 5px 10px;
            border-radius: 5px;
            font-family: 'Courier New', monospace;
            font-size: 0.9em;
            margin: 10px 0;
        }

        .description {
            color: #333;
            margin: 15px 0;
            line-height: 1.8;
        }

        .suggestion {
            background: #e7f3ff;
            border-left: 3px solid #0066cc;
            padding: 15px;
            margin: 15px 0;
            border-radius: 5px;
        }

        .suggestion-title {
            color: #0066cc;
            font-weight: bold;
            margin-bottom: 8px;
        }

        .code {
            background: #2d2d2d;
            color: #f8f8f2;
            padding: 15px;
            border-radius: 5px;
            overflow-x: auto;
            font-family: 'Courier New', monospace;
            margin: 10px 0;
        }

        .footer {
            background: #343a40;
            color: white;
            text-align: center;
            padding: 20px;
            font-size: 0.9em;
        }

        .filter-buttons {
            margin: 20px 0;
            display: flex;
            gap: 10px;
            flex-wrap: wrap;
        }

        .filter-btn {
            padding: 8px 16px;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            font-weight: bold;
            transition: all 0.3s;
        }

        .filter-btn:hover {
            transform: translateY(-2px);
            box-shadow: 0 4px 10px rgba(0,0,0,0.2);
        }

        .filter-btn.active {
            box-shadow: inset 0 3px 5px rgba(0,0,0,0.2);
        }

        @media (max-width: 768px) {
            .container {
                border-radius: 0;
            }

            .header h1 {
                font-size: 1.8em;
            }

            .stats {
                grid-template-columns: repeat(auto-fit, minmax(100px, 1fr));
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🛡️ C++ 智能代码审查报告</h1>
            <p>C++ Code Review Report - Powered by AST Analysis</p>
        </div>
)";
}

std::string HTMLReporter::getHTMLFooter() {
    return R"(
        <div class="footer">
            <p>🎯 由 C++ 智能代码审查 Agent 生成 | Made with ❤️ and ☕ by C++ Community</p>
            <p>Powered by Clang/LLVM AST Technology</p>
        </div>
    </div>

    <script>
        // Filter functionality
        function filterIssues(severity) {
            const issues = document.querySelectorAll('.issue-card');
            const buttons = document.querySelectorAll('.filter-btn');

            buttons.forEach(btn => btn.classList.remove('active'));
            event.target.classList.add('active');

            issues.forEach(issue => {
                if (severity === 'all' || issue.dataset.severity === severity) {
                    issue.style.display = 'block';
                } else {
                    issue.style.display = 'none';
                }
            });
        }

        // Set all button active by default
        document.addEventListener('DOMContentLoaded', () => {
            const allBtn = document.querySelector('[onclick*="all"]');
            if (allBtn) allBtn.classList.add('active');
        });
    </script>
</body>
</html>
)";
}

void HTMLReporter::generateHTMLReport(const Reporter& reporter, const std::string& output_file) {
    std::ofstream file(output_file);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open output file: " + output_file);
    }

    // Write HTML header
    file << getHTMLHeader();

    // Summary section
    const auto& issues = reporter.getIssues();
    std::map<Severity, size_t> severity_counts;

    for (const auto& issue : issues) {
        severity_counts[issue.severity]++;
    }

    file << "        <div class=\"summary\">\n";
    file << "            <h2>📈 问题统计摘要</h2>\n";
    file << "            <div class=\"stats\">\n";

    file << "                <div class=\"stat-card\">\n";
    file << "                    <div class=\"stat-number\" style=\"color: #667eea;\">" << issues.size() << "</div>\n";
    file << "                    <div class=\"stat-label\">总问题数</div>\n";
    file << "                </div>\n";

    file << "                <div class=\"stat-card\">\n";
    file << "                    <div class=\"stat-number\" style=\"color: #dc3545;\">" << severity_counts[Severity::CRITICAL] << "</div>\n";
    file << "                    <div class=\"stat-label\">严重 (Critical)</div>\n";
    file << "                </div>\n";

    file << "                <div class=\"stat-card\">\n";
    file << "                    <div class=\"stat-number\" style=\"color: #fd7e14;\">" << severity_counts[Severity::HIGH] << "</div>\n";
    file << "                    <div class=\"stat-label\">高 (High)</div>\n";
    file << "                </div>\n";

    file << "                <div class=\"stat-card\">\n";
    file << "                    <div class=\"stat-number\" style=\"color: #ffc107;\">" << severity_counts[Severity::MEDIUM] << "</div>\n";
    file << "                    <div class=\"stat-label\">中 (Medium)</div>\n";
    file << "                </div>\n";

    file << "                <div class=\"stat-card\">\n";
    file << "                    <div class=\"stat-number\" style=\"color: #17a2b8;\">" << severity_counts[Severity::LOW] << "</div>\n";
    file << "                    <div class=\"stat-label\">低 (Low)</div>\n";
    file << "                </div>\n";

    file << "                <div class=\"stat-card\">\n";
    file << "                    <div class=\"stat-number\" style=\"color: #28a745;\">" << severity_counts[Severity::SUGGESTION] << "</div>\n";
    file << "                    <div class=\"stat-label\">建议 (Suggestion)</div>\n";
    file << "                </div>\n";

    file << "            </div>\n";
    file << "        </div>\n";

    // Issues section
    file << "        <div class=\"issues\">\n";
    file << "            <h2>🔍 详细问题列表</h2>\n";

    // Filter buttons
    file << "            <div class=\"filter-buttons\">\n";
    file << "                <button class=\"filter-btn\" onclick=\"filterIssues('all')\" style=\"background: #667eea; color: white;\">全部</button>\n";
    file << "                <button class=\"filter-btn\" onclick=\"filterIssues('CRITICAL')\" style=\"background: #dc3545; color: white;\">严重</button>\n";
    file << "                <button class=\"filter-btn\" onclick=\"filterIssues('HIGH')\" style=\"background: #fd7e14; color: white;\">高</button>\n";
    file << "                <button class=\"filter-btn\" onclick=\"filterIssues('MEDIUM')\" style=\"background: #ffc107; color: #333;\">中</button>\n";
    file << "                <button class=\"filter-btn\" onclick=\"filterIssues('LOW')\" style=\"background: #17a2b8; color: white;\">低</button>\n";
    file << "                <button class=\"filter-btn\" onclick=\"filterIssues('SUGGESTION')\" style=\"background: #28a745; color: white;\">建议</button>\n";
    file << "            </div>\n";

    // Issue cards
    for (size_t i = 0; i < issues.size(); ++i) {
        const auto& issue = issues[i];
        std::string color = severityToColor(issue.severity);
        std::string severityStr = severityToHTML(issue.severity);

        file << "            <div class=\"issue-card\" style=\"border-left-color: " << color << ";\" data-severity=\"" << severityStr << "\">\n";
        file << "                <div class=\"issue-header\">\n";
        file << "                    <div class=\"issue-number\">问题 #" << (i + 1) << "</div>\n";
        file << "                    <div class=\"severity-badge\" style=\"background-color: " << color << ";\">" << severityStr << "</div>\n";
        file << "                </div>\n";

        file << "                <div class=\"location\">📍 " << escapeHTML(issue.file_path) << ":" << issue.line << ":" << issue.column << "</div>\n";
        file << "                <div class=\"rule-id\">🏷️ " << escapeHTML(issue.rule_id) << "</div>\n";
        file << "                <div class=\"description\">📝 " << escapeHTML(issue.description) << "</div>\n";

        if (!issue.code_snippet.empty()) {
            file << "                <div class=\"code\">" << escapeHTML(issue.code_snippet) << "</div>\n";
        }

        if (!issue.suggestion.empty()) {
            file << "                <div class=\"suggestion\">\n";
            file << "                    <div class=\"suggestion-title\">💡 修复建议:</div>\n";
            file << "                    <div>" << escapeHTML(issue.suggestion) << "</div>\n";
            file << "                </div>\n";
        }

        file << "            </div>\n";
    }

    if (issues.empty()) {
        file << "            <div style=\"text-align: center; padding: 60px; color: #28a745;\">\n";
        file << "                <h2>✅ 太棒了!</h2>\n";
        file << "                <p style=\"font-size: 1.2em; margin-top: 20px;\">没有发现任何问题,您的代码质量很高!</p>\n";
        file << "            </div>\n";
    }

    file << "        </div>\n";

    // Write HTML footer
    file << getHTMLFooter();

    file.close();
}

} // namespace cpp_review
