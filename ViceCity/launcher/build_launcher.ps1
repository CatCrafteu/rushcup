<#+
.SYNOPSIS
    Build ViceCity Launcher

.DESCRIPTION
    Builds the launcher in Release mode. The launcher auto-starts Steam, launches CS2, and injects ViceCity.dll.
#>

param(
    [switch]$Clean,
    [string]$Config = "Release",
    [string]$Platform = "x64"
)

$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$ProjectRoot = Split-Path -Parent $ScriptDir
$BuildDir = Join-Path $ScriptDir "build"
$OutDir = Join-Path $BuildDir "bin" $Config

Write-Host "=== ViceCity Launcher Build ===" -ForegroundColor Cyan
Write-Host "Config: $Config" -ForegroundColor Gray

if ($Clean) {
    Write-Host "Cleaning..." -ForegroundColor Yellow
    if (Test-Path $BuildDir) {
        Remove-Item -Recurse -Force $BuildDir -ErrorAction SilentlyContinue
    }
}

if (-not (Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

Write-Host "Configuring..." -ForegroundColor Cyan
& cmake -S $ScriptDir -B $BuildDir -G "Visual Studio 18 2026" -A $Platform -DCMAKE_BUILD_TYPE=$Config
if ($LASTEXITCODE -ne 0) { Write-Error "CMake configure failed!"; exit 1 }

Write-Host "Building..." -ForegroundColor Cyan
& cmake --build $BuildDir --config $Config --parallel
if ($LASTEXITCODE -ne 0) { Write-Error "Build failed!"; exit 1 }

$ExePath = Join-Path $OutDir "ViceCityLauncher.exe"
if (Test-Path $ExePath) {
    $Info = Get-Item $ExePath
    Write-Host "Build successful!" -ForegroundColor Green
    Write-Host "Output: $ExePath" -ForegroundColor Cyan
    Write-Host "Size: $([math]::Round($Info.Length / 1KB, 2)) KB" -ForegroundColor Gray
} else {
    Write-Warning "Launcher exe not found at expected location"
}

# Copy cheat DLL if exists
$CheatDll = Join-Path $ProjectRoot "build\bin\$Config\ViceCity.dll"
$TargetDll = Join-Path $OutDir "ViceCity.dll"
if (Test-Path $CheatDll) {
    Copy-Item $CheatDll $TargetDll -Force
    Write-Host "Copied ViceCity.dll to launcher directory" -ForegroundColor Green
} else {
    Write-Warning "ViceCity.dll not found at $CheatDll - build cheat first!"
}

Write-Host "=== Launcher Build Complete ===" -ForegroundColor Cyan

# Create desktop shortcut
Write-Host "Creating desktop shortcut..." -ForegroundColor Cyan
& "$ScriptDir\create_shortcut.ps1" -Config $Config -Platform $Platform
if ($LASTEXITCODE -eq 0) {
    Write-Host "Desktop shortcut created!" -ForegroundColor Green
}

Write-Host ""
Write-Host "Usage:" -ForegroundColor Yellow
Write-Host "  1. Double-click 'ViceCity Launcher' on Desktop (Run as Administrator)" -ForegroundColor Gray
Write-Host "  2. It will start Steam -> Launch CS2 -> Inject cheat" -ForegroundColor Gray
Write-Host "  3. Press INSERT in-game to open menu" -ForegroundColor Gray
