# Flash a container-built binary from Windows; native building is opt-in.
[CmdletBinding()]
param(
    [switch] $Build
)

$ErrorActionPreference = 'Stop'

function Require-Tool([string] $Name, [string] $Hint) {
    $command = Get-Command "$Name.exe" -CommandType Application -ErrorAction SilentlyContinue | Select-Object -First 1
    if (-not $command) {
        throw "$Name was not found on PATH. $Hint"
    }
    return $command.Source
}

function Invoke-Tool([string] $Executable, [string[]] $Arguments) {
    & $Executable @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "$(Split-Path $Executable -Leaf) failed with exit code $LASTEXITCODE."
    }
}

try {
    if (-not $IsWindows -and $PSVersionTable.PSVersion.Major -ge 6) {
        throw 'This script is for native Windows. Use ./upload on Linux or macOS.'
    }

    $projectRoot = $PSScriptRoot
    $buildDir = Join-Path $projectRoot 'build'
    if ($Build) {
        $buildDir = Join-Path $projectRoot 'build/windows'
    }
    $binFile = Join-Path $buildDir 'exo-firmware.bin'

    if ($Build) {
        $cmake = Require-Tool 'cmake' 'Install CMake and add it to PATH.'
        $null = Require-Tool 'ninja' 'Install Ninja and add it to PATH.'
        foreach ($name in @('arm-none-eabi-gcc', 'arm-none-eabi-g++', 'arm-none-eabi-objcopy', 'arm-none-eabi-size')) {
            $null = Require-Tool $name 'Install STM32CubeCLT and add its GNU-tools-for-STM32/bin directory to PATH.'
        }
        Write-Host 'Building firmware (native Windows)...'
        # Keep Windows CMake files separate from the container's Linux cache.
        Invoke-Tool $cmake -Arguments @('-S', $projectRoot, '-B', $buildDir, '-G', 'Ninja', '-DCMAKE_BUILD_TYPE=Debug')
        Invoke-Tool $cmake -Arguments @('--build', $buildDir)
    }

    if (-not (Test-Path -LiteralPath $binFile -PathType Leaf)) {
        throw "Firmware binary not found: $binFile. Build in the dev container with 'cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug' and 'cmake --build build', or use -Build for a native Windows build."
    }

    $programmer = Require-Tool 'STM32_Programmer_CLI' 'Install STM32CubeProgrammer and add its bin directory to PATH.'
    Write-Host "Flashing STM32H7 via ST-LINK SWD: $binFile"
    Invoke-Tool $programmer -Arguments @('-c', 'port=SWD', '-w', $binFile, '0x08000000', '-v', '-rst')
    Write-Host 'Flash and verification complete. Board is running.'
}
catch {
    [Console]::Error.WriteLine("ERROR: $_")
    exit 1
}
