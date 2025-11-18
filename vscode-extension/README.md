# C++ Code Review Agent - VS Code Extension

🛡️ **专业的 C++ 代码审查扩展，实时检测代码问题并提供智能修复建议**

## ✨ 功能特性

### 🔍 实时代码审查
- **保存时自动检测**: 每次保存文件时自动运行代码审查
- **即时反馈**: 在编辑器中直接显示问题和建议
- **严重性分级**: CRITICAL、HIGH、MEDIUM、LOW、SUGGESTION 五级分类

### 🤖 AI 智能修复
- **基于规则的建议**: 内置智能修复策略
- **OpenAI GPT-4**: 集成 GPT-4 提供高级建议
- **Anthropic Claude**: 支持 Claude API
- **自动修复**: 一键应用修复建议

### 🔐 高级安全分析
- **数据流污点分析**: 检测 SQL 注入、命令注入等安全漏洞
- **Use-After-Free 检测**: 内存安全分析
- **缓冲区溢出检测**: 数组越界检查
- **整数溢出检测**: 算术运算安全检查

### 📊 丰富的检测规则
- 空指针解引用
- 未初始化变量
- 内存泄漏
- 赋值/比较混淆
- 不安全的 C 函数
- 循环拷贝优化
- 智能指针建议

## 🚀 快速开始

### 安装

1. 在 VS Code 扩展市场搜索 "C++ Code Review Agent"
2. 点击安装
3. 配置 `cpp-agent` 可执行文件路径

### 配置 cpp-agent

```bash
# 克隆项目
git clone https://github.com/yourusername/cpp-code-review.git
cd cpp-code-review

# 构建
./build.sh

# 将可执行文件路径添加到 PATH 或在扩展设置中配置
export PATH=$PATH:$(pwd)/build
```

### 基本使用

1. **打开 C++ 文件**: 扩展会自动激活
2. **保存文件**: 自动运行代码审查
3. **查看问题**: 在"问题"面板中查看检测到的问题
4. **应用修复**: 使用 `Ctrl+Shift+F` (Mac: `Cmd+Shift+F`) 自动修复

## ⚙️ 配置选项

### 基础配置

```json
{
  "cpp-review.executablePath": "cpp-agent",
  "cpp-review.cppStandard": "c++17",
  "cpp-review.enableAutoReview": true
}
```

### AI 配置

```json
{
  "cpp-review.enableAISuggestions": true,
  "cpp-review.aiProvider": "openai",
  "cpp-review.openaiApiKey": "sk-your-api-key-here"
}
```

或使用 Anthropic Claude:

```json
{
  "cpp-review.enableAISuggestions": true,
  "cpp-review.aiProvider": "anthropic",
  "cpp-review.anthropicApiKey": "sk-ant-your-api-key-here"
}
```

### 高级配置

```json
{
  "cpp-review.enableTaintAnalysis": true,
  "cpp-review.severityFilter": ["CRITICAL", "HIGH", "MEDIUM"]
}
```

## 📖 命令

| 命令 | 快捷键 | 说明 |
|------|--------|------|
| `C++ Review: Analyze Workspace` | - | 分析整个工作区 |
| `C++ Review: Analyze Current File` | `Ctrl+Shift+R` | 分析当前文件 |
| `C++ Review: Auto Fix Issues` | `Ctrl+Shift+F` | 自动修复问题 |
| `C++ Review: Clear Diagnostics` | - | 清除所有诊断 |

## 📸 截图

### 实时问题检测
![实时检测](images/screenshot-1.png)

### AI 智能建议
![AI建议](images/screenshot-2.png)

### 自动修复
![自动修复](images/screenshot-3.png)

## 🔧 配置详解

### cpp-review.executablePath
- **类型**: `string`
- **默认值**: `"cpp-agent"`
- **说明**: cpp-agent 可执行文件的完整路径或命令名

### cpp-review.cppStandard
- **类型**: `string`
- **可选值**: `c++11`, `c++14`, `c++17`, `c++20`
- **默认值**: `"c++17"`
- **说明**: C++ 标准版本

### cpp-review.enableAutoReview
- **类型**: `boolean`
- **默认值**: `true`
- **说明**: 保存文件时自动运行代码审查

### cpp-review.enableAISuggestions
- **类型**: `boolean`
- **默认值**: `false`
- **说明**: 启用 AI 智能修复建议

### cpp-review.aiProvider
- **类型**: `string`
- **可选值**: `rule-based`, `openai`, `anthropic`
- **默认值**: `"rule-based"`
- **说明**: AI 提供者选择

### cpp-review.enableTaintAnalysis
- **类型**: `boolean`
- **默认值**: `true`
- **说明**: 启用数据流污点分析

### cpp-review.severityFilter
- **类型**: `array`
- **默认值**: `["CRITICAL", "HIGH", "MEDIUM", "LOW", "SUGGESTION"]`
- **说明**: 显示的严重性级别过滤器

## 🤝 贡献

欢迎贡献！请查看 [贡献指南](CONTRIBUTING.md)

## 📄 许可证

MIT License - 详见 [LICENSE](LICENSE)

## 🙏 致谢

- 基于 [Clang/LLVM](https://clang.llvm.org/) AST 技术
- 感谢所有贡献者和支持者

## 📧 联系

- **问题反馈**: [GitHub Issues](https://github.com/yourusername/cpp-code-review/issues)
- **功能建议**: [GitHub Discussions](https://github.com/yourusername/cpp-code-review/discussions)

---

**Made with ❤️ for the C++ Community**
