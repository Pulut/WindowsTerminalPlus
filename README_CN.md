# Windows Terminal Plus

基于 [Windows Terminal](https://github.com/microsoft/terminal) 的修改版，内置 AI 助手启动面板。

[English](README.md)

## 功能特性

- **AI 助手启动器** — 左侧面板集成 Claude Code、Codex CLI、Gemini CLI 三个 AI 编程助手的快捷启动按钮，选择工作目录后一键启动
- **常用命令面板（Snippets）** — 右侧上半区，可通过 `snippets.json` 自定义常用命令（如 git 操作、构建命令等），点击即可执行
- **AI 命令面板** — 右侧下半区，根据当前启动的 AI 助手自动切换对应的命令列表（如 `/compact`、`/clear` 等），通过 `commands.json` 配置
- **Git 记录查看** — 左侧面板底部显示所选项目的最近提交记录
- **便携版支持** — 可打包为免安装的便携版，解压即用

## 前提条件

本工具只是一个启动器，你需要先在系统中安装并配置好对应的 AI 助手，确保在命令行中能正常调用：

| AI 助手 | 安装方式 | 验证命令 |
|---------|---------|---------|
| Claude Code | `npm install -g @anthropic-ai/claude-code` | `claude` |
| Codex CLI | `npm install -g @openai/codex` | `codex` |
| Gemini CLI | `npm install -g @anthropic-ai/gemini-cli` | `gemini` |

安装后需要配置对应的 API Key 或登录账号，具体请参考各工具的官方文档。

### 自定义启动命令

左侧面板三个按钮的启动命令默认为：

| 按钮 | 默认命令 |
|------|---------|
| Anthropic | `cc` |
| OpenAI_GPT | `codex` |
| Google | `gemini` |

如果你的命令名称不同（例如你用 `claude` 而不是 `cc`），需要修改源码文件 `src/cascadia/TerminalApp/LauncherPaneContent.cpp` 中对应的启动命令，然后重新编译：

```cpp
void LauncherPaneContent::_claudeClick(const IInspectable&, const RoutedEventArgs&)
{
    _launchWithCommand(L"claude\r");  // 改成你的命令
}
```

## 下载使用

前往 [Releases](https://github.com/Pulut/WindowsTerminalPlus/releases) 下载便携版压缩包，解压后运行 `WindowsTerminal.exe`。

## 自定义命令配置

便携版目录下有两个 JSON 配置文件：

### snippets.json — 常用命令

显示在右侧上半区，点击直接执行。格式：

```json
{
  "snippets": [
    {
      "name": "git push",
      "input": "git push\r",
      "description": "推送到远程仓库"
    }
  ]
}
```

- `name`：显示名称
- `input`：实际执行的命令（`\r` 表示自动回车执行）
- `description`：命令说明

### commands.json — AI 助手命令

显示在右侧下半区，根据当前启动的 AI 助手自动切换。格式：

```json
{
  "claude": [
    { "cmd": "/compact", "desc": "压缩上下文" },
    { "cmd": "/clear", "desc": "清空上下文" }
  ],
  "codex": [
    { "cmd": "/compact", "desc": "压缩上下文" }
  ],
  "gemini": [
    { "cmd": "/compress", "desc": "压缩上下文" }
  ]
}
```

点击命令会直接发送到当前终端。

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

2. 编译：
   ```powershell
   & "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" `
       OpenConsole.slnx /p:Configuration=Release /p:Platform=x64 /m
   ```

3. 编译产物在 `bin\` 目录下。

## 截图

*（待补充）*

## 致谢

基于 [Microsoft Windows Terminal](https://github.com/microsoft/terminal)，遵循 MIT 许可证。

## 许可证

[MIT License](LICENSE)
