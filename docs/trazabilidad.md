# Matriz de trazabilidad IDS

| Requisito del anteproyecto | Implementación | Verificación | Estado |
|---|---|---|---|
| Inicializar SPI y GPIO | `sx1262_radio_init(hw, cfg)` e `_init_gpio_and_spi()` | Build ESP-IDF + log `SX1262 initialized` | Implementado |
| Configurar frecuencia, SF, BW, CR, preámbulo y potencia | `sx1262_radio_set_config()` y `sx1262_params` | Test de parámetros + log | Implementado |
| TX con timeout | `sx1262_radio_send()` | Beacon, contrato API y timeout cero controlado | Implementado |
| RX con timeout | `sx1262_radio_receive()` | Beacon, contrato API y timeout cero controlado | Implementado |
| RSSI y SNR | `sx1262_radio_get_last_packet_metrics()` | Log RX, OLED y reporte de 100 muestras | Implementado |
| Encapsular comandos SPI | `sx1262_radio` sobre `sx1262_transport` | Test de tramas SPI | Implementado |
| Errores tipados | Alias `RADIO_ERR_*` y `esp_err_t` | Tests API/transporte + self-test | Implementado |
| Modularidad y bajo acoplamiento | `radio_hw_config_t`; driver independiente de la placa | Build y API pública | Implementado |
| Concurrencia robusta | Mutex por operación completa + mutex SPI | Diseño y build FreeRTOS | Implementado |
| Observabilidad | Logs, OLED, pérdidas, PDR, RSSI/SNR y DT | Reporte `ACCEPTANCE` | Implementado |
| Testabilidad | Componentes puros y callbacks HAL | `test_all_host.sh` | Implementado |
| Demo entre dos placas | Beacons 0/1 cada 3 s | `monitor_node_0.log`: 116 tramas, 0 pérdidas, 100 % entrega | Aprobado en un extremo |

La evidencia actual prueba el enlace desde un extremo. Para caracterización
bidireccional completa se puede repetir el mismo procedimiento guardando
`monitor_node_1.log` desde la otra placa.
