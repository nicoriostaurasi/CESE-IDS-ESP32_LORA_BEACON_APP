# Driver SX1262

## Alcance

El componente implementa el driver para el SX1262 integrado en la Heltec WiFi
LoRa 32 V3, usando ESP-IDF nativo:

- SPI mediante `spi_master`.
- GPIO de reset, busy y DIO1 inyectados mediante `radio_hw_config_t`.
- Inicializacion LoRa sin LoRaWAN.
- API simple para configurar, transmitir, recibir y leer RSSI/SNR del ultimo
  paquete recibido.

La arquitectura, el algoritmo transaccional SPI, los trade-offs y los diagramas
UML se documentan en
[`sx1262_spi_architecture.md`](sx1262_spi_architecture.md).

El protocolo mesh, ACK, reintentos, vecinos y ruteo quedan fuera de esta fase.

## Pines usados

| Funcion | GPIO |
|---|---:|
| SCLK | 9 |
| MOSI | 10 |
| MISO | 11 |
| NSS | 8 |
| RESET | 12 |
| BUSY | 13 |
| DIO1 | 14 |

## Configuracion inicial

| Parametro | Valor inicial |
|---|---:|
| Frecuencia | 433000000 Hz |
| BW | 125 kHz |
| SF | 7 |
| CR | 4/5 |
| Preambulo | 8 simbolos |
| Potencia TX | 14 dBm |

La configuracion se aplica desde `radio_task`, no desde `app_main()`.

## API

```c
esp_err_t sx1262_radio_init(const radio_hw_config_t *hw, const radio_config_t *cfg);
esp_err_t sx1262_radio_set_config(const radio_config_t *cfg);
esp_err_t sx1262_radio_send(const uint8_t *data, size_t len, uint32_t timeout_ms);
esp_err_t sx1262_radio_receive(uint8_t *buf, size_t buf_len, size_t *rx_len, uint32_t timeout_ms);
esp_err_t sx1262_radio_get_last_packet_metrics(int16_t *rssi_dbm, int8_t *snr_db);
esp_err_t sx1262_radio_sleep(void);
esp_err_t sx1262_radio_standby(void);
```

`radio_config_t` permite configurar frecuencia, spreading factor (5 a 12),
bandwidth LoRa soportado por SX1262, coding rate (4/5 a 4/8), preambulo y
potencia (-9 a 22 dBm). Una configuracion no soportada retorna
`RADIO_ERR_INVALID_CONFIG`; el driver no sustituye parametros silenciosamente.

Codigos estables para clasificar errores:

| Codigo | Significado |
|---|---|
| `ESP_OK` | Operacion completada. |
| `ESP_ERR_TIMEOUT` | Vencio el timeout de transmision o recepcion. |
| `RADIO_ERR_BUSY` | BUSY permanecio activo mas alla del limite interno. |
| `RADIO_ERR_NOT_INITIALIZED` | El radio no fue inicializado o aun no hay metricas RX. |
| `RADIO_ERR_INVALID_CONFIG` | Parametro LoRa fuera del rango soportado. |
| `RADIO_ERR_BUFFER_TOO_SMALL` | El buffer de recepcion no alcanza. |
| `RADIO_ERR_PACKET_CRC` | El paquete recibido fallo CRC. |
| `RADIO_ERR_COMMUNICATION` | Alias reservado para respuestas invalidas del transceptor. |

El calculo del timeout usa las unidades RTC de 15,625 us definidas por el
SX1262. La optimizacion para baja tasa de datos se activa automaticamente
cuando la duracion de simbolo es igual o mayor a 16,384 ms.

## Supuestos

- Se habilita DIO2 como control de RF switch del SX1262.
- Se habilita DIO3 como control de TCXO a 1,8 V. Si la revision exacta de placa
  no usa TCXO controlado por DIO3, se puede desactivar con
  `BOARD_CONFIG_SX1262_USE_DIO3_TCXO=0`.
- El driver no usa Arduino, RadioLib ni LoRaWAN.

## Validacion esperada

Para cerrar completamente la Fase 4 hacen falta dos niveles:

1. Una placa: inicializacion SX1262 sin error y logs de configuracion.
2. Dos placas: prueba TX/RX punto a punto con payload fijo y registro de RSSI/SNR
   reales.

Los valores RSSI/SNR solo son evidencia valida despues de recibir un paquete
LoRa real mediante `sx1262_radio_receive()`.

La conversion y validacion de parametros tiene pruebas host-side:

```bash
docker compose run --rm idf bash scripts/test_sx1262_params_host.sh
docker compose run --rm idf bash scripts/test_sx1262_transport_host.sh
```

Estas pruebas cubren rangos de frecuencia/potencia/SF, mapeos de bandwidth y
coding rate, optimizacion para baja tasa de datos y conversion/saturacion de
timeouts. Las operaciones SPI/GPIO y la prueba TX/RX requieren hardware real.

## Correspondencia con el anteproyecto ISSE

| Requerimiento | Implementacion / evidencia |
|---|---|
| Inicializar SPI y GPIO | `sx1262_radio_init()` configura reset, BUSY, DIO1 y el dispositivo SPI. |
| Configurar parametros LoRa | `sx1262_radio_set_config()` valida y aplica frecuencia, SF, BW, CR, preambulo y potencia. |
| Transmitir/recibir con timeout | `sx1262_radio_send()` y `sx1262_radio_receive()` usan timeout de hardware y control por software. |
| Consultar RSSI/SNR | `sx1262_radio_get_last_packet_metrics()` entrega la ultima medicion RX valida. |
| Encapsular comandos SX1262 | Los opcodes y transferencias SPI son privados al componente. |
| Informar errores | La API expone los codigos estables documentados arriba y registra contexto mediante logs. |
| Aplicacion funcional | Dos builds beacon seleccionados con `BEACON_NODE_ID=0/1`. |

## Prueba punto a punto

Flujo recomendado:

1. Conectar la placa que actuara como receptor.
2. Compilar y flashear RX.
3. Dejar RX alimentada y leyendo monitor/log.
4. Conectar la segunda placa.
5. Compilar y flashear TX.
6. Observar las lineas `RX sender=... RSSI=... SNR=...`.

Desde PowerShell, siempre desde `firmware/`:

```powershell
$env:ESPPORT="/dev/ttyUSB0"
$env:ESPBAUD="57600"
```

Modo transmisor:

```powershell
docker compose -f docker-compose.yml -f docker-compose.serial.yml run --rm idf bash scripts/idf_build_flash_p2p_tx.sh
```

Este build usa `PROJECT_VER=ids-p2p-tx`; al arrancar, el monitor muestra
`Version: ids-p2p-tx`.

Modo receptor:

```powershell
docker compose -f docker-compose.yml -f docker-compose.serial.yml run --rm idf bash scripts/idf_build_flash_p2p_rx.sh
```

Este build usa `PROJECT_VER=ids-p2p-rx`; al arrancar, el monitor muestra
`Version: ids-p2p-rx`.

Dentro del contenedor abierto con `enter_docker_esp32_win`, los comandos son:

```bash
bash scripts/idf_build_flash_p2p_tx.sh
bash scripts/idf_build_flash_p2p_rx.sh
```

Para compilar solamente:

```bash
bash scripts/idf_build_p2p_tx.sh
bash scripts/idf_build_p2p_rx.sh
```

El transmisor envia cada 5 s un payload ASCII:

```text
SIMAI,<addr>,<seq>
```

El receptor registra:

```text
RX sender=<id> seq=<n> dt=<ms> RSSI=<rssi> dBm SNR=<snr> dB
```

El TX transmite cada 5 s. El RX mantiene una ventana de recepcion de 6 s y
solo espera 10 ms antes de volver a escuchar. La ventana RX es deliberadamente
mayor que el periodo TX para evitar que ambos ciclos queden sincronizados y que
las transmisiones caigan repetidamente durante una pausa del receptor.

Las esperas de polling del driver se redondean a un minimo de un tick de
FreeRTOS. Esto es necesario porque, con `CONFIG_FREERTOS_HZ=100`, intervalos
como 1 ms o 5 ms se convertirian en cero ticks y una tarea de radio fijada a un
core podria impedir la ejecucion de la tarea IDLE y disparar el task watchdog.

## Validacion realizada

Build Docker:

```text
docker compose run --rm idf idf.py build
```

Resultado:

```text
simai_mesh.bin binary size 0x41f40 bytes
Smallest app partition is 0x100000 bytes. 0xbe0c0 bytes (74%) free.
```

Flash probado con USBIP usando `--no-stub` a 57600 baudios. Log relevante:

```text
I (...) SIMAI_MESH: Phase: Fase 4 - driver SX1262 LoRa
I (...) SX1262_RADIO: Configured LoRa: freq=433000000 Hz sf=7 bw=125000 cr=4/5 preamble=8 tx=14 dBm
I (...) SX1262_RADIO: SX1262 initialized, status=0xa2
I (...) APP_RUNTIME: radio_task started; SX1262 initialized
I (...) APP_RUNTIME: protocol_task stub started; mesh protocol is not initialized yet
```

Esta evidencia valida inicializacion SPI/reset/busy/configuracion basica del
SX1262 en una placa. No valida alcance, tasa de entrega ni RSSI/SNR de enlace.
