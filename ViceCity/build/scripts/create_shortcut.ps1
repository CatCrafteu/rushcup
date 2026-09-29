<#+
.SYNOPSIS
    Creates a desktop shortcut to ViceCity build folder

.DESCRIPTION
    Creates a shortcut on the Desktop pointing to the ViceCity build output directory.
    Run after building the cheat.
#>

param(
    [string]$Config = "Release",
    [string]$Platform = "x64"
)

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$ProjectRoot = Split-Path -Parent (Split-Path -Parent $ScriptDir)
$TargetDir = Join-Path $ProjectRoot "build\bin\$Config"
$ShortcutPath = Join-Path [Environment]::GetFolderPath("Desktop") "ViceCity Build.lnk"

Write-Host "=== Creating ViceCity Build Folder Shortcut ===" -ForegroundColor Cyan
Write-Host "Target: $TargetDir" -ForegroundColor Gray
Write-Host "Shortcut: $ShortcutPath" -ForegroundColor Gray

if (-not (Test-Path $TargetDir)) {
    Write-Error "Build directory not found at: $TargetDir"
    Write-Host "Build the cheat first: .\build_release.ps1" -ForegroundColor Yellow
    exit 1
}

$Shell = New-Object -ComObject WScript.Shell
$Shortcut = $Shell.CreateShortcut($ShortcutPath)
$Shortcut.TargetPath = $TargetDir
$Shortcut.WorkingDirectory = $TargetDir
$Shortcut.Description = "ViceCity CS2 Cheat Build Output - Contains ViceCity.dll"
$Shortcut.Save()

Write-Host "Shortcut created successfully on Desktop!" -ForegroundColor Green
