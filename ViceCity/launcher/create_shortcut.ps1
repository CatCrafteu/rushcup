<#+
.SYNOPSIS
    Creates a desktop shortcut to ViceCityLauncher.exe

.DESCRIPTION
    Creates a shortcut on the Desktop pointing to the built launcher executable.
    Run after building the launcher.
#>

param(
    [string]$Config = "Release",
    [string]$Platform = "x64"
)

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$ProjectRoot = Split-Path -Parent $ScriptDir
$ExePath = Join-Path $ScriptDir "build\bin\$Config\ViceCityLauncher.exe"
$ShortcutPath = Join-Path [Environment]::GetFolderPath("Desktop") "ViceCity Launcher.lnk"

Write-Host "=== Creating ViceCity Launcher Shortcut ===" -ForegroundColor Cyan
Write-Host "Target: $ExePath" -ForegroundColor Gray
Write-Host "Shortcut: $ShortcutPath" -ForegroundColor Gray

if (-not (Test-Path $ExePath)) {
    Write-Error "Launcher executable not found at: $ExePath"
    Write-Host "Build the launcher first: .\build_launcher.ps1" -ForegroundColor Yellow
    exit 1
}

$Shell = New-Object -ComObject WScript.Shell
$Shortcut = $Shell.CreateShortcut($ShortcutPath)
$Shortcut.TargetPath = $ExePath
$Shortcut.WorkingDirectory = Split-Path $ExePath
$Shortcut.Description = "ViceCity CS2 Cheat Launcher - Auto-starts Steam, launches CS2, injects cheat"
$Shortcut.IconLocation = $ExePath
$Shortcut.Save()

Write-Host "Shortcut created successfully on Desktop!" -ForegroundColor Green
Write-Host ""
Write-Host "Run as Administrator to use the launcher." -ForegroundColor Yellow
