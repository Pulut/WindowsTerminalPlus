# Windows Terminal Plus

基于 [Windows Terminal](https://github.com/microsoft/terminal) 的修改版，内置 AI 助手启动面板。

[English](README.md)

## 功能特性

- **AI 助手启动器** — 左侧面板集成 Claude Code、Codex CLI、Gemini CLI 三个 AI 编程助手的快捷启动按钮，选择工作目录后一键启动
- **常用命令面板（Snippets）** — 右侧上半区，可通过 `snippets.json` 自定义常用命令（如 git 操作、构建命令等），点击即可执行
- **AI 命令面板** — 右侧下半区，根据当前启动的 AI 助手自动切换对应的命令列表（如 `/compact`、`/clear` 等），通过 `commands.json` 配置
- **Git 记录查看** — 左侧面板底部显示所选项目的最近提交记录
- **便携版支持** — 可打包为免安装的便携版，解压即用

## 下载使用

前往 [Releases](https://github.com/Pulut/WindowsTerminalPlus/releases) 下载便携版压缩包，解压后运行 `WindowsTerminal.exe`。

## 前提条件

本工具是一个启动器界面，不包含 AI 助手本身。你需要先在系统中安装并配置好对应的 AI 助手，确保在 cmd、PowerShell 或 Windows Terminal 中能正常调用。

### 安装 AI 助手

| AI 助手 | 安装方式 | 验证命令 |
|---------|---------|---------|
| Claude Code | `npm install -g @anthropic-ai/claude-code` | 在终端输入 `claude` 能正常启动 |
| Codex CLI | `npm install -g @openai/codex` | 在终端输入 `codex` 能正常启动 |
| Gemini CLI | `npm install -g @google/gemini-cli` | 在终端输入 `gemini` 能正常启动 |

安装后需要配置对应的 API Key 或登录账号，具体请参考各工具的官方文档。

### 自定义启动命令

左侧面板三个按钮点击后，会在终端中自动执行以下命令：

| 按钮 | 默认启动命令 |
|------|------------|
| Anthropic | `cc` |
| OpenAI_GPT | `codex` |
| Google | `gemini` |

如果你的命令名称不同（例如你安装后的命令是 `claude` 而不是 `cc`），需要修改源码后重新编译。

修改文件：`src/cascadia/TerminalApp/LauncherPaneContent.cpp`

```cpp
// Claude Code 启动命令，把 L"cc\r" 改成你的命令
void LauncherPaneContent::_claudeClick(const IInspectable&, const RoutedEventArgs&)
{
    _launchWithCommand(L"claude\r");  // 改成你实际使用的命令
}

// Codex CLI 启动命令
void LauncherPaneContent::_codexClick(const IInspectable&, const RoutedEventArgs&)
{
    _launchWithCommand(L"codex\r");
}

// Gemini CLI 启动命令
void LauncherPaneContent::_geminiClick(const IInspectable&, const RoutedEventArgs&)
{
    _launchWithCommand(L"gemini\r");
}
```

`\r` 表示自动回车执行，不要删除。

## 自定义命令配置

便携版目录下（与 `WindowsTerminal.exe` 同级）有两个 JSON 配置文件，可以用文本编辑器直接修改：

### snippets.json — 常用命令

显示在右侧上半区，点击直接发送到当前终端执行。

```json
{
  "snippets": [
    {
      "name": "git push",
      "input": "git push\r",
      "description": "推送到远程仓库"
    },
    {
      "name": "git add && commit",
      "input": "git add . && git commit -m \"\"",
      "description": "暂存所有改动并提交，请修改提交信息"
    }
  ]
}
```

| 字段 | 说明 |
|------|------|
| `name` | 显示名称 |
| `input` | 实际执行的命令，末尾加 `\r` 表示自动回车执行，不加则需要手动按回车 |
| `description` | 命令说明 |

### commands.json — AI 助手命令

显示在右侧下半区。当你通过左侧按钮启动某个 AI 助手后，会自动切换显示对应的命令列表，点击即可发送到终端。

```json
{
  "claude": [
    { "cmd": "/compact", "desc": "压缩上下文" },
    { "cmd": "/clear", "desc": "清空上下文" },
    { "cmd": "/model", "desc": "切换模型" },
    { "cmd": "/cost", "desc": "查看 Token 用量和费用" }
  ],
  "codex": [
    { "cmd": "/compact", "desc": "压缩上下文" },
    { "cmd": "/new", "desc": "清除当前对话历史" },
    { "cmd": "/diff", "desc": "显示文件更改差异" }
  ],
  "gemini": [
    { "cmd": "/compress", "desc": "压缩上下文" },
    { "cmd": "/stats", "desc": "查看会话统计" },
    { "cmd": "/restore", "desc": "回滚文件改动" }
  ]
}
```

| 字段 | 说明 |
|------|------|
| 顶层 key | AI 助手名称（`claude`、`codex`、`gemini`），与启动命令对应 |
| `cmd` | 要发送的命令 |
| `desc` | 命令说明 |

## 从源码构建

### 环境要求

- Windows 10 或更高版本
- Visual Studio 2022（需安装 C++ 桌面开发工作负载）
- Windows SDK（10.0.22621.0 或更高版本）

### 构建步骤

1. 克隆仓库：
   ```powershell
   git clone https://github.com/Pulut/WindowsTerminalPlus.git
   cd WindowsTerminalPlus
   ```

2. 使用一键构建脚本（编译 + 生成便携版）：
   ```powershell
   .\scripts\build_all.ps1
   ```

   或者手动编译：
   ```powershell
   & "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" `
       OpenConsole.slnx /p:Configuration=Release /p:Platform=x64 /m
   ```

3. 便携版输出在 `scripts\portable_build\WindowsTerminalPortable\` 目录下。

### 构建脚本说明

| 脚本 | 说明 |
|------|------|
| `scripts\build_all.ps1` | 一键编译 + 生成便携版 |
| `scripts\create_portable.ps1` | 仅生成便携版（需要先编译） |
| `scripts\clean_build.ps1` | 清理编译中间文件（obj），释放磁盘空间 |

## 截图

![screenshot](scripts/temp_png/screenshot.png)

## 致谢

基于 [Microsoft Windows Terminal](https://github.com/microsoft/terminal)，遵循 MIT 许可证。

## 许可证

[MIT License](LICENSE)
