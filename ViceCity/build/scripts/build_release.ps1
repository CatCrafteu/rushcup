<#+
.SYNOPSIS
    Build ViceCity CS2 Cheat - Release Configuration

.DESCRIPTION
    Builds the ViceCity project in Release mode with all optimizations enabled.
    Outputs to build/bin/Release/

.NOTES
    Requires: Visual Studio 2022 17.4+, CMake 3.20+, Windows 10 SDK 10.0.22621+
#>

param(
    [switch]$Clean,
    [switch]$Package,
    [string]$Config = "Release",
    [string]$Platform = "x64",
    [int]$Jobs = 0
)

$ErrorActionPreference = "Stop"

# Paths
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$ProjectRoot = Split-Path -Parent (Split-Path -Parent $ScriptDir)
$BuildDir = Join-Path $ProjectRoot "build\build"
$OutDir = Join-Path (Join-Path $BuildDir "bin") $Config

Write-Host "=== ViceCity Build Script ===" -ForegroundColor Cyan
Write-Host "Project: $ProjectRoot" -ForegroundColor Gray
Write-Host "Config: $Config" -ForegroundColor Gray
Write-Host "Platform: $Platform" -ForegroundColor Gray
Write-Host ""

# Clean if requested
if ($Clean) {
    Write-Host "Cleaning build directory..." -ForegroundColor Yellow
    if (Test-Path $BuildDir) {
        Remove-Item -Recurse -Force $BuildDir -ErrorAction SilentlyContinue
    }
}

# Create build directory
if (-not (Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

# Configure with CMake
Write-Host "Configuring with CMake..." -ForegroundColor Cyan
$CMakeArgs = @(
    "-S", $ProjectRoot
    "-B", $BuildDir
    "-G", "Visual Studio 18 2026"
    "-A", $Platform
    "-DCMAKE_BUILD_TYPE=$Config"
)

if ($LASTEXITCODE -ne 0) {
    & cmake @CMakeArgs
    if ($LASTEXITCODE -ne 0) {
        Write-Error "CMake configuration failed!"
        exit 1
    }
}

# Build
Write-Host "Building..." -ForegroundColor Cyan
$BuildArgs = @(
    "--build", $BuildDir
    "--config", $Config
)

if ($Jobs -gt 0) {
    $BuildArgs += "--parallel", $Jobs
} else {
    $BuildArgs += "--parallel"
}

& cmake @BuildArgs
if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed!"
    exit 1
}

Write-Host "Build successful!" -ForegroundColor Green

# Verify output
$DllPath = Join-Path $OutDir "ViceCity.dll"
if (Test-Path $DllPath) {
    $FileInfo = Get-Item $DllPath
    Write-Host "Output: $DllPath" -ForegroundColor Cyan
    Write-Host "Size: $([math]::Round($FileInfo.Length / 1KB, 2)) KB" -ForegroundColor Gray
    Write-Host "Modified: $($FileInfo.LastWriteTime)" -ForegroundColor Gray
} else {
    Write-Warning "DLL not found at expected location: $DllPath"
}

# Package if requested
if ($Package) {
    Write-Host "Creating package..." -ForegroundColor Cyan
    $PackageDir = Join-Path $BuildDir "package"
    if (Test-Path $PackageDir) {
        Remove-Item -Recurse -Force $PackageDir
    }
    New-Item -ItemType Directory -Path $PackageDir | Out-Null
    
    Copy-Item $DllPath -Destination $PackageDir
    
    # Copy dependencies if any
    $Deps = @(
        "minhook.x64.dll",
        "lua54.dll"
    )
    
    foreach ($Dep in $Deps) {
        $DepPath = Join-Path $OutDir $Dep
        if (Test-Path $DepPath) {
            Copy-Item $DepPath -Destination $PackageDir
        }
    }
    
    # Create zip
    $ZipPath = Join-Path $BuildDir "ViceCity-$Config-$Platform.zip"
    if (Test-Path $ZipPath) {
        Remove-Item $ZipPath
    }
    
    Compress-Archive -Path (Join-Path $PackageDir "*") -DestinationPath $ZipPath -Force
    Write-Host "Package: $ZipPath" -ForegroundColor Green
}

Write-Host "=== Build Complete ===" -ForegroundColor Cyan

# Create desktop shortcut to build folder
Write-Host "Creating desktop shortcut to build folder..." -ForegroundColor Cyan
& "$ScriptDir\create_shortcut.ps1" -Config $Config -Platform $Platform
if ($LASTEXITCODE -eq 0) {
    Write-Host "Desktop shortcut 'ViceCity Build' created!" -ForegroundColor Green
}

Write-Host ""
Write-Host "Next steps:" -ForegroundColor Yellow
Write-Host "  1. Copy ViceCity.dll from build folder to launcher folder" -ForegroundColor Gray
Write-Host "  2. Build launcher: cd ..\launcher && .\build_launcher.ps1" -ForegroundColor Gray
Write-Host "  3. Run 'ViceCity Launcher' from Desktop as Administrator" -ForegroundColor Gray
