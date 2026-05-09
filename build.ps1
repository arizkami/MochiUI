$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $ScriptDir

$vcvars = "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"

# Run vcvars64 and import env vars into current PowerShell session
$envVars = cmd /c "`"$vcvars`" && set"
foreach ($line in $envVars) {
    if ($line -match "^([^=]+)=(.*)$") {
        [System.Environment]::SetEnvironmentVariable($matches[1], $matches[2], "Process")
    }
}

if (Test-Path "build") {
    # Build directory exists: skip CMake setup, just build
    cmake --build build -j $env:NUMBER_OF_PROCESSORS
} else {
    # First time: full CMake setup + build
    cmake -S . -G "Ninja" -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build -j $env:NUMBER_OF_PROCESSORS
}
