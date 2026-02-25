# Windows Terminal Plus

A modified version of [Windows Terminal](https://github.com/microsoft/terminal) with an integrated AI assistant launcher panel.

[中文说明](README_CN.md)

## Features

- **AI Assistant Launcher** — Quickly launch AI coding assistants (Claude Code, Codex CLI, Gemini CLI) directly from a built-in panel
- **Working Directory Picker** — Select a project folder and launch AI assistants in that directory
- **Git Log Viewer** — View recent git commit history for the selected project
- **Portable Build** — Can be packaged as a standalone portable application, no installation required

## Download

Go to [Releases](https://github.com/Pulut/WindowsTerminalPlus/releases) to download the portable version. Extract the zip and run `WindowsTerminal.exe`.

## Build from Source

### Prerequisites

- Windows 10 or later
- Visual Studio 2022 with C++ desktop development workload
- Windows SDK (10.0.22621.0 or later)

### Steps

1. Clone this repository:
   ```powershell
   git clone https://github.com/Pulut/WindowsTerminalPlus.git
   cd WindowsTerminalPlus
   ```

2. Restore NuGet packages and build:
   ```powershell
   # Build with MSBuild
   & "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" `
       OpenConsole.slnx /p:Configuration=Release /p:Platform=x64 /m
   ```

3. The built binaries will be in the `bin\` directory.

## Screenshots

*(Coming soon)*

## Credits

Based on [Microsoft Windows Terminal](https://github.com/microsoft/terminal), licensed under the MIT License.

## License

[MIT License](LICENSE)
