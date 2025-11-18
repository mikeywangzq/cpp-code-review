/**
 * C++ Code Review Agent - VS Code 扩展
 * 提供实时代码审查、智能修复建议和自动修复功能
 */

import * as vscode from 'vscode';
import * as cp from 'child_process';
import * as path from 'path';

// 诊断集合
let diagnosticCollection: vscode.DiagnosticCollection;

// 扩展激活
export function activate(context: vscode.ExtensionContext) {
    console.log('C++ Code Review Agent 已激活');

    // 创建诊断集合
    diagnosticCollection = vscode.languages.createDiagnosticCollection('cpp-review');
    context.subscriptions.push(diagnosticCollection);

    // 注册命令
    context.subscriptions.push(
        vscode.commands.registerCommand('cpp-review.analyze', analyzeWorkspace)
    );
    context.subscriptions.push(
        vscode.commands.registerCommand('cpp-review.analyzeFile', analyzeCurrentFile)
    );
    context.subscriptions.push(
        vscode.commands.registerCommand('cpp-review.autoFix', autoFixIssues)
    );
    context.subscriptions.push(
        vscode.commands.registerCommand('cpp-review.clearDiagnostics', clearDiagnostics)
    );

    // 文件保存时自动审查
    context.subscriptions.push(
        vscode.workspace.onDidSaveTextDocument(onDocumentSave)
    );

    // 文件打开时自动审查
    context.subscriptions.push(
        vscode.window.onDidChangeActiveTextEditor(onEditorChange)
    );

    // 状态栏项
    const statusBarItem = vscode.window.createStatusBarItem(
        vscode.StatusBarAlignment.Left,
        100
    );
    statusBarItem.text = "$(shield) C++ Review";
    statusBarItem.tooltip = "点击运行代码审查";
    statusBarItem.command = "cpp-review.analyzeFile";
    statusBarItem.show();
    context.subscriptions.push(statusBarItem);

    // 初始审查当前文件
    if (vscode.window.activeTextEditor) {
        analyzeDocument(vscode.window.activeTextEditor.document);
    }
}

// 扩展停用
export function deactivate() {
    if (diagnosticCollection) {
        diagnosticCollection.dispose();
    }
}

/**
 * 分析整个工作区
 */
async function analyzeWorkspace() {
    const config = vscode.workspace.getConfiguration('cpp-review');
    const workspaceFolders = vscode.workspace.workspaceFolders;

    if (!workspaceFolders) {
        vscode.window.showErrorMessage('未打开工作区');
        return;
    }

    const workspacePath = workspaceFolders[0].uri.fsPath;

    vscode.window.withProgress(
        {
            location: vscode.ProgressLocation.Notification,
            title: "正在分析工作区...",
            cancellable: false
        },
        async (progress) => {
            try {
                await runAnalysis(workspacePath, progress);
                vscode.window.showInformationMessage('工作区分析完成');
            } catch (error) {
                vscode.window.showErrorMessage(`分析失败: ${error}`);
            }
        }
    );
}

/**
 * 分析当前文件
 */
async function analyzeCurrentFile() {
    const editor = vscode.window.activeTextEditor;
    if (!editor) {
        vscode.window.showErrorMessage('未打开文件');
        return;
    }

    await analyzeDocument(editor.document);
}

/**
 * 自动修复问题
 */
async function autoFixIssues() {
    const editor = vscode.window.activeTextEditor;
    if (!editor) {
        vscode.window.showErrorMessage('未打开文件');
        return;
    }

    const document = editor.document;
    const filePath = document.uri.fsPath;

    if (!isCppFile(document)) {
        vscode.window.showWarningMessage('当前文件不是 C/C++ 文件');
        return;
    }

    const config = vscode.workspace.getConfiguration('cpp-review');
    const execPath = config.get<string>('executablePath', 'cpp-agent');
    const cppStd = config.get<string>('cppStandard', 'c++17');

    vscode.window.withProgress(
        {
            location: vscode.ProgressLocation.Notification,
            title: "正在自动修复...",
            cancellable: false
        },
        async (progress) => {
            try {
                // 保存当前文件
                await document.save();

                // 运行自动修复
                const args = [
                    'scan',
                    filePath,
                    `--std=${cppStd}`,
                    '--auto-fix',
                    '--backup'
                ];

                await executeCommand(execPath, args);

                // 重新加载文件
                await vscode.commands.executeCommand('workbench.action.files.revert');

                vscode.window.showInformationMessage('自动修复完成');

                // 重新分析
                await analyzeDocument(document);

            } catch (error) {
                vscode.window.showErrorMessage(`自动修复失败: ${error}`);
            }
        }
    );
}

/**
 * 清除诊断
 */
function clearDiagnostics() {
    diagnosticCollection.clear();
    vscode.window.showInformationMessage('已清除所有诊断信息');
}

/**
 * 文件保存事件处理
 */
async function onDocumentSave(document: vscode.TextDocument) {
    const config = vscode.workspace.getConfiguration('cpp-review');
    const enableAutoReview = config.get<boolean>('enableAutoReview', true);

    if (enableAutoReview && isCppFile(document)) {
        await analyzeDocument(document);
    }
}

/**
 * 编辑器切换事件处理
 */
async function onEditorChange(editor: vscode.TextEditor | undefined) {
    if (editor && isCppFile(editor.document)) {
        await analyzeDocument(editor.document);
    }
}

/**
 * 分析文档
 */
async function analyzeDocument(document: vscode.TextDocument) {
    if (!isCppFile(document)) {
        return;
    }

    const filePath = document.uri.fsPath;
    const config = vscode.workspace.getConfiguration('cpp-review');
    const execPath = config.get<string>('executablePath', 'cpp-agent');
    const cppStd = config.get<string>('cppStandard', 'c++17');
    const enableTaint = config.get<boolean>('enableTaintAnalysis', true);
    const enableAI = config.get<boolean>('enableAISuggestions', false);
    const aiProvider = config.get<string>('aiProvider', 'rule-based');

    try {
        // 构建命令参数
        const args = [
            'scan',
            filePath,
            `--std=${cppStd}`,
            '--json-output'
        ];

        if (enableTaint) {
            args.push('--enable-taint-analysis');
        }

        if (enableAI) {
            args.push('--enable-ai');
            args.push(`--ai-provider=${aiProvider}`);

            if (aiProvider === 'openai') {
                const apiKey = config.get<string>('openaiApiKey');
                if (apiKey) {
                    args.push(`--openai-key=${apiKey}`);
                }
            } else if (aiProvider === 'anthropic') {
                const apiKey = config.get<string>('anthropicApiKey');
                if (apiKey) {
                    args.push(`--anthropic-key=${apiKey}`);
                }
            }
        }

        // 执行分析
        const output = await executeCommand(execPath, args);

        // 解析 JSON 输出
        const issues = parseIssues(output);

        // 更新诊断
        updateDiagnostics(document, issues);

    } catch (error) {
        console.error('分析失败:', error);
    }
}

/**
 * 运行分析
 */
async function runAnalysis(
    targetPath: string,
    progress: vscode.Progress<{ message?: string; increment?: number }>
): Promise<void> {
    const config = vscode.workspace.getConfiguration('cpp-review');
    const execPath = config.get<string>('executablePath', 'cpp-agent');
    const cppStd = config.get<string>('cppStandard', 'c++17');

    const args = [
        'scan',
        targetPath,
        `--std=${cppStd}`,
        '--json-output'
    ];

    progress.report({ message: '正在扫描文件...' });
    const output = await executeCommand(execPath, args);

    progress.report({ message: '正在生成报告...' });
    const issues = parseIssues(output);

    progress.report({ message: '正在更新诊断...' });
    // 按文件分组问题
    const issuesByFile = new Map<string, any[]>();
    for (const issue of issues) {
        const filePath = issue.file_path;
        if (!issuesByFile.has(filePath)) {
            issuesByFile.set(filePath, []);
        }
        issuesByFile.get(filePath)!.push(issue);
    }

    // 更新每个文件的诊断
    for (const [filePath, fileIssues] of issuesByFile) {
        const uri = vscode.Uri.file(filePath);
        const document = await vscode.workspace.openTextDocument(uri);
        updateDiagnostics(document, fileIssues);
    }
}

/**
 * 执行命令
 */
function executeCommand(command: string, args: string[]): Promise<string> {
    return new Promise((resolve, reject) => {
        const proc = cp.spawn(command, args);
        let stdout = '';
        let stderr = '';

        proc.stdout.on('data', (data) => {
            stdout += data.toString();
        });

        proc.stderr.on('data', (data) => {
            stderr += data.toString();
        });

        proc.on('close', (code) => {
            if (code === 0) {
                resolve(stdout);
            } else {
                reject(new Error(stderr || `进程退出码: ${code}`));
            }
        });

        proc.on('error', (error) => {
            reject(error);
        });
    });
}

/**
 * 解析问题
 */
function parseIssues(output: string): any[] {
    try {
        // 尝试解析 JSON 输出
        const data = JSON.parse(output);
        return data.issues || [];
    } catch {
        // 如果不是 JSON，尝试解析文本输出
        return parseTextOutput(output);
    }
}

/**
 * 解析文本输出
 */
function parseTextOutput(output: string): any[] {
    const issues: any[] = [];
    const lines = output.split('\n');

    // 简化的文本解析逻辑
    for (const line of lines) {
        const match = line.match(/(.+):(\d+):(\d+):\s*(\w+):\s*(.+)/);
        if (match) {
            issues.push({
                file_path: match[1],
                line: parseInt(match[2]),
                column: parseInt(match[3]),
                severity: match[4],
                description: match[5]
            });
        }
    }

    return issues;
}

/**
 * 更新诊断
 */
function updateDiagnostics(document: vscode.TextDocument, issues: any[]) {
    const diagnostics: vscode.Diagnostic[] = [];
    const config = vscode.workspace.getConfiguration('cpp-review');
    const severityFilter = config.get<string[]>('severityFilter', [
        'CRITICAL', 'HIGH', 'MEDIUM', 'LOW', 'SUGGESTION'
    ]);

    for (const issue of issues) {
        // 过滤严重性
        if (!severityFilter.includes(issue.severity)) {
            continue;
        }

        const line = Math.max(0, (issue.line || 1) - 1);
        const column = Math.max(0, (issue.column || 1) - 1);
        const range = new vscode.Range(line, column, line, column + 100);

        const severity = mapSeverity(issue.severity);
        const diagnostic = new vscode.Diagnostic(
            range,
            issue.description,
            severity
        );

        diagnostic.code = issue.rule_id;
        diagnostic.source = 'C++ Review Agent';

        if (issue.suggestion) {
            diagnostic.relatedInformation = [
                new vscode.DiagnosticRelatedInformation(
                    new vscode.Location(document.uri, range),
                    `建议: ${issue.suggestion}`
                )
            ];
        }

        diagnostics.push(diagnostic);
    }

    diagnosticCollection.set(document.uri, diagnostics);
}

/**
 * 映射严重性级别
 */
function mapSeverity(severity: string): vscode.DiagnosticSeverity {
    switch (severity.toUpperCase()) {
        case 'CRITICAL':
        case 'HIGH':
            return vscode.DiagnosticSeverity.Error;
        case 'MEDIUM':
            return vscode.DiagnosticSeverity.Warning;
        case 'LOW':
            return vscode.DiagnosticSeverity.Information;
        case 'SUGGESTION':
            return vscode.DiagnosticSeverity.Hint;
        default:
            return vscode.DiagnosticSeverity.Warning;
    }
}

/**
 * 检查是否为 C++ 文件
 */
function isCppFile(document: vscode.TextDocument): boolean {
    const languageId = document.languageId;
    return languageId === 'cpp' || languageId === 'c';
}
