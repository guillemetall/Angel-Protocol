**English** | [Espanol](README.es.md)

# Xiaomesh - Mesh Firmware BLE Coded PHY S8

Firmware for **Seeed Studio XIAO ESP32-C6** that acts as a node in an offline mesh messaging network. Uses **Bluetooth LE Coded PHY S8** for extended range (200-500 m per hop) without depending on internet, cell towers, Wi-Fi, or LoRa.

Copyright (C) 2026 guillemetall
License: GNU General Public License v3.0 or later. See LICENSE file.

## Features

- Coded PHY S8 for extended range (200-500 m per hop).
- Dual Extended Advertising: connectable instance on 1M PHY (phones) and non-connectable on Coded PHY (mesh).
- Dual Extended Scanning: simultaneous listening on 1M and Coded PHY S8.
- Hybrid routing: flooding for route discovery, direct for communication.
- Composite score (RSSI 40%, PDR 30%, battery 10%, hops 20%).
- Adaptive PHY switching (1M / 2M / S8) with hysteresis and counters.
- AES-128-GCM encryption of all payloads with mbedTLS.
- Fragmentation and reassembly of long messages (up to 8 fragments).
- FNV-1a deduplication with 32-hash cache.
- Cumulative ACKs with 32-bit bitmap.
- Batching of short messages.
- Room server (mailbox for disconnected nodes).
- Bridge for iPhone, old Android, or PC without BLE 5.0 adapter.
- Congestion control by measured airtime.
- ID rotation every 10 minutes for privacy.
- OTA with SHA-256 verification (via BLE or Wi-Fi).
- Recovery mode via GPIO to rescue the board without PC.
- Solar and battery management with timer or GPIO deep sleep.

## Hardware

| Component | Model |
|-----------|-------|
| Main board | Seeed Studio XIAO ESP32-C6 |
| Antenna | External U.FL 2.4 GHz (2.8 dBi or higher) |
| Power | Solar panel 5-10 W + LiPo battery 2000-5000 mAh |
| Charging | TP4056 controller or similar |
| Connection | USB-C cable for programming |

### Pins used

| GPIO | Function |
|------|----------|
| GPIO15 | Blue status LED |
| GPIO3 | Antenna switch (LOW = enabled) |
| GPIO14 | Antenna select (HIGH = external U.FL) |
| GPIO9 | Recovery mode (LOW at boot) |
| GPIO0 | Battery ADC |
| GPIO1 | Solar panel ADC |
| GPIO2 | Wakeup GPIO from deep sleep |

## Project structure

main/config.h - Constants and configuration
main/app_main.c - Entry point and main loop
main/antenna.c - U.FL antenna selection
main/power_manager.c - Battery/panel ADC + deep sleep
main/ota_manager.c - OTA via BLE/WiFi + recovery mode
main/ble_manager.c - Dual advertising and scanning (1M + S8)
main/gatt_server.c - GATT server (config, messages, OTA, status)
main/protocol.c - Packet format + routing + neighbor table
main/phy_manager.c - Adaptive switching 1M / 2M / S8
main/fragment.c - Fragmentation and reassembly
main/dedup.c - FNV-1a deduplication
main/ack_manager.c - Cumulative ACKs
main/batch.c - Batching of short messages
main/congestion.c - Airtime control
main/room.c - Room server (mailbox)
main/crypto.c - AES-128-GCM encryption
main/bridge.c - Bridge 1M <-> S8 for clients
main/id_rotation.c - ID rotation every 10 minutes
main/uart_config.c - UART commands

## Build without PC (GitHub Actions)

Firmware is compiled automatically in the cloud using GitHub Actions.

1. Push this repository to GitHub.
2. Every push to main triggers a build in the Actions tab.
3. Wait 3-5 minutes for the green check.
4. Enter the completed build, scroll to Artifacts.
5. Download xiaomesh-firmware.zip.

Contains: bootloader.bin, partition-table.bin, xiaomesh.bin

## Flash from Android

1. Install ESP32_Flasher from F-Droid or Play Store.
2. Connect XIAO ESP32-C6 to phone via USB-C OTG cable.
3. Flash in this order:

| File | Offset |
|------|--------|
| bootloader.bin | 0x0 |
| partition-table.bin | 0x8000 |
| xiaomesh.bin | 0x10000 |

4. Restart the board by disconnecting and reconnecting the cable.

## BLE Configuration (nRF Connect)

Install nRF Connect for Mobile. Scan and look for XIAO-MESH.

Service UUID: 0x00FF

| Characteristic | UUID | Permissions | Use |
|----------------|------|-------------|-----|
| Config | 0xFF01 | Read + Write | Configure node |
| Messages | 0xFF02 | Write + Notify | Send and receive |
| OTA | 0xFF03 | Write + Notify | Update firmware |
| Status | 0xFF04 | Read + Notify | Read status |

### Config commands (write to 0xFF01)

- [0x01, value] - PHY mode: 0x00=Auto, 0x01=1M, 0x02=2M, 0x03=S8
- [0x02, value] - Node mode: 0x00=Repeater, 0x01=Bridge, 0x02=Room server
- [0x03, value] - TX power: -12 to +20 dBm
- [0x04, 16 bytes] - Custom AES-128 key

### Send message (write to 0xFF02)

Format: [message text][4 bytes destination]

If you don't know the destination, use [0xFF, 0xFF, 0xFF, 0xFF] for broadcast.

### Read status (notification from 0xFF04)

10 bytes: [battery%, PHY, panel_lo, panel_hi, congestion, ...]

## UART Configuration

Connect the board via USB and open a serial terminal at 115200 baud.

Commands:
- set phy auto|1m|2m|s8
- set mode repeater|bridge|room
- status

## Status LED (GPIO15)

| Pattern | Meaning |
|---------|---------|
| Very fast blink (5 Hz) | Booting |
| Slow blink (1 Hz) | Searching for mesh |
| Solid on | Connected to mesh |
| Double blink | Relaying |
| Triple blink | Low battery |

## Recovery mode

1. Hold GPIO9 to GND at boot.
2. The node creates a Wi-Fi AP:
   - SSID: XIAO-MESH-OTA
   - Password: mesh1234
3. Open http://192.168.4.1/ota in browser.
4. Upload the .bin firmware file.
5. The node reboots with the new version.

The board is NEVER locked (no Secure Boot, no Flash Encryption). It can always be reflashed via USB.

## Node roles

| Role | Description |
|------|-------------|
| Repeater | Relays mesh packets. No GATT clients. |
| Bridge | Repeater + accepts GATT clients (iPhone, Android, PC). |
| Room server | Stores messages for disconnected nodes (mailbox). |

Bridge mode activates automatically when a GATT client connects.

## Protocol (summary)

### Packet format (max 27 bytes on S8)

[header: 1B][msg_id: 1B][nonce: 12B][path_len: 1B][path: 0-64B][payload_len: 1B][payload: 0-24B]

### Route types

- FLOOD: floods the network to discover a route.
- DIRECT: uses a known route to the destination.

### Payload types

- TXT: text message.
- ACK: individual acknowledgment.
- ACK_CUM: cumulative acknowledgment (32-bit bitmap).
- ADVERT: beacon with battery level.
- FRAG: fragment of a long message.
- BATCH: multiple short messages grouped.
- SOS: emergency message (forces S8).

## Project status

| Phase | Content | Status |
|-------|---------|--------|
| 1 | Skeleton + power management | Complete |
| 2 | BLE + GATT echo | Complete |
| 3 | Coded PHY S8 + Extended Advertising | Complete |
| 4 | Full mesh protocol | Complete |
| 5 | Light Q-Learning for adaptive PHY | Pending |
| 6 | OLED display + Flutter client app | Pending |

## License

GNU General Public License v3.0 or later. See LICENSE file.

This program is free software. You can redistribute and/or modify it under the terms of the GPLv3.

## Credits

- Inspired by the ideas of Meshtastic, MeshCore, and Knit.
- Built on ESP-IDF 5.3 from Espressif.
- Encryption with mbedTLS.
- Developed in Termux from Android.
