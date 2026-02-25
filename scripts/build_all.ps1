# Windows Terminal Plus 一键编译+生成便携版脚本
# 用法: 右键点击此脚本，选择"使用 PowerShell 运行"
# 或者在 PowerShell 中运行: .\build_all.ps1

$ErrorActionPreference = "Stop"

# 路径配置（基于脚本所在位置自动推算）
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$terminalDir = Split-Path -Parent $scriptDir
$msbuildPath = "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
$createPortableScript = Join-Path $scriptDir "create_portable.ps1"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Windows Terminal Plus 一键构建" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# 检查 MSBuild 是否存在
if (-not (Test-Path $msbuildPath)) {
    Write-Host "错误: 找不到 MSBuild.exe" -ForegroundColor Red
    Write-Host "请确认 Visual Studio 2022 已安装" -ForegroundColor Red
    Read-Host "按回车键退出"
    exit 1
}

# 检查源码目录是否存在
if (-not (Test-Path $terminalDir)) {
    Write-Host "错误: 找不到源码目录 $terminalDir" -ForegroundColor Red
    Read-Host "按回车键退出"
    exit 1
}

# 检查 create_portable.ps1 是否存在
if (-not (Test-Path $createPortableScript)) {
    Write-Host "错误: 找不到 create_portable.ps1" -ForegroundColor Red
    Read-Host "按回车键退出"
    exit 1
}

# ========== 步骤 1: 编译 ==========
Write-Host "[1/2] 开始编译..." -ForegroundColor Yellow
Write-Host "源码目录: $terminalDir" -ForegroundColor Gray
Write-Host ""

Push-Location $terminalDir
try {
    # 关闭 WholeProgramOptimization (LTCG) 大幅减少中间文件体积
    # LTCG 的 .obj 文件比普通 obj 大 10-20 倍，是 30GB 的主要来源
    # 对便携版的运行性能影响很小
    & $msbuildPath OpenConsole.slnx /p:Configuration=Release /p:Platform=x64 /p:WholeProgramOptimization=false /m

    if ($LASTEXITCODE -ne 0) {
        Write-Host ""
        Write-Host "编译失败!" -ForegroundColor Red
        Read-Host "按回车键退出"
        exit 1
    }

    Write-Host ""
    Write-Host "编译成功!" -ForegroundColor Green
}
finally {
    Pop-Location
}

# ========== 步骤 2: 生成便携版 ==========
Write-Host ""
Write-Host "[2/2] 生成便携版..." -ForegroundColor Yellow
Write-Host ""

& $createPortableScript

if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "生成便携版失败!" -ForegroundColor Red
    Read-Host "按回车键退出"
    exit 1
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "  全部完成!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "便携版位置:" -ForegroundColor Cyan
Write-Host "$scriptDir\portable_build\WindowsTerminalPortable\WindowsTerminal.exe" -ForegroundColor White
Write-Host ""
Read-Host "按回车键退出"
