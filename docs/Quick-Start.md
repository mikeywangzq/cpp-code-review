# 快速开始指南

## 📦 安装

### 前置要求

- **操作系统**: Linux, macOS, WSL
- **编译器**: GCC 7+ 或 Clang 6+
- **CMake**: 3.15+
- **LLVM**: 10.0+
- **可选**: libcurl (用于 API 集成)

### Ubuntu/Debian

```bash
sudo apt-get update && sudo apt-get install -y \
    build-essential cmake \
    llvm-14 llvm-14-dev \
    clang-14 libclang-14-dev \
    libcurl4-openssl-dev
```

### macOS

```bash
brew install llvm cmake curl
export LLVM_DIR=/usr/local/opt/llvm/lib/cmake/llvm
export Clang_DIR=/usr/local/opt/llvm/lib/cmake/clang
```

### Fedora/RHEL

```bash
sudo dnf install -y gcc-c++ cmake llvm-devel clang-devel libcurl-devel
```

## 🔨 构建

### 自动构建（推荐）

```bash
git clone https://github.com/yourusername/cpp-code-review.git
cd cpp-code-review
./build.sh
```

### 手动构建

```bash
mkdir build && cd build

# 基础构建
cmake ..
make -j$(nproc)

# 启用 API 支持
cmake .. -DENABLE_CURL=ON
make -j$(nproc)
```

### 验证安装

```bash
./build/cpp-agent --version
./build/cpp-agent --help
```

## 🎯 基本使用

### 1. 扫描单个文件

```bash
./build/cpp-agent scan main.cpp
```

### 2. 扫描整个项目

```bash
./build/cpp-agent scan ./src
```

### 3. 指定 C++ 标准

```bash
./build/cpp-agent scan main.cpp --std=c++20
```

### 4. 生成 HTML 报告

```bash
./build/cpp-agent scan main.cpp --html-output=report.html
```

## 🚀 V3.0 新功能

### 自动修复

```bash
# 自动修复所有问题
./build/cpp-agent scan main.cpp --auto-fix

# 交互式修复
./build/cpp-agent scan main.cpp --auto-fix --interactive

# 只修复严重问题
./build/cpp-agent scan main.cpp --auto-fix --severity-filter=CRITICAL,HIGH
```

### AI 智能建议

```bash
# 配置 API 密钥
export OPENAI_API_KEY="sk-your-api-key"

# 使用 OpenAI GPT-4
./build/cpp-agent scan main.cpp --enable-ai --ai-provider=openai

# 使用 Anthropic Claude
export ANTHROPIC_API_KEY="sk-ant-your-api-key"
./build/cpp-agent scan main.cpp --enable-ai --ai-provider=anthropic
```

### 数据流污点分析

```bash
# 检测安全漏洞
./build/cpp-agent scan src/ --enable-taint-analysis
```

### Git 增量分析

```bash
# 只分析工作区变更
./build/cpp-agent --incremental

# 分析相对于 main 分支的变更
./build/cpp-agent --branch=main

# PR 审查模式
./build/cpp-agent --pr --pr-comment=review.md
```

## 💻 VS Code 集成

### 安装扩展

```bash
cd vscode-extension
npm install
npm run compile
code --install-extension cpp-code-review-1.0.0.vsix
```

### 配置扩展

在 VS Code 设置中：

```json
{
  "cpp-review.executablePath": "/path/to/cpp-agent",
  "cpp-review.cppStandard": "c++17",
  "cpp-review.enableAutoReview": true
}
```

### 使用快捷键

- `Ctrl+Shift+R` - 分析当前文件
- `Ctrl+Shift+F` - 自动修复

## 🔄 CI/CD 集成

### GitHub Actions

创建 `.github/workflows/code-review.yml`:

```yaml
name: Code Review
on: [push, pull_request]
jobs:
  review:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - run: ./build.sh
      - run: ./build/cpp-agent --pr --pr-comment=review.md
```

### GitLab CI

创建 `.gitlab-ci.yml`:

```yaml
code-review:
  script:
    - ./build.sh
    - ./build/cpp-agent --pr --pr-comment=mr-review.md
```

## 📚 下一步

- 📖 阅读[完整功能文档](V3.0-Features.md)
- 🔧 配置[自定义规则](Configuration.md)
- 🎨 安装 [VS Code 扩展](../vscode-extension/README.md)
- 🤝 查看[贡献指南](Contributing.md)

## 💡 常见问题

### Q: 如何禁用某个规则？

```bash
./build/cpp-agent scan main.cpp --disable-rule=NULL-PTR-001
```

### Q: 如何只显示严重问题？

```bash
./build/cpp-agent scan main.cpp --severity-filter=CRITICAL,HIGH
```

### Q: API 密钥如何保密？

使用环境变量或配置文件，不要硬编码在代码中。

### Q: 如何获取帮助？

```bash
./build/cpp-agent --help
```

或访问 [GitHub Issues](https://github.com/yourusername/cpp-code-review/issues)
