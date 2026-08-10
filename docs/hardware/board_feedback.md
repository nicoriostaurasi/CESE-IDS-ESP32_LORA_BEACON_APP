# Board feedback

## Alcance

La demo IDS implementa feedback local en la placa Heltec WiFi LoRa 32 V3 mediante:

- OLED integrada 128x64 por I2C.
- LED digital integrado en GPIO35.
- API `board_feedback` para que capas superiores no dibujen ni controlen GPIO
  directamente.

El componente recibe estados desde `app_runtime`, pero no accede directamente
al SX1262 ni interpreta el protocolo beacon. Tampoco controla sensores externos.

## Componentes

| Componente | Responsabilidad |
|---|---|
| `display_graphics` | Framebuffer 128x64, texto basico y primitivas simples. |
| `oled_display` | Puerto ESP-IDF/I2C para SSD1306 y actualizacion de pantalla. |
| `status_led` | Pulso digital no bloqueante asociado a una recepción válida. |
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

`app_main()` inicializa la placa y arranca `app_runtime`. La tarea de radio
transmite un beacon cada tres segundos y permanece en recepción entre envíos.
Los eventos confirmados se envían por cola a `feedback_task`, por lo que el
control del GPIO del LED no bloquea las operaciones del SX1262.

El LED permanece apagado cuando no ocurre una recepción válida:

| Evento | Estado | Duración |
|---|---|---:|
| Inicialización | Apagado | — |
| TX exitoso | Apagado | — |
| RX válido | Encendido | 1 s |
| Error de inicialización | Apagado | — |

No se genera un pulso por el solo hecho de entrar en modo recepción: el GPIO se
activa únicamente después de recibir y validar una trama del otro nodo. El
pulso usa `gpio_set_level()` explícitamente para encender y apagar; no depende
de una operación toggle ni del estado anterior.

La primera linea de la OLED identifica el modulo como `ESP32 + SX1262`, version
abreviada para la pantalla 128x64 del nombre completo del trabajo:
`Modulo de comunicacion LoRa/SPI para ESP32-S3 + SX1262`.

La OLED muestra el identificador del nodo, el último TX/RX, el intervalo entre
recepciones y el RSSI del último paquete válido.

## Limitaciones

- La fuente de texto es minima y propia; cubre letras mayusculas, numeros y
  simbolos usados en pantallas de estado.
- Los pulsos LED son no bloqueantes y dependen de llamadas periódicas a
  `board_feedback_tick()` desde la tarea FreeRTOS de feedback.
- El log `STATUS_LED` informa el inicio y la duración real medida del pulso RX.
