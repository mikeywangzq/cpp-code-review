# C++ Code Review Agent 文档中心

欢迎来到 C++ Code Review Agent 的文档中心！这里提供了完整的文档和指南，帮助您快速上手和深入使用。

## 📚 文档结构

```
docs/
├── README.md                    # 本文件 - 文档导航
├── Quick-Start.md               # 快速开始指南
├── V3.0-Features.md             # V3.0 完整功能文档
└── CI-CD-Integration.md         # CI/CD 集成指南
```

## 🚀 快速导航

### 新手入门

如果您是第一次使用，请按以下顺序阅读：

1. **[快速开始指南](Quick-Start.md)** ⭐ 推荐首读
   - 安装和构建（5分钟）
   - 基本使用示例
   - 常见问题解答

2. **[V3.0 功能文档](V3.0-Features.md)**
   - 深入了解所有功能
   - API 参考和示例
   - 架构设计说明

### 进阶使用

当您熟悉基础功能后，可以探索这些高级主题：

3. **[CI/CD 集成指南](CI-CD-Integration.md)**
   - 4大平台完整配置
   - 自动化工作流
   - 最佳实践

4. **[VS Code 扩展](../vscode-extension/README.md)**
   - IDE 集成
   - 实时代码审查
   - 快捷键参考

## 📖 文档概览

### Quick-Start.md

**适合人群**: 所有用户，特别是新手

**主要内容**:
- ✅ 多平台安装指南（Ubuntu, macOS, Fedora）
- ✅ 构建步骤（自动和手动）
- ✅ 基本使用命令
- ✅ V3.0 新功能快速上手
- ✅ 常见问题解答

**预计阅读时间**: 10-15 分钟

### V3.0-Features.md

**适合人群**: 需要深入了解功能的用户和开发者

**主要内容**:
- 📦 **自动代码修复** - 完整的 AutoFixer API 和使用示例
- 🤖 **LLM API 集成** - OpenAI 和 Anthropic 集成指南
- 💻 **VS Code 扩展** - 扩展功能和配置详解
- 🔄 **CI/CD 集成** - 多平台集成概览
- 🔬 **污点分析** - 安全漏洞检测原理和用法
- 🏗️ **架构设计** - 系统架构和模块依赖
- 📚 **API 参考** - 完整的 C++ API 文档

**预计阅读时间**: 30-45 分钟

### CI-CD-Integration.md

**适合人群**: DevOps 工程师和需要 CI/CD 集成的团队

**主要内容**:
- 🐙 **GitHub Actions** - 完整工作流配置，含自动修复
- 🦊 **GitLab CI/CD** - Pipeline 配置，Code Quality 报告
- 🔵 **CircleCI** - 工作流和缓存优化
- 🔧 **Jenkins** - Pipeline 脚本和凭据管理
- 💡 **最佳实践** - 增量分析、缓存、定时扫描
- 🔧 **故障排查** - 常见问题和解决方案

**预计阅读时间**: 45-60 分钟

## 🎯 按场景查找文档

### 我想...

**...快速开始使用**
→ [Quick-Start.md](Quick-Start.md) - 基本使用

**...启用自动修复**
→ [V3.0-Features.md](V3.0-Features.md#1-自动代码修复) - 自动修复

**...集成 OpenAI/Claude**
→ [V3.0-Features.md](V3.0-Features.md#2-llm-api-集成) - LLM API

**...在 VS Code 中使用**
→ [VS Code 扩展](../vscode-extension/README.md) - IDE 集成

**...在 GitHub Actions 中集成**
→ [CI-CD-Integration.md](CI-CD-Integration.md#github-actions) - GitHub CI

**...检测安全漏洞**
→ [V3.0-Features.md](V3.0-Features.md#5-数据流污点分析) - 污点分析

**...了解 API 接口**
→ [V3.0-Features.md](V3.0-Features.md#api-参考) - API 文档

## 📊 功能概览

| 功能 | 文档链接 | 难度 | 推荐度 |
|------|----------|------|--------|
| 基础扫描 | [Quick-Start](Quick-Start.md#基本使用) | ⭐ | ⭐⭐⭐⭐⭐ |
| 自动修复 | [V3.0-Features](V3.0-Features.md#1-自动代码修复) | ⭐⭐ | ⭐⭐⭐⭐⭐ |
| AI 建议 | [V3.0-Features](V3.0-Features.md#2-llm-api-集成) | ⭐⭐ | ⭐⭐⭐⭐ |
| VS Code 集成 | [VS Code 扩展](../vscode-extension/README.md) | ⭐⭐ | ⭐⭐⭐⭐⭐ |
| GitHub Actions | [CI-CD](CI-CD-Integration.md#github-actions) | ⭐⭐⭐ | ⭐⭐⭐⭐⭐ |
| GitLab CI | [CI-CD](CI-CD-Integration.md#gitlab-cicd) | ⭐⭐⭐ | ⭐⭐⭐⭐ |
| 污点分析 | [V3.0-Features](V3.0-Features.md#5-数据流污点分析) | ⭐⭐⭐⭐ | ⭐⭐⭐⭐ |

## 💡 使用技巧

### 快速查找

使用浏览器的搜索功能（Ctrl+F 或 Cmd+F）在文档中快速查找关键词：

- 搜索 "OpenAI" 查找 AI 集成相关内容
- 搜索 "auto-fix" 查找自动修复相关内容
- 搜索 "GitHub" 查找 GitHub Actions 配置
- 搜索 "污点" 查找安全分析相关内容

### 代码示例

所有文档都包含丰富的代码示例，可以直接复制使用。示例代码块都有语法高亮和注释说明。

### 外部链接

文档中的外部链接都会在新标签页打开，方便您参考官方文档。

## 🔄 文档更新

文档会随着项目版本更新。每次发布新版本时，都会更新相应的文档。

**当前文档版本**: V3.0.0
**最后更新时间**: 2025-11-18

## 🤝 贡献文档

如果您发现文档中的错误或有改进建议，欢迎：

1. 提交 Issue 报告问题
2. 提交 PR 改进文档
3. 在 Discussions 中讨论

## 📞 获取帮助

如果您在使用过程中遇到问题：

1. 📖 **先查阅文档** - 大多数问题都能在文档中找到答案
2. 🔍 **搜索 Issues** - 看看是否有人遇到过类似问题
3. 💬 **提问** - 在 GitHub Discussions 提问
4. 🐛 **报告 Bug** - 在 GitHub Issues 报告问题

## 🌟 推荐阅读路径

### 路径 1: 快速上手（适合新手）

```
快速开始 → 基本使用 → VS Code 集成 → GitHub Actions
  (10分钟)   (20分钟)    (15分钟)      (20分钟)
```

### 路径 2: 深入学习（适合开发者）

```
快速开始 → V3.0 完整文档 → API 参考 → 架构设计
  (10分钟)     (45分钟)      (20分钟)   (15分钟)
```

### 路径 3: CI/CD 集成（适合 DevOps）

```
快速开始 → CI/CD 集成 → 最佳实践 → 故障排查
  (10分钟)    (30分钟)    (15分钟)    (10分钟)
```

## 📚 额外资源

- 🏠 [项目主页](../README.md)
- 💻 [示例代码](../examples/)
- 🔧 [配置示例](../config/)
- 🎨 [VS Code 扩展](../vscode-extension/)

---

<div align="center">

**祝您使用愉快！** 🎉

如果觉得文档有帮助，请给项目一个 ⭐ Star

</div>
