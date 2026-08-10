# Cheat sheet de puerto serial

Guia rapida para recuperar y usar la Heltec WiFi LoRa 32 V3 desde Docker cuando
el puerto queda tomado, el monitor no responde o se pierde `/dev/ttyUSB0`.

## Datos habituales

- USB-UART de la placa: CP2102.
- VID:PID: `10c4:ea60`.
- Puerto esperado en Linux/WSL/Docker: `/dev/ttyUSB0`.
- Baud recomendado para flash por USBIP: `115200`.
- Baud fallback si USBIP corta el flash: `57600` con `--no-stub`.
- `BUSID` observado en esta maquina: `2-2`.
- Si el `BUSID` cambia, usar el valor actual mostrado por `usbipd list`.

## Ver estado desde PowerShell

```powershell
& "C:\Program Files\usbipd-win\usbipd.exe" list
```

Estados utiles:

```text
Not shared  -> falta bind
Shared      -> listo para attach
Attached    -> WSL/Docker deberia ver el USB
```

## Configuracion desde cero

Primera vez o si aparece `Not shared`:

```powershell
# PowerShell como administrador
& "C:\Program Files\usbipd-win\usbipd.exe" bind --busid 2-2
```

Luego:

```powershell
# PowerShell normal
& "C:\Program Files\usbipd-win\usbipd.exe" attach --wsl --busid 2-2
```

Verificar puerto Linux:

```powershell
wsl sh -lc "modprobe cp210x 2>/dev/null || true; ls -l /dev/ttyUSB* /dev/ttyACM* 2>/dev/null"
```

## Entrar al Docker con puerto mapeado

```powershell
cd C:\Users\nicol\Documents\CESE\simai-mesh\firmware
$env:ESPPORT="/dev/ttyUSB0"
$env:ESPBAUD="115200"
docker compose -f docker-compose.yml -f docker-compose.serial.yml run --rm idf bash
```

Dentro del contenedor:

```bash
idf.py build
idf.py -p /dev/ttyUSB0 -b 115200 flash
idf.py -p /dev/ttyUSB0 monitor
```

Para salir del monitor interactivo de ESP-IDF:

```text
Ctrl+]
```

Nota: `Ctrl+T` es el prefijo de comandos de `idf_monitor`, pero para salir el
atajo directo es `Ctrl+]`.

## Si el monitor queda colgado

Desde otra PowerShell:

```powershell
cd C:\Users\nicol\Documents\CESE\simai-mesh\firmware
docker compose -f docker-compose.yml -f docker-compose.serial.yml down --remove-orphans
```

Si sigue tomado el puerto, listar contenedores y matar el que este usando IDF:

```powershell
docker ps
docker kill <CONTAINER_ID>
```

Luego resetear el attach USB:

```powershell
& "C:\Program Files\usbipd-win\usbipd.exe" detach --busid 2-2
& "C:\Program Files\usbipd-win\usbipd.exe" attach --wsl --busid 2-2
wsl sh -lc "modprobe cp210x 2>/dev/null || true; ls -l /dev/ttyUSB* /dev/ttyACM* 2>/dev/null"
```

Volver a entrar al contenedor:

```powershell
cd C:\Users\nicol\Documents\CESE\simai-mesh\firmware
$env:ESPPORT="/dev/ttyUSB0"
$env:ESPBAUD="115200"
docker compose -f docker-compose.yml -f docker-compose.serial.yml run --rm idf bash
```

## Si desaparece `/dev/ttyUSB0`

1. Desconectar y reconectar la placa.
2. Revisar el `BUSID` actual:

```powershell
& "C:\Program Files\usbipd-win\usbipd.exe" list
```

3. Adjuntar nuevamente:

```powershell
& "C:\Program Files\usbipd-win\usbipd.exe" attach --wsl --busid 2-2
```

4. Cargar driver y verificar:

```powershell
wsl sh -lc "modprobe cp210x 2>/dev/null || true; dmesg | tail -n 20; ls -l /dev/ttyUSB* /dev/ttyACM* 2>/dev/null"
```

## Comandos rapidos de uso normal

Build:

```powershell
cd C:\Users\nicol\Documents\CESE\simai-mesh\firmware
docker compose run --rm idf idf.py build
```

Flash:

```powershell
cd C:\Users\nicol\Documents\CESE\simai-mesh\firmware
$env:ESPPORT="/dev/ttyUSB0"
$env:ESPBAUD="115200"
docker compose -f docker-compose.yml -f docker-compose.serial.yml run --rm idf bash scripts/idf_flash.sh
```

Si aparece `A fatal error occurred: The chip stopped responding`, recuperar el
attach USB y flashear de forma mas conservadora:

```powershell
docker compose -f docker-compose.yml -f docker-compose.serial.yml down --remove-orphans
& "C:\Program Files\usbipd-win\usbipd.exe" detach --busid 2-2
& "C:\Program Files\usbipd-win\usbipd.exe" attach --wsl --busid 2-2
$env:ESPPORT="/dev/ttyUSB0"
docker compose -f docker-compose.yml -f docker-compose.serial.yml run --rm idf bash -lc "python -m esptool --chip esp32s3 -p /dev/ttyUSB0 -b 57600 --before default_reset --after hard_reset --no-stub write_flash --flash_mode dio --flash_size 8MB --flash_freq 80m 0x0 build/bootloader/bootloader.bin 0x8000 build/partition_table/partition-table.bin 0x10000 build/ids_lora_beacon.bin"
```

Monitor interactivo:

```powershell
cd C:\Users\nicol\Documents\CESE\simai-mesh\firmware
$env:ESPPORT="/dev/ttyUSB0"
$env:ESPBAUD="115200"
docker compose -f docker-compose.yml -f docker-compose.serial.yml run --rm idf bash
```

Dentro del contenedor:

```bash
idf.py -p /dev/ttyUSB0 monitor
```

## Prueba rapida sin monitor interactivo

Cuando `idf_monitor` no puede tomar TTY desde esta terminal, se puede leer el
serial con Python dentro del contenedor:

```powershell
cd C:\Users\nicol\Documents\CESE\simai-mesh\firmware
$env:ESPPORT="/dev/ttyUSB0"
$env:ESPBAUD="115200"
docker compose -f docker-compose.yml -f docker-compose.serial.yml run --rm idf bash -lc "python - <<'PY'
import serial, time
ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=0.2)
end = time.time() + 6
buf = b''
while time.time() < end:
    buf += ser.read(4096)
ser.close()
print(buf.decode('utf-8', errors='replace'))
PY"
```
