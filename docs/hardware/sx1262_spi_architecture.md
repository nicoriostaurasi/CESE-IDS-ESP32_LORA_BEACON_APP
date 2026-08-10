# Arquitectura y algoritmo SPI del driver SX1262

## 1. Objetivo y decisiones arquitectonicas

El driver adopta una arquitectura en capas combinando los patrones **Facade** y
**Hardware Abstraction Layer (HAL)**. La aplicacion utiliza una API de radio y
no conoce opcodes, formato de transacciones, GPIO, SPI ni primitivas FreeRTOS.

La plataforma inyecta host SPI y pines mediante `radio_hw_config_t`; el driver
no depende de `board_config` ni de una placa específica. Un mutex de API cubre
cada operación pública completa para impedir que TX, RX o reconfiguración
intercalen comandos. El transporte conserva otro mutex para cada trama SPI.

```text
app_runtime / beacon_protocol
          |
          v
sx1262_radio        Fachada y protocolo del transceptor
          |
          v
sx1262_transport    Algoritmo transaccional independiente de plataforma
          |
          v
ESP-IDF adapter     SPI master + GPIO BUSY + mutex + reloj/delay
          |
          v
ESP32-S3 / SX1262
```

Responsabilidades:

- `sx1262_radio`: inicializacion, configuracion LoRa, comandos SX1262, TX/RX,
  IRQ y metricas.
- `sx1262_transport`: construccion de tramas SPI, sincronizacion, espera
  acotada de BUSY y clasificacion de errores de transporte.
- Adaptador ESP-IDF: implementa callbacks de lock, BUSY, transferencia, reloj
  monotono y espera usando FreeRTOS/ESP-IDF.
- `sx1262_params`: validaciones y conversiones puras de parametros LoRa.

Esta separacion aplica inversion de dependencias: el algoritmo depende de una
interfaz de operaciones y no del hardware concreto. Los tests sustituyen el
adaptador ESP-IDF por un bus simulado determinista.

## 2. Estructuras de datos y visibilidad

La interfaz publica `sx1262_transport_ops_t` contiene las operaciones que la
plataforma debe proveer:

- `lock()` / `unlock()`: exclusion mutua de una transaccion completa.
- `is_busy()`: lectura del GPIO BUSY.
- `transfer()`: intercambio SPI full-duplex con NSS gestionado por el driver.
- `now_us()`: tiempo monotono para deadlines.
- `sleep_ms()`: espera cooperativa entre consultas de BUSY.
- `context`: dependencia inyectada para hardware real o fake de pruebas.

`sx1262_transport_t` conserva una copia de esas operaciones y los limites de
tiempo de BUSY y lock. Buffers, helpers y estado ESP-IDF permanecen privados en
los archivos `.c`.

Los diagramas fuente se encuentran en:

- `docs/uml/sx1262_spi_components.puml`
- `docs/uml/sx1262_spi_write_sequence.puml`
- `docs/uml/sx1262_spi_read_sequence.puml`
- `docs/uml/sx1262_spi_transport_states.puml`

## 3. Algoritmo transaccional

### 3.1 Escritura de comando

Precondiciones: transporte inicializado; argumentos coherentes; trama no mayor
a 260 bytes.

1. Construir `TX = [opcode | argumentos]` en un buffer local inicializado.
2. Intentar tomar el mutex con timeout.
3. Consultar BUSY hasta nivel bajo, durmiendo 1 ms entre consultas y usando un
   deadline monotono.
4. Ejecutar una unica transferencia SPI. El periférico SPI controla NSS durante
   toda la trama, evitando comandos partidos.
5. Esperar nuevamente BUSY bajo para confirmar que el SX1262 proceso el comando.
6. Liberar el mutex en todos los caminos de salida.
7. Retornar un resultado tipado; la fachada lo traduce a `esp_err_t` y registra
   opcode y causa.

### 3.2 Lectura de comando

1. Validar argumentos y calcular el largo total.
2. Construir `TX = [opcode | argumentos | dummy | ceros de clock]`.
3. Tomar el mutex y esperar BUSY bajo.
4. Ejecutar una unica transferencia full-duplex.
5. Descartar los bytes de opcode/argumentos y el byte de estado/dummy.
6. Copiar los bytes de datos al buffer del llamador.
7. Liberar el mutex aun ante error.

No se espera BUSY despues de una lectura: el intercambio ya finalizo y la
siguiente operacion volvera a verificarlo antes de acceder al radio.

## 4. Resultados y excepciones controladas

| Resultado transporte | Traduccion publica | Causa |
|---|---|---|
| `OK` | `ESP_OK` | Operacion completada. |
| `INVALID_ARG` | `ESP_ERR_INVALID_ARG` | Punteros o longitudes incoherentes. |
| `FRAME_TOO_LARGE` | `ESP_ERR_INVALID_SIZE` | Trama mayor al limite estatico. |
| `BUSY_TIMEOUT` | `RADIO_ERR_BUSY` | SX1262 no quedo disponible. |
| `LOCK_TIMEOUT` | `RADIO_ERR_BUSY` | Otro contexto retuvo el transporte. |
| `IO_ERROR` | `RADIO_ERR_COMMUNICATION` | Fallo reportado por SPI master. |

La implementacion no usa excepciones del lenguaje C: todos los caminos retornan
codigos verificables y liberan los recursos adquiridos.

## 5. Trade-offs

| Decision | Beneficio | Costo |
|---|---|---|
| Callbacks HAL | Tests host y portabilidad a otra plataforma. | Una indirección por operacion. |
| API sincronica | Secuencia simple y determinista para la demo. | La tarea llamadora espera hasta completar o vencer timeout. |
| Mutex por transaccion | Evita intercalado de comandos entre tareas. | Serializa accesos al unico radio. |
| Polling BUSY cooperativo | No bloquea activamente la CPU y es facil de probar. | Resolucion aproximada de 1 ms. |
| Buffers estaticos en stack | Sin heap ni fragmentacion. | Hasta 520 bytes temporales en lecturas. |
| Transferencia SPI unica | NSS estable y trama atomica. | Requiere reservar el largo maximo. |

## 6. Verificacion y trazabilidad

Prueba unitaria del algoritmo independiente de hardware:

```bash
docker compose run --rm idf bash scripts/test_sx1262_transport_host.sh
```

Casos cubiertos:

- Formato y longitud de escritura.
- Formato de lectura y offset de los datos recibidos.
- Exclusión mutua y liberacion del lock.
- Timeout de BUSY.
- Timeout de lock.
- Error de transferencia SPI.

La verificacion de integracion se completa con el build ESP-IDF. La validacion
contra el producto requiere dos placas: inicializacion, TX, RX, payload esperado
y metricas RSSI/SNR reales.
