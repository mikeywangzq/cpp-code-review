#!/bin/bash

###############################################################################
# PR 自动审查脚本
# 用于 CI/CD 环境中的自动代码审查
#
# 使用方法:
#   1. GitHub Actions: 将此脚本添加到 .github/workflows/pr-review.yml
#   2. GitLab CI: 将此脚本添加到 .gitlab-ci.yml
#   3. 本地测试: ./scripts/pr-review.sh
###############################################################################

set -e  # 遇到错误立即退出

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo -e "${BLUE}  🤖 C++ Code Review Agent - PR 自动审查${NC}"
echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo ""

# 检查是否在 Git 仓库中
if ! git rev-parse --git-dir > /dev/null 2>&1; then
    echo -e "${RED}✗ Error: Not a Git repository${NC}"
    exit 1
fi

# 检查 cpp-agent 是否存在
if ! command -v cpp-agent &> /dev/null; then
    if [ -f "./build/cpp-agent" ]; then
        CPP_AGENT="./build/cpp-agent"
    else
        echo -e "${RED}✗ Error: cpp-agent not found${NC}"
        echo -e "  Please build the project first: ./build.sh"
        exit 1
    fi
else
    CPP_AGENT="cpp-agent"
fi

echo -e "${GREEN}✓ Found cpp-agent: $CPP_AGENT${NC}"

# 检测 PR 环境
if [ -n "$GITHUB_ACTIONS" ] && [ "$GITHUB_EVENT_NAME" = "pull_request" ]; then
    echo -e "${BLUE}📍 GitHub Actions PR environment detected${NC}"
    echo -e "   PR #$GITHUB_PR_NUMBER: $GITHUB_BASE_REF <- $GITHUB_HEAD_REF"
    PR_MODE="--pr"
elif [ -n "$GITLAB_CI" ] && [ -n "$CI_MERGE_REQUEST_IID" ]; then
    echo -e "${BLUE}📍 GitLab CI MR environment detected${NC}"
    echo -e "   MR !$CI_MERGE_REQUEST_IID: $CI_MERGE_REQUEST_TARGET_BRANCH_NAME <- $CI_MERGE_REQUEST_SOURCE_BRANCH_NAME"
    PR_MODE="--pr"
else
    echo -e "${YELLOW}⚠  Not in PR environment, using default branch comparison${NC}"
    PR_MODE="--branch=main"
fi

echo ""
echo -e "${BLUE}🔍 Running incremental analysis...${NC}"
echo ""

# 运行分析并生成 PR 评论
COMMENT_FILE="pr-review-comment.md"

# 运行 cpp-agent
if $CPP_AGENT $PR_MODE --pr-comment=$COMMENT_FILE --std=c++17; then
    EXIT_CODE=$?
else
    EXIT_CODE=$?
fi

echo ""
echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"

if [ -f "$COMMENT_FILE" ]; then
    echo -e "${GREEN}✓ PR review comment generated: $COMMENT_FILE${NC}"
    echo ""

    # 在 CI 环境中,尝试发布评论
    if [ -n "$GITHUB_ACTIONS" ]; then
        echo -e "${BLUE}💬 Posting comment to GitHub PR...${NC}"
        if command -v gh &> /dev/null; then
            gh pr comment $GITHUB_PR_NUMBER --body-file $COMMENT_FILE || true
        else
            echo -e "${YELLOW}⚠  GitHub CLI (gh) not found, skipping comment posting${NC}"
            echo -e "   Review comment content:"
            cat $COMMENT_FILE
        fi
    elif [ -n "$GITLAB_CI" ]; then
        echo -e "${YELLOW}⚠  GitLab API comment posting not implemented yet${NC}"
        echo -e "   Review comment content:"
        cat $COMMENT_FILE
    else
        echo -e "${YELLOW}📋 Review comment preview:${NC}"
        cat $COMMENT_FILE
    fi
else
    echo -e "${YELLOW}⚠  No review comment generated${NC}"
fi

echo ""
echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"

# 返回分析的退出码
if [ $EXIT_CODE -eq 2 ]; then
    echo -e "${RED}✗ Critical issues found${NC}"
    exit 1
elif [ $EXIT_CODE -eq 0 ]; then
    echo -e "${GREEN}✓ Analysis completed successfully${NC}"
    exit 0
else
    echo -e "${RED}✗ Analysis failed${NC}"
    exit $EXIT_CODE
fi
