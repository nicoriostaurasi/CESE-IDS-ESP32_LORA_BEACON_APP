# Board feedback

## Alcance

La Fase 3.5 agrega feedback local en la placa Heltec WiFi LoRa 32 V3 mediante:

- OLED integrada 128x64 por I2C.
- LED integrado en GPIO35.
- API `board_feedback` para que capas superiores no dibujen ni controlen GPIO
  directamente.

No se inicializa SX1262, no se implementa protocolo LoRa y no se accede a
sensores externos en esta fase.

## Componentes

| Componente | Responsabilidad |
|---|---|
| `display_graphics` | Framebuffer 128x64, texto basico y primitivas simples. |
| `oled_display` | Puerto ESP-IDF/I2C para SSD1306 y actualizacion de pantalla. |
| `status_led` | LED integrado con patrones no bloqueantes. |
| `board_feedback` | Estados de alto nivel y pantallas para nodo. |

La estructura replica la separacion conceptual tomada como referencia desde
`PCSE_Nivel_Digital`: driver/puerto, graficos y servicio de pantalla, adaptado a
ESP-IDF.

## Pines usados

| Funcion | GPIO |
|---|---:|
| OLED SDA | 17 |
| OLED SCL | 18 |
| OLED reset | 21 |
| LED integrado | 35 |
| VEXT control | 36 |

VEXT se configura en nivel bajo para habilitar alimentacion de perifericos de la
placa, segun el comportamiento documentado por Heltec para placas con VEXT.

## API publica

```c
esp_err_t board_feedback_init(void);
esp_err_t board_feedback_set_state(board_state_t state);
esp_err_t board_feedback_show_status(const board_status_t *status);
esp_err_t board_feedback_show_message(const char *line1, const char *line2, const char *line3);
esp_err_t board_feedback_tick(void);
```

Estados soportados:

```c
BOARD_STATE_BOOT
BOARD_STATE_IDLE
BOARD_STATE_TX
BOARD_STATE_RX
BOARD_STATE_FORWARDING
BOARD_STATE_ERROR
BOARD_STATE_TEST_RUNNING
```

## Demo actual

`app_main()` inicializa la placa y arranca `app_runtime`. Desde ahi, `app_task`
cicla estados simulados y los envia por cola a `feedback_task`. `radio_task` y
`protocol_task` existen como stubs FreeRTOS, pero no inicializan SX1262 ni
protocolo en esta fase:

```text
BOOT -> IDLE -> TX -> RX -> FWD -> ERROR -> TEST
```

La primera linea de la OLED identifica el modulo como `ESP32 + SX1262`, version
abreviada para la pantalla 128x64 del nombre completo del trabajo:
`Modulo de comunicacion LoRa/SPI para ESP32-S3 + SX1262`.

La OLED muestra nombre del proyecto, estado, rol y direccion. En estado `TEST`
muestra contadores simulados. El LED cambia de patron segun el estado.

Validacion realizada en placa:

```text
I (...) SIMAI_MESH: Phase: Fase 3.5 - drivers de placa y feedback local
I (...) APP_RUNTIME: radio_task stub started; SX1262 is not initialized yet
I (...) APP_RUNTIME: protocol_task stub started; mesh protocol is not initialized yet
I (...) APP_RUNTIME: app_task demo started
I (...) APP_RUNTIME: FreeRTOS runtime started with shared event queue
I (...) SIMAI_MESH: app_main completed initialization; FreeRTOS tasks are running
I (...) OLED_DISPLAY: OLED initialized at 0x3C
I (...) APP_RUNTIME: Board feedback task started
I (...) APP_RUNTIME: Feedback state: 0
I (...) APP_RUNTIME: Feedback state: 1
I (...) APP_RUNTIME: Feedback state: 2
I (...) APP_RUNTIME: Feedback state: 3
```

## Limitaciones

- La fuente de texto es minima y propia; cubre letras mayusculas, numeros y
  simbolos usados en pantallas de estado.
- Los patrones LED son no bloqueantes y dependen de llamadas periodicas a
  `board_feedback_tick()` desde la tarea FreeRTOS de feedback.
- La validacion de radio SX1262 queda para la Fase 4.
