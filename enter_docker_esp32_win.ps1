[CmdletBinding()]
param(
    [switch]$NoSerial
)

$ErrorActionPreference = "Stop"
$firmwareDir = $PSScriptRoot
$composeBase = Join-Path $firmwareDir "docker-compose.yml"
$composeSerial = Join-Path $firmwareDir "docker-compose.serial.yml"
$envFile = Join-Path $firmwareDir ".env"

if (-not (Get-Command docker -ErrorAction SilentlyContinue)) {
    throw "No se encontro Docker. Instala o inicia Docker Desktop."
}

$previousPreference = $ErrorActionPreference
try {
    # Docker Desktop puede escribir warnings de blkio en stderr aun cuando
    # `docker info` termina correctamente. Windows PowerShell 5 no debe
    # convertir esos warnings en una excepcion.
    $ErrorActionPreference = "Continue"
    $dockerInfoOutput = docker info 2>&1 | Out-String
    $dockerInfoExitCode = $LASTEXITCODE
} finally {
    $ErrorActionPreference = $previousPreference
}
if ($dockerInfoExitCode -ne 0) {
    Write-Host $dockerInfoOutput
    throw "Docker Desktop no esta iniciado o el daemon no responde."
}

if (-not (Test-Path -LiteralPath $composeBase)) {
    throw "No se encontro docker-compose.yml en $firmwareDir"
}

Push-Location $firmwareDir
try {
    if ($NoSerial) {
        Write-Host "[ESP32] Entrando al contenedor ESP-IDF sin puerto serie..." -ForegroundColor Cyan
        docker compose -f $composeBase run --rm idf bash
    } else {
        if (-not (Test-Path -LiteralPath $envFile)) {
            throw "No existe .env. Ejecuta primero: .\config_env_esp32_win"
        }

        $envValues = @{}
        Get-Content -LiteralPath $envFile | ForEach-Object {
            if ($_ -match "^([^#=]+)=(.*)$") {
                $envValues[$Matches[1].Trim()] = $Matches[2].Trim()
            }
        }
        $port = $envValues["ESPPORT"]
        if ([string]::IsNullOrWhiteSpace($port)) {
            throw "El archivo .env no contiene ESPPORT. Ejecuta nuevamente: .\config_env_esp32_win"
        }

        Write-Host "[ESP32] Entrando al contenedor ESP-IDF con $port..." -ForegroundColor Cyan
        docker compose -f $composeBase -f $composeSerial run --rm idf bash
    }
} finally {
    Pop-Location
}
