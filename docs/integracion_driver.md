# Integración del driver SX1262

Esta aplicación de demostración IDS separa deliberadamente dos responsabilidades:

- La aplicación beacon administra tareas FreeRTOS, tramas de prueba, métricas,
  señalización local y presentación en la OLED.
- El submódulo `components/sx1262_radio` administra GPIO, SPI, comandos SX1262,
  configuración LoRa y operaciones de transmisión y recepción.

El submódulo apunta a:

<https://github.com/nicoriostaurasi/ESP32-S3-SX1262-DRIVER>

## Dependencia ESP-IDF

`components/app_runtime/CMakeLists.txt` declara `sx1262_radio` en `REQUIRES`.
Por ello, `app_runtime` puede incluir `sx1262_radio.h`, mientras que las
cabeceras ubicadas en `private_include/` permanecen reservadas al driver.

## Actualización del módulo

```bash
git submodule update --remote components/sx1262_radio
git add components/sx1262_radio
```

El segundo comando registra en esta aplicación el nuevo commit exacto del
driver. Esto permite reproducir una demostración aun cuando el repositorio del
driver continúe evolucionando.

## Alcance de la demo

Los nodos 0 y 1 transmiten beacons independientes cada tres segundos. El
objetivo es observar envío, recepción, RSSI, intervalo entre paquetes y
estabilidad del enlace. No se implementan confirmaciones, retransmisiones,
enrutamiento mesh ni lógica de producto.
