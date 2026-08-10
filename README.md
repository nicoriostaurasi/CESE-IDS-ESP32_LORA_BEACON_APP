# Firmware IDS LoRa Beacon

> **Alcance:** este repositorio es una aplicación de demostración desarrollada
> para IDS. Su objetivo es ejercitar e integrar el módulo de comunicación
> ESP32-S3/SX1262; no pretende ser el firmware final de TESIS para la CESE.

Firmware ESP-IDF para presentar el módulo de comunicación LoRa/SPI basado en
ESP32-S3 y SX1262. Este branch contiene una sola aplicación: dos nodos beacon
independientes que transmiten cada 3 segundos y escuchan entre transmisiones.

## Funcionamiento

```text
Nodo 0: SIMAI,0,001  SIMAI,0,002  SIMAI,0,003 ...
Nodo 1: SIMAI,1,001  SIMAI,1,002  SIMAI,1,003 ...
```

No hay ACK ni respuesta encadenada. Cada nodo mantiene su propio contador. El
nodo 1 aplica un offset inicial de 1500 ms para reducir colisiones.

La OLED informa identidad, último RX/TX, intervalo de recepción y potencia RSSI:

```text
ESP32 + SX1262
IDS BEACON N:0
RX:004 TX:005
DT:3001ms P:-39dBm
```

El LED digital integrado permanece apagado durante el arranque, la espera y la
transmisión. Se enciende durante 1 segundo únicamente después de recibir y
validar un paquete del otro nodo. El pulso no bloquea las tareas FreeRTOS y el
monitor informa su duración real.

## Componentes

- `app_runtime`: planificación periódica, recepción y tareas FreeRTOS.
- `beacon_protocol`: parseo/formato `SIMAI,<nodo>,<secuencia>`.
- `sx1262_radio`: submódulo Git con el driver LoRa/SPI reutilizable
  [`ESP32-S3-SX1262-DRIVER`](https://github.com/nicoriostaurasi/ESP32-S3-SX1262-DRIVER).
- `board_config`: pinout Heltec WiFi LoRa 32 V3.
- `oled_display`, `display_graphics`, `status_led`, `board_feedback`: interfaz local.

## Obtener el repositorio

El driver de radio se encuentra versionado como submódulo en
`components/sx1262_radio`. Al clonar la demo se debe inicializar mediante:

```bash
git clone --recurse-submodules https://github.com/nicoriostaurasi/CESE-IDS-ESP32_LORA_BEACON_APP.git
```

Si el repositorio ya fue clonado sin submódulos:

```bash
git submodule update --init --recursive
```

La aplicación solamente consume la API pública:

```c
#include "sx1262_radio.h"
```

ESP-IDF descubre automáticamente el submódulo porque está ubicado dentro de
`components/`. La implementación del transporte SPI permanece encapsulada en
el repositorio del driver.

## Preparar el entorno Docker

Los siguientes comandos se ejecutan desde PowerShell, en la raíz del
repositorio. Primero se construye la imagen basada en ESP-IDF 5.3.5:

```powershell
docker compose build
```

La imagen resultante se llama `cese-ids-beacon-idf:v5.3.5`. Para comprobar el
entorno:

```powershell
docker compose run --rm idf idf.py --version
```

Para abrir una terminal dentro del contenedor sin conectar la placa:

```powershell
.\enter_docker_esp32_win -NoSerial
```

También se puede entrar directamente con Docker Compose:

```powershell
docker compose run --rm idf bash
```

## Compilar

Una vez dentro del contenedor, se puede compilar cada nodo sin flashearlo:

```bash
bash scripts/idf_build_beacon_node_0.sh
bash scripts/idf_build_beacon_node_1.sh
```

También puede hacerse en un solo comando desde PowerShell, sin entrar de forma
interactiva al contenedor:

```powershell
docker compose run --rm idf bash scripts/idf_build_beacon_node_0.sh
docker compose run --rm idf bash scripts/idf_build_beacon_node_1.sh
```

Los binarios quedan respectivamente en `build_beacon_node_0/` y
`build_beacon_node_1/` dentro del repositorio.

## Conectar, flashear y monitorear

### Requisitos de Windows

- Docker Desktop iniciado y configurado para usar WSL 2.
- Una distribución WSL instalada.
- `usbipd-win`, que puede instalarse desde PowerShell como administrador:

```powershell
winget install --id dorssel.usbipd-win -e
```

Conectar la placa Heltec al USB. Para comprobar que Windows la detecta:

```powershell
usbipd list
```

Normalmente aparece como `CP2102`, `CP210x`, `Silicon Labs` o `Heltec`, junto
con un identificador similar a `2-2` en la columna `BUSID`.

### Compartir el puerto con WSL

Desde PowerShell, en la raíz del repositorio, ejecutar:

```powershell
.\config_env_esp32_win
```

El script realiza automáticamente estas operaciones:

1. Busca la interfaz USB CP2102 de la placa.
2. Ejecuta `usbipd bind` si el dispositivo todavía no está compartido.
3. Ejecuta `usbipd attach --wsl` si todavía no está adjuntado.
4. Busca el puerto Linux, normalmente `/dev/ttyUSB0`.
5. Crea el archivo local `.env` con `ESPPORT` y `ESPBAUD`.

Si hay más de una placa conectada, indicar el BUSID obtenido con
`usbipd list`:

```powershell
.\config_env_esp32_win -BusId 2-2
```

Que el script informe que el dispositivo ya estaba compartido o adjuntado no
es un error: reutiliza la conexión existente.

### Entrar al contenedor con el puerto serie

Después de ejecutar la configuración anterior:

```powershell
.\enter_docker_esp32_win
```

Este comando combina `docker-compose.yml` con `docker-compose.serial.yml` y
expone dentro del contenedor el puerto guardado en `.env`. Se puede verificar
desde la terminal del contenedor con:

```bash
echo "$ESPPORT"
ls -l "$ESPPORT"
```

La salida esperada es una ruta como `/dev/ttyUSB0`.

### Compilar y flashear

Dentro del contenedor con acceso serial, ejecutar solamente el script del nodo
que se quiere grabar:

```bash
# Nodo 0
bash scripts/idf_build_flash_beacon_node_0.sh

# Nodo 1
bash scripts/idf_build_flash_beacon_node_1.sh
```

Para hacerlo directamente desde PowerShell, sin abrir una terminal interactiva:

```powershell
# Nodo 0
docker compose -f docker-compose.yml -f docker-compose.serial.yml run --rm idf `
  bash scripts/idf_build_flash_beacon_node_0.sh

# Nodo 1
docker compose -f docker-compose.yml -f docker-compose.serial.yml run --rm idf `
  bash scripts/idf_build_flash_beacon_node_1.sh
```

### Monitor serie

Desde el contenedor interactivo:

```bash
bash scripts/idf_monitor.sh
```

También puede iniciarse directamente desde PowerShell:

```powershell
docker compose -f docker-compose.yml -f docker-compose.serial.yml run --rm idf `
  bash scripts/idf_monitor.sh
```

El monitor de ESP-IDF se cierra con `Ctrl+]`, no con `Ctrl+C`.

Si el puerto desaparece después de desconectar o reiniciar Docker/WSL, volver a
ejecutar `.\config_env_esp32_win` antes de entrar al contenedor.

Preparación de Windows/Docker: [docs/guia_rapida_esp32_windows.md](docs/guia_rapida_esp32_windows.md).
Integración del driver: [docs/integracion_driver.md](docs/integracion_driver.md).
Prueba beacon: [docs/protocol/beacon_ids.md](docs/protocol/beacon_ids.md).

## Pruebas host

```bash
bash scripts/test_beacon_protocol_host.sh
bash scripts/test_beacon_metrics_host.sh
bash scripts/test_sx1262_api_host.sh
bash scripts/test_sx1262_params_host.sh
bash scripts/test_sx1262_transport_host.sh
```

Todas juntas: `bash scripts/test_all_host.sh`.

Los tres scripts `test_sx1262_*` son envoltorios que ejecutan las pruebas
propias del submódulo; la demo no mantiene una segunda copia del driver.

Trazabilidad: [docs/trazabilidad.md](docs/trazabilidad.md).
Ensayo físico: [docs/protocol/aceptacion_ids.md](docs/protocol/aceptacion_ids.md).
