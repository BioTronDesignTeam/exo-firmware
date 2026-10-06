# Build and flash from native Windows PowerShell (outside the dev container).
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

    $cmake = Require-Tool 'cmake' 'Install CMake and add it to PATH.'
    $null = Require-Tool 'ninja' 'Install Ninja and add it to PATH.'
    foreach ($name in @('arm-none-eabi-gcc', 'arm-none-eabi-g++', 'arm-none-eabi-objcopy', 'arm-none-eabi-size')) {
        $null = Require-Tool $name 'Install STM32CubeCLT and add its GNU-tools-for-STM32/bin directory to PATH.'
    }
    $programmer = Require-Tool 'STM32_Programmer_CLI' 'Install STM32CubeCLT (or STM32CubeProgrammer) and add its STM32CubeProgrammer/bin directory to PATH.'

    $projectRoot = $PSScriptRoot
    $buildDir = Join-Path $projectRoot 'build/windows'
    $binFile = Join-Path $buildDir 'exo-firmware.bin'

    Write-Host '[1/2] Building firmware (native Windows)...'
    # The dev container uses build/, whose CMake cache contains Linux paths.
    Invoke-Tool $cmake -Arguments @('-S', $projectRoot, '-B', $buildDir, '-G', 'Ninja', '-DCMAKE_BUILD_TYPE=Debug')
    Invoke-Tool $cmake -Arguments @('--build', $buildDir)

    if (-not (Test-Path -LiteralPath $binFile -PathType Leaf)) {
        throw "Firmware binary was not generated: $binFile"
    }

    Write-Host '[2/2] Flashing STM32H7 via ST-LINK SWD...'
    Invoke-Tool $programmer -Arguments @('-c', 'port=SWD', '-w', $binFile, '0x08000000', '-v', '-rst')
    Write-Host 'Build and flash complete. Board is running.'
}
catch {
    [Console]::Error.WriteLine("ERROR: $_")
    exit 1
}
