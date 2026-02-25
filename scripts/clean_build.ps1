# Windows Terminal Plus 编译产物清理脚本
# 清理 obj 中间文件，保留 bin 最终产物和 packages
# 用法: .\clean_build.ps1

$ErrorActionPreference = "Stop"

# 路径配置（基于脚本所在位置自动推算）
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$terminalDir = Split-Path -Parent $scriptDir
$objDir = Join-Path $terminalDir "obj"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  编译产物清理" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# 统计清理前的大小
if (Test-Path $objDir) {
    $sizeBefore = (Get-ChildItem $objDir -Recurse -ErrorAction SilentlyContinue | Measure-Object -Property Length -Sum).Sum
    $sizeGB = '{0:N2}' -f ($sizeBefore / 1GB)
    Write-Host "obj 目录大小: $sizeGB GB" -ForegroundColor Yellow
    Write-Host "路径: $objDir" -ForegroundColor Gray
    Write-Host ""

    $confirm = Read-Host "确认删除 obj 目录? (y/N)"
    if ($confirm -eq 'y' -or $confirm -eq 'Y') {
        Write-Host "正在清理..." -ForegroundColor Yellow
        Remove-Item -Recurse -Force $objDir
        Write-Host "已释放 $sizeGB GB" -ForegroundColor Green
    } else {
        Write-Host "已取消" -ForegroundColor Gray
    }
} else {
    Write-Host "obj 目录不存在，无需清理" -ForegroundColor Gray
}

Write-Host ""
Read-Host "按回车键退出"
