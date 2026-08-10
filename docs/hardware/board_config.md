# Board config

`board_config` centraliza la informacion basica de placa y nodo para evitar que
`main` o futuros componentes dupliquen datos de hardware.

## Plataforma

- Placa: Heltec WiFi LoRa 32 V3.
- Variante LoRa: 433 MHz.
- MCU: ESP32-S3.
- Radio: SX1262.
- USB-UART: CP2102.
- OLED: SSD1306-compatible 128x64.

## Configuracion inicial de nodo

- Rol por defecto: `NODE_ROLE_SENSOR`.
- Direccion logica por defecto: `1`.

Roles disponibles:

```c
NODE_ROLE_SENSOR
NODE_ROLE_BRIDGE
NODE_ROLE_CONCENTRATOR
```

## Pines

Los pines se declaran simbolicamente en `board_config.h`. La Fase 3 completa la
tabla de pines desde referencias oficiales de Heltec, pero no inicializa
perifericos todavia.

## Pines internos documentados

| Funcion | GPIO | Nota |
|---|---:|---|
| OLED SDA | 17 | I2C OLED integrada |
| OLED SCL | 18 | I2C OLED integrada |
| OLED reset | 21 | Reservado por OLED |
| SX1262 MOSI | 10 | SPI radio |
| SX1262 MISO | 11 | SPI radio |
| SX1262 SCLK | 9 | SPI radio |
| SX1262 NSS | 8 | Chip select radio |
| SX1262 reset | 12 | Reset radio |
| SX1262 busy | 13 | Busy radio |
| SX1262 DIO1/IRQ | 14 | Definido como `DIO0` en la variante Arduino Heltec |
| LED integrado | 35 | LED de placa |
| VEXT control | 36 | Control de alimentacion externa/perifericos |
| USB UART TX | 43 | USB serial/download |
| USB UART RX | 44 | USB serial/download |

## Pines seguros para sensores externos

Segun la guia GPIO de Heltec V3, los GPIO recomendados para hardware externo son:

```text
GPIO1, GPIO2, GPIO4, GPIO5, GPIO6, GPIO7, GPIO19, GPIO20, GPIO47, GPIO48
```

## Pines reservados o no recomendados

- GPIO8 a GPIO14: radio SX1262.
- GPIO17, GPIO18 y GPIO21: OLED integrada.
- GPIO35: LED integrado.
- GPIO36: VEXT.
- GPIO43 y GPIO44: USB serial/download.
- GPIO33 a GPIO38: SPI Flash/SubSPI.
- GPIO26: SubSPI chip-select interno.
- GPIO39 a GPIO42: JTAG por defecto; no recomendados durante desarrollo.
- Pines de strapping, especialmente GPIO46, no recomendados para hardware
  externo.

## Logs esperados

El firmware imprime al arranque:

- Nombre de firmware.
- Nombre de proyecto.
- Version de aplicacion.
- Version de ESP-IDF.
- Plataforma objetivo.
- Rol del nodo.
- Direccion logica.
