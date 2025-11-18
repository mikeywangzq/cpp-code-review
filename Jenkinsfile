// Jenkins Pipeline for C++ Code Review
// 支持多分支和 PR 构建

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
                    apt-get install -y build-essential cmake \
                        llvm-14 llvm-14-dev clang-14 libclang-14-dev \
                        libcurl4-openssl-dev git
                '''
            }
        }

        stage('构建') {
            steps {
                sh '''
                    mkdir -p build
                    cd build
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

                // 发布 PR 评论
                script {
                    if (fileExists('pr-review.md')) {
                        def review = readFile('pr-review.md')
                        echo "代码审查结果:\n${review}"

                        // 如果集成了 GitHub/GitLab，可以发布评论
                        // 这里仅打印到控制台
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

                // 检查是否有严重问题
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
            // 归档报告
            archiveArtifacts artifacts: '*.txt,*.md,*.html', allowEmptyArchive: true

            // 清理工作区
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
