# Entorno ESP-IDF con Docker

## Version registrada

- Imagen Docker: `espressif/idf:v5.3.5`.
- Target ESP-IDF: `esp32s3`.
- Baud de flash por defecto: `115200`.
- Proyecto: `ids_lora_beacon`.
- Plataforma objetivo: Heltec WiFi LoRa 32 V3 433 MHz.

La imagen oficial `espressif/idf` contiene ESP-IDF y las herramientas necesarias
para compilar proyectos. La version concreta queda fijada en `Dockerfile` y
`docker-compose.yml`.

El contenedor monta el repositorio completo en `/workspace` y compila desde
`/workspace/firmware`, para que ESP-IDF pueda acceder a la metadata de Git del
proyecto.

## Build

Desde `firmware/`:

```bash
docker compose build
docker compose run --rm idf idf.py set-target esp32s3
docker compose run --rm idf idf.py build
```

Tambien puede usarse el script:

```bash
docker compose run --rm idf bash scripts/idf_build.sh
```

## Menuconfig

```bash
docker compose run --rm idf bash scripts/idf_menuconfig.sh
```

## Flash y monitor en Linux o WSL2

Si la placa aparece como `/dev/ttyUSB0`:

```bash
ESPPORT=/dev/ttyUSB0 ESPBAUD=115200 docker compose -f docker-compose.yml -f docker-compose.serial.yml run --rm idf bash scripts/idf_flash.sh
ESPPORT=/dev/ttyUSB0 docker compose -f docker-compose.yml -f docker-compose.serial.yml run --rm idf bash scripts/idf_monitor.sh
```

Para otro puerto:

```bash
ESPPORT=/dev/ttyACM0 ESPBAUD=115200 docker compose -f docker-compose.yml -f docker-compose.serial.yml run --rm idf bash scripts/idf_flash.sh
```

## Windows, PowerShell y WSL2

El flujo oficial del proyecto es usar siempre Docker. En Windows, la placa puede
aparecer inicialmente como `COM3`, `COM4` u otro puerto `COMx`, pero Docker
Desktop con contenedores Linux normalmente no puede usar ese puerto serie
directamente. Para mantener el flujo Docker, se debe exponer el USB a WSL2 con
`usbipd` y trabajar con el puerto Linux resultante, por ejemplo `/dev/ttyUSB0`.

Para recuperacion rapida del puerto ante monitores colgados, detach/attach o
perdida de `/dev/ttyUSB0`, usar
[serial_port_cheatsheet.md](C:/Users/nicol/Documents/CESE/simai-mesh/firmware/docs/serial_port_cheatsheet.md).

Flujo recomendado desde PowerShell:

```powershell
usbipd list
usbipd bind --busid <BUSID>
usbipd attach --wsl --busid <BUSID>
```

Luego, desde WSL2:

```bash
ls /dev/ttyUSB* /dev/ttyACM*
cd /mnt/c/Users/nicol/Documents/CESE/simai-mesh/firmware
docker compose build
docker compose run --rm idf idf.py build
ESPPORT=/dev/ttyUSB0 ESPBAUD=115200 docker compose -f docker-compose.yml -f docker-compose.serial.yml run --rm idf bash scripts/idf_flash.sh
ESPPORT=/dev/ttyUSB0 docker compose -f docker-compose.yml -f docker-compose.serial.yml run --rm idf bash scripts/idf_monitor.sh
```

Si el dispositivo aparece como `/dev/ttyACM0`, reemplazar `ESPPORT` por ese
valor.

## WSL2 con usbipd

Pasos resumidos:

1. Compartir el dispositivo USB desde Windows con `usbipd`.
2. Adjuntarlo a la distribucion WSL2.
3. Verificar el puerto dentro de WSL2, por ejemplo `/dev/ttyUSB0`.
4. Ejecutar `build`, `flash` y `monitor` con Docker.

## Limitaciones conocidas

- El acceso serial depende de como Windows, WSL2 o Linux expongan la placa.
- El build no requiere puerto serial.
- `flash` y `monitor` requieren que el puerto exista dentro del entorno desde el
  que se ejecuta el comando.
- No se documenta como flujo principal el flasheo con ESP-IDF instalado en el
  host Windows; se mantiene Docker como unico entorno operativo del proyecto.
