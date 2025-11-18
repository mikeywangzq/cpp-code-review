# CI/CD 集成指南

本文档详细介绍如何在各种 CI/CD 平台上集成 C++ Code Review Agent。

## 目录

- [GitHub Actions](#github-actions)
- [GitLab CI/CD](#gitlab-cicd)
- [CircleCI](#circleci)
- [Jenkins](#jenkins)
- [Azure Pipelines](#azure-pipelines)
- [Travis CI](#travis-ci)
- [最佳实践](#最佳实践)

---

## GitHub Actions

### 基础配置

创建 `.github/workflows/code-review.yml`:

```yaml
name: C++ Code Review

on:
  push:
    branches: [ main, master, develop ]
    paths:
      - '**.cpp'
      - '**.h'
      - '**.cc'
      - '**.hpp'
  pull_request:
    branches: [ main, master, develop ]

jobs:
  code-review:
    name: 代码审查
    runs-on: ubuntu-latest

    steps:
    - name: 检出代码
      uses: actions/checkout@v3
      with:
        fetch-depth: 0  # 完整历史以支持增量分析

    - name: 安装依赖
      run: |
        sudo apt-get update
        sudo apt-get install -y \
          build-essential cmake \
          llvm-14 llvm-14-dev \
          clang-14 libclang-14-dev \
          libcurl4-openssl-dev

    - name: 构建 cpp-agent
      run: |
        mkdir -p build && cd build
        cmake .. -DENABLE_CURL=ON -DCMAKE_BUILD_TYPE=Release
        make -j$(nproc)

    - name: 运行代码审查 (PR)
      if: github.event_name == 'pull_request'
      env:
        OPENAI_API_KEY: ${{ secrets.OPENAI_API_KEY }}
        ANTHROPIC_API_KEY: ${{ secrets.ANTHROPIC_API_KEY }}
      run: |
        ./build/cpp-agent --pr \
          --pr-comment=pr-review.md \
          --enable-taint-analysis \
          --std=c++17

    - name: 运行代码审查 (Push)
      if: github.event_name == 'push'
      run: |
        ./build/cpp-agent --incremental \
          --enable-taint-analysis

    - name: 上传审查报告
      if: github.event_name == 'pull_request'
      uses: actions/upload-artifact@v3
      with:
        name: pr-review
        path: pr-review.md

    - name: 发布 PR 评论
      if: github.event_name == 'pull_request' && hashFiles('pr-review.md') != ''
      uses: actions/github-script@v6
      with:
        github-token: ${{ secrets.GITHUB_TOKEN }}
        script: |
          const fs = require('fs');
          const comment = fs.readFileSync('pr-review.md', 'utf8');

          // 查找现有评论
          const { data: comments } = await github.rest.issues.listComments({
            owner: context.repo.owner,
            repo: context.repo.repo,
            issue_number: context.issue.number,
          });

          const botComment = comments.find(comment =>
            comment.user.type === 'Bot' &&
            comment.body.includes('C++ Code Review')
          );

          if (botComment) {
            // 更新现有评论
            await github.rest.issues.updateComment({
              owner: context.repo.owner,
              repo: context.repo.repo,
              comment_id: botComment.id,
              body: comment
            });
          } else {
            // 创建新评论
            await github.rest.issues.createComment({
              owner: context.repo.owner,
              repo: context.repo.repo,
              issue_number: context.issue.number,
              body: comment
            });
          }

    - name: 检查关键问题
      if: github.event_name == 'pull_request'
      run: |
        if [ -f "pr-review.md" ]; then
          if grep -q "🚨 CRITICAL\|⚠️  HIGH" pr-review.md; then
            echo "::error::发现严重或高优先级问题！"
            exit 1
          fi
        fi
```

### 高级功能

#### 自动修复

```yaml
  auto-fix:
    name: 自动修复
    runs-on: ubuntu-latest
    if: github.event_name == 'pull_request' &&
        github.event.pull_request.head.repo.full_name == github.repository

    steps:
    - uses: actions/checkout@v3
      with:
        ref: ${{ github.head_ref }}
        token: ${{ secrets.GITHUB_TOKEN }}

    - name: 安装依赖
      run: |
        sudo apt-get update
        sudo apt-get install -y build-essential cmake \
          llvm-14 clang-14 libclang-14-dev

    - name: 构建
      run: |
        mkdir -p build && cd build
        cmake .. && make -j$(nproc)

    - name: 运行自动修复
      run: |
        ./build/cpp-agent --incremental --auto-fix --backup

    - name: 提交修复
      run: |
        git config user.name "github-actions[bot]"
        git config user.email "github-actions[bot]@users.noreply.github.com"
        git add -A
        git diff --staged --quiet || \
          git commit -m "🤖 自动修复代码问题 [skip ci]"
        git push
```

#### 定时全量扫描

```yaml
  scheduled-scan:
    name: 定时全量扫描
    runs-on: ubuntu-latest
    if: github.event_name == 'schedule'

    steps:
    - uses: actions/checkout@v3

    - name: 完整扫描
      run: |
        ./build.sh
        ./build/cpp-agent scan . \
          --html-output=full-report.html \
          --enable-taint-analysis

    - name: 上传报告
      uses: actions/upload-artifact@v3
      with:
        name: full-scan-report
        path: full-report.html
        retention-days: 90

on:
  schedule:
    - cron: '0 0 * * 0'  # 每周日午夜
```

### 密钥配置

在仓库 Settings → Secrets and variables → Actions 中添加:

- `OPENAI_API_KEY`: OpenAI API 密钥
- `ANTHROPIC_API_KEY`: Anthropic API 密钥

---

## GitLab CI/CD

### 基础配置

创建 `.gitlab-ci.yml`:

```yaml
# GitLab CI/CD 配置
stages:
  - build
  - review
  - report
  - security

variables:
  CPP_STANDARD: "c++17"
  ENABLE_TAINT_ANALYSIS: "true"
  ENABLE_AI_SUGGESTIONS: "false"

# 构建阶段
build:
  stage: build
  image: ubuntu:22.04
  before_script:
    - apt-get update -qq
    - apt-get install -y build-essential cmake \
        llvm-14 llvm-14-dev clang-14 libclang-14-dev \
        libcurl4-openssl-dev
  script:
    - mkdir -p build && cd build
    - cmake .. -DENABLE_CURL=ON -DCMAKE_BUILD_TYPE=Release
    - make -j$(nproc)
  artifacts:
    paths:
      - build/cpp-agent
    expire_in: 1 hour
  cache:
    paths:
      - build/

# Merge Request 审查
review:mr:
  stage: review
  image: ubuntu:22.04
  dependencies:
    - build
  only:
    - merge_requests
  before_script:
    - apt-get update -qq && apt-get install -y git
  script:
    - |
      TAINT_FLAG=""
      if [ "$ENABLE_TAINT_ANALYSIS" = "true" ]; then
        TAINT_FLAG="--enable-taint-analysis"
      fi

      AI_FLAGS=""
      if [ "$ENABLE_AI_SUGGESTIONS" = "true" ]; then
        AI_FLAGS="--enable-ai --ai-provider=${AI_PROVIDER:-rule-based}"
        if [ -n "$OPENAI_API_KEY" ]; then
          AI_FLAGS="$AI_FLAGS --openai-key=$OPENAI_API_KEY"
        fi
      fi

      ./build/cpp-agent --pr \
        --pr-comment=mr-review.md \
        --std=$CPP_STANDARD \
        $TAINT_FLAG $AI_FLAGS

    - |
      if [ -f mr-review.md ]; then
        echo "📊 代码审查报告:"
        cat mr-review.md
      fi
  artifacts:
    reports:
      codequality: gl-code-quality-report.json
    paths:
      - mr-review.md
    expire_in: 30 days
  allow_failure: true

# Push 审查
review:push:
  stage: review
  image: ubuntu:22.04
  dependencies:
    - build
  only:
    - branches
  except:
    - merge_requests
  script:
    - ./build/cpp-agent --incremental --std=$CPP_STANDARD
  allow_failure: true

# 安全扫描
security:scan:
  stage: security
  image: ubuntu:22.04
  dependencies:
    - build
  only:
    - merge_requests
    - main
    - master
    - develop
  script:
    - |
      ./build/cpp-agent scan . \
        --std=$CPP_STANDARD \
        --enable-taint-analysis \
        --severity-filter=CRITICAL,HIGH \
        > security-report.txt || true

      if grep -q "CRITICAL\|HIGH" security-report.txt; then
        echo "⚠️  发现安全漏洞！"
        cat security-report.txt
        exit 1
      else
        echo "✅ 未发现安全漏洞"
      fi
  artifacts:
    paths:
      - security-report.txt
    expire_in: 30 days

# HTML 报告
report:html:
  stage: report
  image: ubuntu:22.04
  dependencies:
    - build
  only:
    - schedules
    - main
    - master
  script:
    - |
      ./build/cpp-agent scan . \
        --std=$CPP_STANDARD \
        --html-output=code-review-report.html
  artifacts:
    paths:
      - code-review-report.html
    expire_in: 90 days
```

### Code Quality 报告

生成 GitLab Code Quality 格式的报告:

```yaml
report:codequality:
  stage: report
  dependencies:
    - review:mr
  script:
    - |
      # 转换为 Code Quality 格式
      python3 << 'EOF'
      import json

      issues = []
      # 解析 mr-review.md 并生成 Code Quality JSON
      # ...

      with open('gl-code-quality-report.json', 'w') as f:
          json.dump(issues, f)
      EOF
  artifacts:
    reports:
      codequality: gl-code-quality-report.json
```

### 变量配置

在 GitLab 项目 Settings → CI/CD → Variables 中添加:

- `OPENAI_API_KEY` (protected, masked)
- `ANTHROPIC_API_KEY` (protected, masked)
- `AI_PROVIDER`: `openai` 或 `anthropic`

---

## CircleCI

### 基础配置

创建 `.circleci/config.yml`:

```yaml
version: 2.1

# 执行器定义
executors:
  cpp-executor:
    docker:
      - image: ubuntu:22.04
    resource_class: medium
    working_directory: ~/project

# 作业定义
jobs:
  build:
    executor: cpp-executor
    steps:
      - checkout

      - restore_cache:
          keys:
            - v1-deps-{{ checksum "CMakeLists.txt" }}
            - v1-deps-

      - run:
          name: 安装依赖
          command: |
            apt-get update -qq
            apt-get install -y build-essential cmake git \
              llvm-14 llvm-14-dev clang-14 libclang-14-dev \
              libcurl4-openssl-dev

      - run:
          name: 构建项目
          command: |
            mkdir -p build && cd build
            cmake .. -DENABLE_CURL=ON -DCMAKE_BUILD_TYPE=Release
            make -j$(nproc)

      - save_cache:
          paths:
            - build
          key: v1-deps-{{ checksum "CMakeLists.txt" }}

      - persist_to_workspace:
          root: .
          paths:
            - build/cpp-agent

  code-review:
    executor: cpp-executor
    steps:
      - checkout

      - attach_workspace:
          at: .

      - run:
          name: 代码审查
          command: |
            MODE="--incremental"
            if [ -n "$CIRCLE_PULL_REQUEST" ]; then
              MODE="--pr --pr-comment=pr-review.md"
            fi

            TAINT_FLAG=""
            if [ "${ENABLE_TAINT_ANALYSIS:-true}" = "true" ]; then
              TAINT_FLAG="--enable-taint-analysis"
            fi

            ./build/cpp-agent $MODE \
              --std=${CPP_STANDARD:-c++17} \
              $TAINT_FLAG

      - run:
          name: 检查严重问题
          command: |
            if [ -f pr-review.md ]; then
              if grep -q "CRITICAL\|HIGH" pr-review.md; then
                echo "⚠️  发现严重问题！"
                cat pr-review.md
                exit 1
              fi
            fi

      - store_artifacts:
          path: pr-review.md
          destination: reviews/

  security-scan:
    executor: cpp-executor
    steps:
      - checkout

      - attach_workspace:
          at: .

      - run:
          name: 安全扫描
          command: |
            ./build/cpp-agent scan . \
              --std=${CPP_STANDARD:-c++17} \
              --enable-taint-analysis \
              --severity-filter=CRITICAL,HIGH \
              > security-report.txt || true

      - run:
          name: 分析安全报告
          command: |
            if [ -f security-report.txt ]; then
              echo "📊 安全扫描报告:"
              cat security-report.txt

              if grep -q "CRITICAL\|HIGH" security-report.txt; then
                echo "⚠️  发现安全漏洞！"
                exit 1
              else
                echo "✅ 未发现安全漏洞"
              fi
            fi

      - store_artifacts:
          path: security-report.txt
          destination: security/

  generate-report:
    executor: cpp-executor
    steps:
      - checkout

      - attach_workspace:
          at: .

      - run:
          name: 生成 HTML 报告
          command: |
            ./build/cpp-agent scan . \
              --std=${CPP_STANDARD:-c++17} \
              --html-output=code-review-report.html

      - store_artifacts:
          path: code-review-report.html
          destination: reports/

# 工作流定义
workflows:
  version: 2
  main:
    jobs:
      - build
      - code-review:
          requires:
            - build
      - security-scan:
          requires:
            - build
          filters:
            branches:
              only:
                - main
                - master
                - develop
      - generate-report:
          requires:
            - build
          filters:
            branches:
              only:
                - main
                - master

  # 定时全量扫描
  scheduled:
    triggers:
      - schedule:
          cron: "0 0 * * 0"  # 每周日午夜
          filters:
            branches:
              only:
                - main
                - master
    jobs:
      - build
      - security-scan:
          requires:
            - build
      - generate-report:
          requires:
            - build
```

### 环境变量

在 CircleCI 项目 Settings → Environment Variables 中添加:

- `OPENAI_API_KEY`
- `ANTHROPIC_API_KEY`
- `CPP_STANDARD` (可选)
- `ENABLE_TAINT_ANALYSIS` (可选)

---

## Jenkins

### Pipeline 配置

创建 `Jenkinsfile`:

```groovy
pipeline {
    agent {
        docker {
            image 'ubuntu:22.04'
            args '-u root:root'
        }
    }

    environment {
        CPP_STANDARD = 'c++17'
        ENABLE_TAINT = 'true'
        ENABLE_AI = credentials('enable-ai-suggestions') ?: 'false'
        OPENAI_API_KEY = credentials('openai-api-key') ?: ''
        ANTHROPIC_API_KEY = credentials('anthropic-api-key') ?: ''
    }

    options {
        buildDiscarder(logRotator(numToKeepStr: '10'))
        timestamps()
        timeout(time: 30, unit: 'MINUTES')
    }

    stages {
        stage('准备环境') {
            steps {
                sh '''
                    apt-get update -qq
                    apt-get install -y build-essential cmake git \
                        llvm-14 llvm-14-dev clang-14 libclang-14-dev \
                        libcurl4-openssl-dev
                '''
            }
        }

        stage('构建') {
            steps {
                sh '''
                    mkdir -p build && cd build
                    cmake .. -DENABLE_CURL=ON -DCMAKE_BUILD_TYPE=Release
                    make -j$(nproc)
                '''
            }
        }

        stage('代码审查') {
            when {
                changeRequest()
            }
            steps {
                script {
                    def taintFlag = env.ENABLE_TAINT == 'true' ? '--enable-taint-analysis' : ''
                    def aiFlags = ''

                    if (env.ENABLE_AI == 'true') {
                        aiFlags = '--enable-ai'
                        if (env.OPENAI_API_KEY) {
                            aiFlags += " --openai-key=${env.OPENAI_API_KEY}"
                        }
                        if (env.ANTHROPIC_API_KEY) {
                            aiFlags += " --anthropic-key=${env.ANTHROPIC_API_KEY}"
                        }
                    }

                    sh """
                        ./build/cpp-agent --pr \
                            --pr-comment=pr-review.md \
                            --std=${env.CPP_STANDARD} \
                            ${taintFlag} ${aiFlags}
                    """
                }

                script {
                    if (fileExists('pr-review.md')) {
                        def review = readFile('pr-review.md')
                        echo "代码审查结果:\n${review}"
                    }
                }
            }
        }

        stage('增量分析') {
            when {
                not {
                    changeRequest()
                }
            }
            steps {
                sh '''
                    TAINT_FLAG=""
                    if [ "$ENABLE_TAINT" = "true" ]; then
                        TAINT_FLAG="--enable-taint-analysis"
                    fi

                    ./build/cpp-agent --incremental \
                        --std=$CPP_STANDARD \
                        $TAINT_FLAG
                '''
            }
        }

        stage('安全扫描') {
            steps {
                sh '''
                    ./build/cpp-agent scan . \
                        --std=$CPP_STANDARD \
                        --enable-taint-analysis \
                        --severity-filter=CRITICAL,HIGH \
                        > security-report.txt || true
                '''

                script {
                    if (fileExists('security-report.txt')) {
                        def report = readFile('security-report.txt')
                        if (report.contains('CRITICAL') || report.contains('HIGH')) {
                            unstable(message: '发现安全问题')
                        }
                    }
                }
            }
        }

        stage('生成报告') {
            steps {
                sh '''
                    ./build/cpp-agent scan . \
                        --std=$CPP_STANDARD \
                        --html-output=code-review-report.html
                '''

                publishHTML([
                    allowMissing: false,
                    alwaysLinkToLastBuild: true,
                    keepAll: true,
                    reportDir: '.',
                    reportFiles: 'code-review-report.html',
                    reportName: 'Code Review Report'
                ])
            }
        }
    }

    post {
        always {
            archiveArtifacts artifacts: '*.txt,*.md,*.html',
                           allowEmptyArchive: true
            cleanWs()
        }

        success {
            echo '✅ 代码审查完成，未发现严重问题'
        }

        unstable {
            echo '⚠️  代码审查发现安全问题，请检查报告'
        }

        failure {
            echo '❌ 代码审查失败'
        }
    }
}
```

### 凭据配置

在 Jenkins → Manage Jenkins → Manage Credentials 中添加:

- `openai-api-key`: Secret text
- `anthropic-api-key`: Secret text
- `enable-ai-suggestions`: String parameter (true/false)

---

## 最佳实践

### 1. 增量分析优先

```bash
# 只分析变更的文件，提升速度
cpp-agent --incremental
```

### 2. 严重性过滤

```bash
# 只检查关键问题
cpp-agent --severity-filter=CRITICAL,HIGH
```

### 3. 缓存构建产物

```yaml
# GitHub Actions
- uses: actions/cache@v3
  with:
    path: build
    key: ${{ runner.os }}-build-${{ hashFiles('CMakeLists.txt') }}
```

### 4. 并行执行

```yaml
# GitLab CI
build:
  parallel: 4
  script:
    - make -j$(nproc)
```

### 5. 失败策略

```yaml
# 允许非致命错误
allow_failure: true

# 检查致命错误后失败
if grep -q "CRITICAL" report.txt; then exit 1; fi
```

### 6. 定时全量扫描

```yaml
# 每周运行一次完整扫描
on:
  schedule:
    - cron: '0 0 * * 0'
```

### 7. API 密钥管理

```bash
# 使用密钥管理服务
export OPENAI_API_KEY=$(vault read -field=value secret/openai/api-key)
```

### 8. 报告归档

```yaml
# 保存报告 90 天
artifacts:
  expire_in: 90 days
```

---

## 故障排查

### 构建失败

```bash
# 检查依赖
llvm-config --version
clang --version

# 清理重建
rm -rf build && mkdir build && cd build
cmake .. && make
```

### API 调用失败

```bash
# 检查密钥
echo $OPENAI_API_KEY | head -c 10

# 测试连接
curl https://api.openai.com/v1/models \
  -H "Authorization: Bearer $OPENAI_API_KEY"
```

### PR 评论失败

```bash
# 检查权限
# GitHub: 需要 write 权限
# GitLab: 需要 Maintainer 角色
```

---

## 总结

选择合适的 CI/CD 平台配置：

- **GitHub Actions**: 最佳 GitHub 集成
- **GitLab CI/CD**: 强大的 Code Quality 支持
- **CircleCI**: 灵活的工作流
- **Jenkins**: 企业级自定义

所有平台都支持增量分析、安全扫描和 HTML 报告生成。
