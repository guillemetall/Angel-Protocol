# 📡 Angel Protocol

**Offline mesh messaging firmware for the Seeed Studio XIAO ESP32-C6.**

Angel Protocol turns a low-power ESP32-C6 board into a node of a decentralized messaging network that works without internet, cell towers, Wi-Fi, or LoRa. Uses Bluetooth LE Coded PHY S8 for 200-500 m per hop, and chains nodes to cover kilometers.

## ✨ Highlights

- 📻 Coded PHY S8 (200-500 m per hop) + adaptive 1M/2M/S8
- 🛰️ Works offline: no internet, no infrastructure
- 🕸️ Self-organizing mesh with composite scoring
- 🔋 Solar + LiPo friendly, deep sleep support
- 🔐 AES-128-GCM encryption + BLE pairing with button
- 📨 Messages up to 640 bytes (32 fragments)
- 📱 Works with Android, iOS and PC via BLE GATT
- 🆓 GPLv3

## 🚀 Install

### Web installer (Chrome/Edge desktop)
**[▶ guillemetall.github.io/Angel-Protocol](https://guillemetall.github.io/Angel-Protocol/)**

### Android
1. Install ESP32_Flasher from Play Store
2. Connect via USB-C OTG
3. Flash: bootloader.bin (0x0), partition-table.bin (0x8000), angel.bin (0x10000)

### Command line
esptool.py --chip esp32c6 write_flash 0x0 bootloader.bin 0x8000 partition-table.bin 0x10000 angel.bin

## 🔗 Pair

1. Open nRF Connect, scan for XIAO-MESH
2. Enter passkey 123456
3. Press GPIO9 briefly to confirm

## 🔐 Security

- AES-128-GCM encryption
- BLE pairing (passkey + MITM)
- Physical button confirmation
- Admin key in NVS
- OTA gated by button (3 s) + SHA-256
- ID rotation every 10 min
- No Secure Boot / Flash Encryption (fully reflashable)

## 🕸️ Mesh

Flooding for discovery, direct routing after. Composite neighbor score:

| Weight | Metric |
|--------|--------|
| 40% | RSSI |
| 30% | PDR |
| 10% | Battery |
| 20% | Hops |

Routes expire in 5 min, neighbors in 30 s.

## 📻 Radio

| PHY | Range | When |
|-----|-------|------|
| 1M | ~100 m | Default |
| 2M | ~50 m | Close neighbors |
| S8 | 200-500 m | Long/weak links |

## 📨 Messaging

Fragments up to 32 (640 B), cumulative ACKs, FNV-1a dedup, batching, room server (72 h), delivery notifications.

## 🚦 Anti-congestion

1. Deduplication
2. TTL (7 hops) + random delay
3. Token bucket (10 frag/s per node)
4. Priority queues with SOS interruption

## 🛠️ Hardware

Seeed Studio XIAO ESP32-C6 + U.FL antenna + LiPo/solar.

Pinout: LED (GPIO15), antenna (GPIO3/14), button (GPIO9), battery ADC (GPIO0), panel ADC (GPIO1).

## ⚙️ Config

Service UUID: 0x00FF

| Char | UUID | Use |
|------|------|-----|
| Config | 0xFF01 | Read/Write (auth) |
| Messages | 0xFF02 | Write/Notify |
| OTA | 0xFF03 | Write (button) |
| Status | 0xFF04 | Read/Notify |

Commands (write to 0xFF01):

| Bytes | Action |
|-------|--------|
| [0x01, v] | PHY: 0=auto, 1=1M, 2=2M, 3=S8 |
| [0x02, v] | Mode: 0=repeater, 1=bridge, 2=room |
| [0x03, v] | TX power: -12 to +20 dBm |
| [0x04, 16B] | AES-128 key |
| [0x05, 4B] | Request room messages |

Send (0xFF02): [text][4B destination]

Notifications (0xFF02): [0x01, id]=received, [0x02, id]=delivered, [text]=incoming.

UART: set phy auto|1m|2m|s8 / set mode repeater|bridge|room / set passkey / status / save.

Recovery: hold GPIO9 to GND. AP XIAO-MESH-OTA / mesh1234.

## 📶 LED

Fast blink=booting, slow blink=searching, solid=connected, double=relaying, triple=low battery.

## 📋 Status

**Under active development. Not yet field-tested.**

### ✅ Implemented
Coded PHY S8, adaptive PHY, dual advertising/scanning, hybrid mesh, composite scoring, AES-128-GCM, BLE pairing + button, admin key, OTA + SHA-256, ID rotation, fragmentation 640 B, cumulative ACKs, dedup, batching, room server with delivery, client ACK notifications, low priority queue + SOS, token bucket, multi-GATT (8), bridge default, solar/battery, deep sleep, UART, recovery, web installer.

### 🚧 In progress
Sliding window, adaptive thresholds, Q-Learning, backpressure.

### 🔜 Planned
Flutter app, GPS, sensors, audio (Codec2), image (AVIF), OLED.

## 📜 License

GNU General Public License v3.0 or later. See LICENSE.
