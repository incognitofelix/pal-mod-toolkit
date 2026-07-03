<#
.SYNOPSIS
    Generates an import library (UE4SS.lib) from an installed UE4SS.dll.

.DESCRIPTION
    PalModToolkit links against the UE4SS.dll the game already loads, instead of
    recompiling UE4SS. To link, the MSVC linker needs an import library built from
    that DLL's export table. This script produces it with dumpbin + lib.

    Run it once (and again whenever you update UE4SS). Output goes to the folder you
    pass as -OutDir; point xmake at it via `--ue4ss_implib=<that folder>`.

.PARAMETER Ue4ssDll
    Full path to UE4SS.dll (e.g. ...\Pal\Binaries\Win64\ue4ss\UE4SS.dll).

.PARAMETER OutDir
    Folder to write UE4SS.lib / UE4SS.def into. Defaults to .\external\ue4ss_implib.

.PARAMETER MsvcWineBin
    Linux only. Folder containing the msvc-wine wrapper scripts (dumpbin/lib), e.g.
    /opt/msvc/bin/x64. On Windows this is ignored; MSVC is located via vswhere instead.

.EXAMPLE
    .\scripts\generate_import_lib.ps1 -Ue4ssDll "C:\...\ue4ss\UE4SS.dll"

.EXAMPLE
    pwsh ./scripts/generate_import_lib.ps1 -Ue4ssDll "/path/to/ue4ss/UE4SS.dll" -MsvcWineBin /opt/msvc/bin/x64
#>
param(
    [Parameter(Mandatory = $true)] [string] $Ue4ssDll,
    [string] $OutDir = (Join-Path $PSScriptRoot "..\external\ue4ss_implib"),
    [string] $MsvcWineBin
)

$ErrorActionPreference = "Stop"

if ($IsLinux) {
    # Cross-compiling via msvc-wine: dumpbin/lib are Wine-wrapper shell scripts, not a
    # real Visual Studio install, so vswhere.exe doesn't apply here.
    if (-not $MsvcWineBin) { $MsvcWineBin = "/opt/msvc/bin/x64" }
    if (-not (Test-Path $MsvcWineBin)) { throw "msvc-wine bin dir not found: $MsvcWineBin (pass -MsvcWineBin)." }
    $bin = $MsvcWineBin
} else {
    # Locate dumpbin.exe / lib.exe from a Visual Studio installation via vswhere.
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (-not (Test-Path $vswhere)) { throw "vswhere.exe not found - install Visual Studio with the C++ workload." }
    $vsPath = & $vswhere -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (-not $vsPath) { throw "No Visual Studio with the C++ toolset found." }

    $msvcRoot = Join-Path $vsPath "VC\Tools\MSVC"
    $msvcVer  = (Get-ChildItem $msvcRoot -Directory | Sort-Object Name -Descending | Select-Object -First 1).Name
    $bin      = Join-Path $msvcRoot "$msvcVer\bin\Hostx64\x64"
}

if (-not (Test-Path $Ue4ssDll)) { throw "UE4SS.dll not found at: $Ue4ssDll" }
New-Item -ItemType Directory -Force -Path $OutDir | Out-Null

# 1) Parse the DLL export table. Every mangled C++ export dumpbin prints has a trailing
# " = <decorated name> (<demangled signature>)" -- the 4th column (the name this DLL
# actually exports) still ends where the whitespace before " = " starts, so the pattern
# must not anchor on end-of-line or it fails to match almost every export.
$raw = & (Join-Path $bin "dumpbin.exe") /EXPORTS $Ue4ssDll
$names = foreach ($line in $raw) {
    if ($line -match '^\s*\d+\s+[0-9A-Fa-f]+\s+[0-9A-Fa-f]+\s+(\S+)') { $matches[1] }
}
Write-Host "Parsed $($names.Count) exported symbols."

# 2) Write a .def file.
$def = "EXPORTS`r`n" + (($names | ForEach-Object { "    $_" }) -join "`r`n") + "`r`n"
$defPath = Join-Path $OutDir "UE4SS.def"
Set-Content -Path $defPath -Value $def -Encoding ascii

# 3) Build the import library.
& (Join-Path $bin "lib.exe") "/def:$defPath" "/out:$(Join-Path $OutDir 'UE4SS.lib')" /machine:x64 | Out-Null

Write-Host "Created $(Join-Path $OutDir 'UE4SS.lib')"
