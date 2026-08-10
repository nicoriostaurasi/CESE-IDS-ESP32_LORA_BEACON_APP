# Prueba IDS con dos beacons LoRa

Ambas placas trabajan de forma autónoma. Cada una transmite cada 3 segundos y
permanece en recepción el resto del tiempo.

```text
Nodo 0: SIMAI,0,001  SIMAI,0,002  SIMAI,0,003 ...
Nodo 1: SIMAI,1,001  SIMAI,1,002  SIMAI,1,003 ...
```

No existen ACK, reintentos ni respuestas automáticas. El contador de cada nodo
es independiente. El nodo 1 comienza con un offset de 1500 ms.

## OLED

```text
ESP32 + SX1262
IDS BEACON N:0
RX:004 TX:005
DT:3001ms P:-39dBm
```

- `RX`: última secuencia recibida del otro beacon.
- `TX`: última secuencia local transmitida.
- `DT`: intervalo entre las últimas dos recepciones.
- `P`: potencia recibida del último paquete (RSSI, dBm).

## Comandos

```bash
bash scripts/idf_build_flash_beacon_node_0.sh
bash scripts/idf_build_flash_beacon_node_1.sh
```

Sólo compilación:

```bash
bash scripts/idf_build_beacon_node_0.sh
bash scripts/idf_build_beacon_node_1.sh
```
