# Pinout Heltec WiFi LoRa 32 V3

## Alcance

Este documento corresponde a la Fase 3 del plan de trabajo. Su objetivo es
documentar la asignacion de pines internos de la Heltec WiFi LoRa 32 V3 y dejar
separados los pines reservados de los GPIO recomendados para sensores externos.

En esta fase no se implementan drivers funcionales ni se inicializan OLED, radio,
LED o botones. La validacion funcional de OLED/LED queda para la Fase 3.5.

## Fuentes

- Heltec WiFi LoRa 32 V3: https://heltec.org/project/wifi-lora-32-v3/
- Documentacion Heltec WiFi LoRa 32: https://docs.heltec.org/en/node/esp32/wifi_lora_32/index.html
- Guia GPIO Heltec V3: https://github.com/HelTecAutomation/HeltecWiKi/blob/main/docs/devices/open-source-hardware/esp32-series/lora-32/wifi-lora-32-v3/Pin-diagram-guidance.md
- Variante oficial Heltec V3 `pins_arduino.h`: https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series/blob/master/variants/heltec_wifi_lora_32_V3/pins_arduino.h

La variante Arduino se usa solo como referencia de pinout y nombres de placa. El
firmware final se mantiene en ESP-IDF.

## Pines internos de placa

| Funcion | GPIO | Uso | Estado |
|---|---:|---|---|
| OLED SDA | 17 | I2C OLED integrada | Reservado interno |
| OLED SCL | 18 | I2C OLED integrada | Reservado interno |
| OLED reset | 21 | Reset OLED | Reservado interno |
| SX1262 NSS | 8 | SPI CS radio | Reservado interno |
| SX1262 SCLK | 9 | SPI clock radio | Reservado interno |
| SX1262 MOSI | 10 | SPI MOSI radio | Reservado interno |
| SX1262 MISO | 11 | SPI MISO radio | Reservado interno |
| SX1262 reset | 12 | Reset radio | Reservado interno |
| SX1262 busy | 13 | Busy radio | Reservado interno |
| SX1262 DIO1/IRQ | 14 | IRQ radio; nombrado `DIO0` en la variante Heltec | Reservado interno |
| LED integrado | 35 | LED de estado de placa | Reservado interno |
| VEXT control | 36 | Control de alimentacion externa/perifericos | Reservado interno |
| USB UART TX | 43 | Consola/descarga USB serial | Reservado sistema |
| USB UART RX | 44 | Consola/descarga USB serial | Reservado sistema |

## Pines recomendados para sensores externos

Segun la guia GPIO de Heltec V3, los siguientes pines no estan ocupados por
perifericos internos y son recomendados para hardware externo:

| GPIO | Funciones alternativas relevantes | Uso recomendado |
|---:|---|---|
| 1 | ADC1_CH0, TOUCH1 | Sensor externo / GPIO |
| 2 | ADC1_CH1, TOUCH2 | Sensor externo / GPIO |
| 4 | ADC1_CH3, TOUCH4 | Sensor externo / GPIO |
| 5 | ADC1_CH4, TOUCH5 | Sensor externo / GPIO |
| 6 | ADC1_CH5, TOUCH6 | Sensor externo / GPIO |
| 7 | ADC1_CH6, TOUCH7 | Sensor externo / GPIO |
| 19 | ADC2_CH9, USB_D- alternativo | Sensor externo / GPIO si no se usa USB nativo |
| 20 | ADC2_CH9, USB_D+ alternativo | Sensor externo / GPIO si no se usa USB nativo |
| 47 | GPIO | Sensor externo / GPIO |
| 48 | GPIO | Sensor externo / GPIO |

## Pines reservados o no recomendados

| GPIO | Motivo | Recomendacion |
|---:|---|---|
| 8-14 | Radio SX1262 | No usar para sensores |
| 17, 18, 21 | OLED integrada | No usar para sensores |
| 26 | SubSPI chip-select interno | No usar |
| 33-38 | SPI Flash/SubSPI | No usar |
| 35 | LED integrado | Usar solo desde `status_led` |
| 36 | VEXT | Usar solo desde soporte de placa |
| 39-42 | JTAG por defecto | Evitar durante desarrollo |
| 43-44 | USB serial/download | No reutilizar |
| 46 | Strapping sensible | No usar para hardware externo |

## Notas de implementacion

- Todos los pines se centralizan en `components/board_config`.
- Los componentes futuros no deben hardcodear GPIO.
- OLED, LED, VEXT y radio se inicializaran en fases posteriores.
- Antes de conectar sensores externos, revisar nuevamente el esquematico de la
  revision fisica de placa usada.
- La medicion formal de bateria/autonomia queda fuera de alcance del proyecto.
