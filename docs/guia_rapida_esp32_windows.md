# Guía rápida IDS Beacon en Windows

## Preparar USB y entrar al contenedor

Desde PowerShell, con una placa conectada:

```powershell
cd C:\Users\nicol\Documents\CESE\simai-mesh\firmware
.\config_env_esp32_win
.\enter_docker_esp32_win
```

El primer script comparte el CP2102 mediante `usbipd`, lo adjunta a WSL y
genera `.env`. Dentro del contenedor el puerto esperado es `/dev/ttyUSB0` y está
disponible como `$ESPPORT`.

## Nodo beacon 0

```bash
bash scripts/idf_build_flash_beacon_node_0.sh
idf.py -p "$ESPPORT" monitor
```

Salir del monitor con `Ctrl+]`.

## Nodo beacon 1

Conectar la segunda placa y, si fuera necesario, repetir la preparación USB.
Luego, dentro del contenedor:

```bash
bash scripts/idf_build_flash_beacon_node_1.sh
idf.py -p "$ESPPORT" monitor
```

## Sólo compilar

```bash
bash scripts/idf_build_beacon_node_0.sh
bash scripts/idf_build_beacon_node_1.sh
```

Los binarios quedan en:

```text
build_beacon_node_0/ids_lora_beacon.bin
build_beacon_node_1/ids_lora_beacon.bin
```

Importante: `idf_build_beacon_node_X.sh` sólo compila. Para cargar físicamente
la placa hay que usar `idf_build_flash_beacon_node_X.sh`.
