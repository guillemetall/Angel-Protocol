[English](README.md) | **Espanol**

# Xiaomesh - Firmware Mesh BLE Coded PHY S8

Firmware para Seeed Studio XIAO ESP32-C6 que actua como nodo de una red de mensajeria mesh offline. Usa Bluetooth LE Coded PHY S8 para alcance extendido (200-500 m por salto) sin depender de internet, torres celulares, Wi-Fi ni LoRa.

Copyright (C) 2026 guillemetall
Licencia: GNU General Public License v3.0 o posterior. Ver archivo LICENSE.

## Caracteristicas

- Coded PHY S8 para alcance extendido (200-500 m por salto).
- Extended Advertising dual: instancia conectable en 1M PHY (telefonos) y no conectable en Coded PHY (malla).
- Extended Scanning dual: escucha simultanea en 1M y Coded PHY S8.
- Enrutamiento hibrido: flooding para descubrir rutas, directo para comunicar.
- Score compuesto (RSSI 40%, PDR 30%, bateria 10%, saltos 20%).
- Conmutacion adaptativa de PHY (1M / 2M / S8) con histeresis y contadores.
- Cifrado AES-128-GCM de todos los payloads con mbedTLS.
- Fragmentacion y reensamblado de mensajes largos (hasta 8 fragmentos).
- Deduplicacion FNV-1a con cache de 32 hashes.
- ACKs acumulativos con bitmap de 32 bits.
- Batching de mensajes cortos.
- Room server (buzon de mensajes para nodos desconectados).
- Bridge para iPhone, Android antiguo o PC sin adaptador BLE 5.0.
- Control de congestion por airtime medido.
- Rotacion de IDs cada 10 minutos para privacidad.
- OTA con verificacion SHA-256 (por BLE o Wi-Fi).
- Modo recuperacion por GPIO para rescatar la placa sin PC.
- Gestion solar y bateria con deep sleep por timer o GPIO.

## Hardware

| Componente | Modelo |
|------------|--------|
| Placa principal | Seeed Studio XIAO ESP32-C6 |
| Antena | Externa U.FL 2.4 GHz (2.8 dBi o mayor) |
| Energia | Panel solar 5-10 W + bateria LiPo 2000-5000 mAh |
| Carga | Controlador TP4056 o similar |
| Conexion | Cable USB-C para programacion |

### Pines usados

| GPIO | Funcion |
|------|---------|
| GPIO15 | LED azul de estado |
| GPIO3 | Conmutador de antena (LOW = habilitado) |
| GPIO14 | Seleccion de antena (HIGH = externa U.FL) |
| GPIO9 | Modo recuperacion (LOW al arrancar) |
| GPIO0 | ADC bateria |
| GPIO1 | ADC panel solar |
| GPIO2 | Wakeup GPIO desde deep sleep |

## Estructura del proyecto

main/config.h - Constantes y configuracion
main/app_main.c - Punto de entrada y bucle principal
main/antenna.c - Seleccion de antena U.FL
main/power_manager.c - ADC bateria/panel + deep sleep
main/ota_manager.c - OTA por BLE/WiFi + modo recuperacion
main/ble_manager.c - Advertising y scanning dual (1M + S8)
main/gatt_server.c - Servidor GATT (config, mensajes, OTA, status)
main/protocol.c - Formato de paquete + enrutamiento + tabla vecinos
main/phy_manager.c - Conmutacion adaptativa 1M / 2M / S8
main/fragment.c - Fragmentacion y reensamblado
main/dedup.c - Deduplicacion FNV-1a
main/ack_manager.c - ACKs acumulativos
main/batch.c - Batching de mensajes cortos
main/congestion.c - Control de airtime
main/room.c - Room server (buzon)
main/crypto.c - Cifrado AES-128-GCM
main/bridge.c - Bridge 1M <-> S8 para clientes
main/id_rotation.c - Rotacion de IDs cada 10 minutos
main/uart_config.c - Comandos por UART

## Compilar sin PC (GitHub Actions)

El firmware se compila automaticamente en la nube usando GitHub Actions.

1. Sube este repositorio a GitHub.
2. Cada push a main dispara un build en la pestana Actions.
3. Espera 3-5 minutos al check verde.
4. Entra al build completado, baja hasta Artifacts.
5. Descarga xiaomesh-firmware.zip.

Contiene: bootloader.bin, partition-table.bin, xiaomesh.bin

## Flashear desde Android

1. Instala ESP32_Flasher desde F-Droid o Play Store.
2. Conecta la XIAO ESP32-C6 al telefono por cable OTG USB-C.
3. Flashea en este orden:

| Archivo | Offset |
|---------|--------|
| bootloader.bin | 0x0 |
| partition-table.bin | 0x8000 |
| xiaomesh.bin | 0x10000 |

4. Reinicia la placa desconectando y reconectando el cable.

## Configuracion por BLE (nRF Connect)

Instala nRF Connect for Mobile. Escanea y busca XIAO-MESH.

Servicio UUID: 0x00FF

| Caracteristica | UUID | Permisos | Uso |
|----------------|------|----------|-----|
| Config | 0xFF01 | Read + Write | Configurar nodo |
| Mensajes | 0xFF02 | Write + Notify | Enviar y recibir |
| OTA | 0xFF03 | Write + Notify | Actualizar firmware |
| Status | 0xFF04 | Read + Notify | Leer estado |

### Comandos de configuracion (escribir en 0xFF01)

- [0x01, valor] - Modo PHY: 0x00=Auto, 0x01=1M, 0x02=2M, 0x03=S8
- [0x02, valor] - Modo nodo: 0x00=Repeater, 0x01=Bridge, 0x02=Room server
- [0x03, valor] - TX power: -12 a +20 dBm
- [0x04, 16 bytes] - Clave AES-128 personalizada

### Enviar mensaje (escribir en 0xFF02)

Formato: [texto del mensaje][4 bytes de destino]

Si no conoces el destino, usa [0xFF, 0xFF, 0xFF, 0xFF] para broadcast.

### Leer estado (notificacion de 0xFF04)

10 bytes: [bateria%, PHY, panel_lo, panel_hi, congestion, ...]

## Configuracion por UART

Conecta la placa por USB y abre terminal serie a 115200 baud.

Comandos:
- set phy auto|1m|2m|s8
- set mode repeater|bridge|room
- status

## LED de estado (GPIO15)

| Patron | Significado |
|--------|-------------|
| Parpadeo muy rapido (5 Hz) | Arrancando |
| Parpadeo lento (1 Hz) | Buscando malla |
| Encendido fijo | Conectado a la malla |
| Doble parpadeo | Retransmitiendo |
| Triple parpadeo | Bateria baja |

## Modo recuperacion

1. Manten GPIO9 a GND al arrancar la placa.
2. El nodo crea un AP Wi-Fi:
   - SSID: XIAO-MESH-OTA
   - Password: mesh1234
3. Abre http://192.168.4.1/ota en el navegador.
4. Sube el .bin del firmware.
5. El nodo se reinicia con la nueva version.

La placa NO se bloquea en ningun momento (sin Secure Boot, sin Flash Encryption). Siempre se puede reflashear por USB.

## Roles del nodo

| Rol | Descripcion |
|-----|-------------|
| Repeater | Retransmite paquetes de la malla. Sin clientes GATT. |
| Bridge | Repeater + acepta clientes GATT (iPhone, Android, PC). |
| Room server | Almacena mensajes para nodos desconectados (buzon). |

El modo bridge se activa automaticamente cuando un cliente GATT se conecta.

## Protocolo (resumen)

### Formato de paquete (max 27 bytes en S8)

[header: 1B][msg_id: 1B][nonce: 12B][path_len: 1B][path: 0-64B][payload_len: 1B][payload: 0-24B]

### Tipos de ruta

- FLOOD: inunda la red para descubrir ruta.
- DIRECT: usa ruta conocida hacia el destino.

### Tipos de payload

- TXT: mensaje de texto.
- ACK: confirmacion individual.
- ACK_CUM: confirmacion acumulativa (bitmap de 32 bits).
- ADVERT: beacon con bateria.
- FRAG: fragmento de mensaje largo.
- BATCH: multiples mensajes cortos agrupados.
- SOS: mensaje de emergencia (fuerza S8).

## Estado del proyecto

| Fase | Contenido | Estado |
|------|-----------|--------|
| 1 | Esqueleto + gestion de energia | Completada |
| 2 | BLE + GATT echo | Completada |
| 3 | Coded PHY S8 + Extended Advertising | Completada |
| 4 | Protocolo mesh completo | Completada |
| 5 | Q-Learning ligero para PHY adaptativo | Pendiente |
| 6 | Pantalla OLED + app cliente Flutter | Pendiente |

## Licencia

GNU General Public License v3.0 o posterior. Ver archivo LICENSE.

Este programa es software libre. Puedes redistribuirlo y/o modificarlo bajo los terminos de la GPLv3.

## Creditos

- Inspirado en Meshtastic, MeshCore y Knit.
- Construido sobre ESP-IDF 5.3 de Espressif.
- Cifrado con mbedTLS.
- Desarrollado en Termux desde Android.
