[CmdletBinding()]
param(
    [string]$BusId,
    [int]$Baud = 115200
)

$ErrorActionPreference = "Stop"
$usbipd = "C:\Program Files\usbipd-win\usbipd.exe"

function Write-Step([string]$Message) {
    Write-Host "[ESP32] $Message" -ForegroundColor Cyan
}

function Get-UsbipdList {
    & $usbipd list 2>&1 | Out-String
}

function Invoke-NativeCaptured {
    param(
        [Parameter(Mandatory = $true)][string]$FilePath,
        [Parameter(Mandatory = $true)][string[]]$ArgumentList
    )

    # Windows PowerShell 5 convierte cualquier texto de stderr de un programa
    # nativo en NativeCommandError cuando ErrorActionPreference vale Stop.
    # Se captura con Continue y se decide por el exit code real del proceso.
    $previousPreference = $ErrorActionPreference
    try {
        $ErrorActionPreference = "Continue"
        $output = & $FilePath @ArgumentList 2>&1 | Out-String
        $exitCode = $LASTEXITCODE
    } finally {
        $ErrorActionPreference = $previousPreference
    }

    [pscustomobject]@{
        Output = $output
        ExitCode = $exitCode
    }
}

if (-not (Test-Path -LiteralPath $usbipd)) {
    throw "No se encontro usbipd-win. Instalarlo con: winget install --id dorssel.usbipd-win -e"
}

Write-Step "Buscando Heltec/CP2102..."
$usbList = Get-UsbipdList
$candidateLines = @($usbList -split "`r?`n" | Where-Object {
    $_ -match "CP2102|10c4:ea60|Heltec"
})

if ([string]::IsNullOrWhiteSpace($BusId)) {
    $detectedBusIds = @($candidateLines | ForEach-Object {
        if ($_ -match "^\s*([0-9]+-[0-9]+)\s+") { $Matches[1] }
    } | Sort-Object -Unique)

    if ($detectedBusIds.Count -eq 0) {
        Write-Host $usbList
        throw "No se encontro un CP2102/Heltec conectado. Conecta la placa y vuelve a ejecutar el script."
    }
    if ($detectedBusIds.Count -gt 1) {
        throw "Se encontraron varios dispositivos: $($detectedBusIds -join ', '). Ejecuta: .\config_env_esp32_win -BusId <BUSID>"
    }
    $BusId = $detectedBusIds[0]
}

if ($BusId -notmatch "^[0-9]+-[0-9]+$") {
    throw "BUSID invalido: '$BusId'. Formato esperado: 2-2"
}

$deviceLine = $candidateLines | Where-Object { $_ -match "^\s*$([regex]::Escape($BusId))\s+" } | Select-Object -First 1
if (-not $deviceLine) {
    Write-Host $usbList
    throw "El BUSID $BusId no corresponde a un CP2102/Heltec detectado."
}

Write-Step "Dispositivo detectado en BUSID $BusId"

if ($deviceLine -match "Not shared|No compartido") {
    Write-Step "El dispositivo necesita bind. Windows solicitara permisos de administrador."
    $bindProcess = Start-Process -FilePath $usbipd `
        -ArgumentList @("bind", "--busid", $BusId) `
        -Verb RunAs -Wait -PassThru
    if ($bindProcess.ExitCode -ne 0) {
        throw "usbipd bind fallo con codigo $($bindProcess.ExitCode)."
    }
} else {
    Write-Step "El dispositivo ya esta compartido; no hace falta repetir bind."
}

Write-Step "Adjuntando el dispositivo a WSL..."
$attachResult = Invoke-NativeCaptured -FilePath $usbipd -ArgumentList @("attach", "--wsl", "--busid", $BusId)
if ($attachResult.ExitCode -ne 0 -and $attachResult.Output -notmatch "already attached|ya.*adjunt") {
    Write-Host $attachResult.Output
    throw "usbipd attach fallo."
}
if ($attachResult.ExitCode -ne 0) {
    Write-Step "El dispositivo ya estaba adjuntado; se reutilizara la conexion existente."
}

Write-Step "Cargando el driver CP210x y buscando el puerto Linux..."
$portOutput = wsl sh -lc "modprobe cp210x 2>/dev/null || true; ls /dev/ttyUSB* /dev/ttyACM* 2>/dev/null || true"
$ports = @($portOutput -split "`r?`n" | ForEach-Object { $_.Trim() } | Where-Object {
    $_ -match "^/dev/tty(USB|ACM)[0-9]+$"
})

if ($ports.Count -eq 0) {
    throw "El dispositivo fue adjuntado, pero no aparecio /dev/ttyUSB* ni /dev/ttyACM*. Desconecta y reconecta la placa."
}

$espPort = $ports[0]
$envFile = Join-Path $PSScriptRoot ".env"
@(
    "ESPPORT=$espPort"
    "ESPBAUD=$Baud"
) | Set-Content -LiteralPath $envFile -Encoding ascii

$env:ESPPORT = $espPort
$env:ESPBAUD = [string]$Baud

Write-Host ""
Write-Host "Entorno ESP32 listo" -ForegroundColor Green
Write-Host "  BUSID:   $BusId"
Write-Host "  Puerto:  $espPort"
Write-Host "  Baud:    $Baud"
Write-Host "  Docker:  $envFile"
Write-Host ""
Write-Host "Ya puedes ejecutar:"
Write-Host "  docker compose run --rm idf idf.py build"
Write-Host "  docker compose -f docker-compose.yml -f docker-compose.serial.yml run --rm idf idf.py -p $espPort flash"
Write-Host "  docker compose -f docker-compose.yml -f docker-compose.serial.yml run --rm idf idf.py -p $espPort monitor"
