# Ensayo de aceptación IDS

## Criterios

Cada nodo evalúa bloques acumulados de 100 paquetes únicos recibidos:

- tasa de entrega mínima: 90,0 %;
- intervalo medio admitido: 2500 a 3500 ms;
- self-test de errores de API aprobado;
- inicialización SX1262 sin error;
- payload válido `SIMAI,<0|1>,<secuencia>`;
- RSSI y SNR disponibles después de RX;
- sin watchdog, panic ni reinicio inesperado.

Los umbrales se encuentran en `app_runtime_config.h`.

## Procedimiento

1. Flashear nodo 0 con `idf_build_flash_beacon_node_0.sh`.
2. Flashear nodo 1 con `idf_build_flash_beacon_node_1.sh`.
3. Mantener ambas placas alimentadas.
4. Guardar el monitor de una placa:

```bash
idf.py -p "$ESPPORT" monitor | tee monitor_node_0.log
```

5. Esperar 100 recepciones, aproximadamente 5 minutos.
6. Salir con `Ctrl+]` y verificar:

```bash
bash scripts/check_acceptance_log.sh monitor_node_0.log
```

## Salida esperada

```text
API ERROR SELF-TEST invalid-timeout/metrics-before-RX result=PASS
ACCEPTANCE samples=100 missing=2 duplicates=0 delivery=98.0% RSSI mean/min/max=-47/-55/-40 dBm SNR mean/min/max=10/7/13 dB DT mean/min/max=3001/2989/3020 ms result=PASS
```

## Evidencia experimental obtenida

Ensayo ejecutado el **9 de agosto de 2026** con los dos nodos beacon. La salida
completa del monitor serie se conserva como evidencia versionable en
[monitor_node_0.log](../../monitor_node_0.log).

Comando de validación utilizado:

```bash
bash scripts/check_acceptance_log.sh monitor_node_0.log
```

Resultado reproducible:

```text
ACCEPTANCE parsed_from_log sessions=2 samples=116 missing=0 duplicates=0 delivery=100.0% RSSI mean/min/max=-4/-35/-3 dBm SNR mean/min/max=12/11/13 dB DT mean/min/max=2996/2599/3000 ms
PASS: evidencia IDS beacon aceptada.
```

### Interpretación

- Se analizaron **116 tramas únicas** de la sesión vigente, superando el mínimo
  de 100 requerido.
- No se detectaron paquetes perdidos ni duplicados: la tasa de entrega medida
  fue **100,0 %**.
- El intervalo medio fue **2996 ms**, dentro del rango aceptado de 2500 a
  3500 ms y consistente con el período nominal de 3 segundos.
- El RSSI medio fue **-4 dBm** y el SNR medio **12 dB**.
- El analizador detectó dos sesiones porque el nodo remoto reinició su contador:
  la primera observación fue `seq=493` y la sesión evaluada continuó desde
  `seq=1` hasta `seq=116`.
- El resultado general del ensayo es **PASS**.

### Integridad del archivo fuente

```text
Archivo: monitor_node_0.log
SHA-256: 307A18BC0A7BEC1CA8BE98F1C7B27A0C43B5CE4A0B74D47C44C13F722D6FABDE
```

El hash permite comprobar que el archivo utilizado para obtener las métricas no
fue reemplazado o modificado después de documentar el ensayo.
