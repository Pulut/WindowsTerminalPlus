$ErrorActionPreference = 'Stop'

# 路径配置（基于脚本所在位置自动推算）
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$terminalDir = Split-Path -Parent $scriptDir

$msixPath = Join-Path $terminalDir 'src\cascadia\CascadiaPackage\AppPackages\CascadiaPackage_0.0.1.0_x64_Test\CascadiaPackage_0.0.1.0_x64.msix'
$xamlAppx = Join-Path $terminalDir 'packages\Microsoft.UI.Xaml.2.8.4\tools\AppX\x64\Release\Microsoft.UI.Xaml.2.8.appx'
$outputDir = Join-Path $scriptDir 'portable_build'
$makeappx = 'C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\MakeAppx.exe'
$makepri = 'C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64\MakePri.exe'

# Clean output
if (Test-Path $outputDir) { Remove-Item -Recurse -Force $outputDir }
New-Item -ItemType Directory -Path $outputDir | Out-Null

# Extract Terminal MSIX
$terminalExtract = Join-Path $outputDir 'terminal_extracted'
New-Item -ItemType Directory -Path $terminalExtract | Out-Null
Write-Host "Extracting Terminal MSIX..."
& $makeappx unpack /p $msixPath /d $terminalExtract /o
if ($LASTEXITCODE -ne 0) { throw 'Failed to extract Terminal MSIX' }

# Extract XAML
$xamlExtract = Join-Path $outputDir 'xaml_extracted'
New-Item -ItemType Directory -Path $xamlExtract | Out-Null
Write-Host "Extracting XAML..."
& $makeappx unpack /p $xamlAppx /d $xamlExtract /o
if ($LASTEXITCODE -ne 0) { throw 'Failed to extract XAML MSIX' }

# Create final directory
$finalDir = Join-Path $outputDir 'WindowsTerminalPortable'
Copy-Item -Recurse $terminalExtract $finalDir

# Copy XAML files
Write-Host "Copying XAML files..."
Copy-Item (Join-Path $xamlExtract 'Microsoft.UI.Xaml.dll') $finalDir
Copy-Item -Recurse (Join-Path $xamlExtract 'Microsoft.UI.Xaml') $finalDir

# Merge resources.pri
Write-Host "Merging resources.pri..."
$mergeScript = Join-Path $terminalDir 'build\scripts\Merge-TerminalAndXamlResources.ps1'
$finalPri = Join-Path $finalDir 'resources.pri'
& $mergeScript -TerminalRoot $finalDir -XamlRoot $xamlExtract -OutputPath $finalPri -MakePriPath $makepri

# Clean up unnecessary files
Write-Host "Cleaning up..."
Get-Item (Join-Path $finalDir '*.xml') -EA:SilentlyContinue | Remove-Item -Force
Get-Item (Join-Path $finalDir '*.winmd') -EA:SilentlyContinue | Remove-Item -Force
Get-ChildItem (Join-Path $finalDir 'Appx*') -EA:SilentlyContinue | Remove-Item -Recurse -Force
Get-ChildItem (Join-Path $finalDir 'Images') -Filter '*Tile*' -EA:SilentlyContinue | Remove-Item -Force
Get-ChildItem (Join-Path $finalDir 'Images') -Filter '*Logo*' -EA:SilentlyContinue | Remove-Item -Force
# Remove webp and jpg from Assets (we use PNG instead)
Get-ChildItem (Join-Path $finalDir 'Assets') -Filter '*.webp' -EA:SilentlyContinue | Remove-Item -Force
Get-ChildItem (Join-Path $finalDir 'Assets') -Filter '*.jpg' -EA:SilentlyContinue | Remove-Item -Force

# Copy PNG images for AI buttons
$pngSource = Join-Path $scriptDir 'temp_png'
if (Test-Path $pngSource) {
    Copy-Item (Join-Path $pngSource '*.png') (Join-Path $finalDir 'Assets') -Force
}

# Add portable marker
"" | Out-File (Join-Path $finalDir '.portable')

Write-Host ""
Write-Host "Portable build created at: $finalDir"
$exe = Join-Path $finalDir 'WindowsTerminal.exe'
if (Test-Path $exe) {
    Write-Host "WindowsTerminal.exe exists: $(Get-Item $exe | Select-Object -Expand Length) bytes"
} else {
    Write-Host "ERROR: WindowsTerminal.exe not found!"
}
