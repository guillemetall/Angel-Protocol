[Espanol](README.es.md) | **English**

# Xiaomesh

**Offline mesh messaging firmware for the Seeed Studio XIAO ESP32-C6.**

Xiaomesh turns a small, low-power ESP32-C6 board into a node of a decentralized messaging network that works without internet, cell towers, Wi-Fi, or LoRa. It uses Bluetooth LE Coded PHY S8 to reach distances of 200-500 m per hop, and chains multiple nodes together to cover kilometers.

Designed for emergencies, remote areas, hiking, sailing, festivals, and any situation where the usual networks fail.

## Highlights

- **Extended range**: Coded PHY S8 for up to 500 m per hop.
- **Works offline**: no internet, no cell towers, no infrastructure.
- **Low power**: solar + LiPo friendly, with deep sleep support.
- **End-to-end encryption**: AES-128-GCM for all messages.
- **Self-organizing mesh**: hybrid routing with composite node scoring.
- **Works with any phone**: Android, iOS, and PC via BLE GATT.
- **Open source**: GPLv3 licensed.

<details>
<summary><b>Full description</b></summary>

### What Xiaomesh is

Xiaomesh is a firmware for the Seeed Studio XIAO ESP32-C6 that converts the board into a self-contained mesh node. Each node can act as a repeater, a bridge for phone clients, or a mailbox for disconnected users.

### How the mesh works

Nodes discover each other through Bluetooth Low Energy advertisements. When a message is sent, the originating node uses a flooding mechanism to find a route to the destination. Once the route is learned, subsequent messages travel directly along it. Each node evaluates its neighbors using a composite score (RSSI, packet delivery ratio, battery level, hop count) and picks the most reliable next hop.

### Adaptive radio

The firmware automatically switches between Bluetooth LE PHYs (1M, 2M, and Coded PHY S8) based on link quality. S8 is used only when needed, preserving battery on short links. Long links get the extended range they require.

### Security

All messages are encrypted with AES-128-GCM. Configuration and OTA are gated behind BLE pairing (passkey + MITM protection) and, for OTA, a physical button. No unauthorized device can reconfigure a node.

### Compatible devices

- **Android**: any device with BLE 5.0 and API 26+.
- **iOS**: via GATT bridge (CoreBluetooth limitation).
- **PC**: BLE adapter with Coded PHY support, or GATT bridge.
- **Other XIAO ESP32-C6**: full mesh node.

### Use cases

- Emergency communications when infrastructure fails.
- Remote areas without cellular coverage.
- Hiking, sailing, and outdoor activities.
- Festivals and large events where cell networks collapse.
- Community mesh networks.

</details>

## Requirements

| Component | Minimum |
|-----------|---------|
| Board | Seeed Studio XIAO ESP32-C6 |
| Antenna | External U.FL 2.4 GHz |
| Power | USB-C or LiPo + solar panel |
| Framework | ESP-IDF 5.3 |

## Quick start

### 1. Build without a PC

Firmware is compiled automatically by GitHub Actions when you push to the repository.

- Push to `main`.
- Wait 3-5 minutes in the **Actions** tab.
- Download the artifact `xiaomesh-firmware.zip`.

### 2. Flash

Using **ESP32_Flasher** (Android) or **esptool.py** (PC):

| File | Offset |
|------|--------|
| `bootloader.bin` | `0x0` |
| `partition-table.bin` | `0x8000` |
| `xiaomesh.bin` | `0x10000` |

### 3. Configure

Connect to `XIAO-MESH` from **nRF Connect** (or any compatible app).

- Service UUID: `0x00FF`
- Config: `0xFF01`
- Messages: `0xFF02`
- OTA: `0xFF03`
- Status: `0xFF04`

The first time you power up, note the **passkey** shown on the serial monitor. You will need it to pair.

### 4. LED status

| Pattern | Meaning |
|---------|---------|
| Fast blink (5 Hz) | Booting |
| Slow blink (1 Hz) | Searching for mesh |
| Solid on | Connected |
| Double blink | Relaying |
| Triple blink | Low battery |

<details>
<summary><b>Detailed configuration</b></summary>

### Configuration commands

Write to `0xFF01`:

| Bytes | Action |
|-------|--------|
| `[0x01, v]` | PHY: 0=auto, 1=1M, 2=2M, 3=S8 |
| `[0x02, v]` | Mode: 0=repeater, 1=bridge, 2=room |
| `[0x03, v]` | TX power: -12 to +20 dBm |
| `[0x04, 16B]` | Custom AES-128 key |

### Sending messages

Write to `0xFF02`:


Use `[0xFF,0xFF,0xFF,0xFF]` for broadcast.

### UART commands

Serial at 115200 baud:


### Recovery mode

Hold GPIO9 to GND at boot. The node creates a Wi-Fi AP:

- SSID: `XIAO-MESH-OTA`
- Password: `mesh1234`
- URL: `http://192.168.4.1/ota`

Upload the firmware and the node reboots.

</details>

## Hardware

### Wiring

| GPIO | Function |
|------|----------|
| GPIO15 | Status LED |
| GPIO3 | Antenna switch (LOW = enabled) |
| GPIO14 | Antenna select (HIGH = external) |
| GPIO9 | Recovery / OTA button |
| GPIO0 | Battery ADC |
| GPIO1 | Solar panel ADC |
| GPIO2 | Wakeup GPIO |

### Recommended setup

- XIAO ESP32-C6 + external U.FL antenna.
- LiPo 2000-5000 mAh + 5-10 W solar panel.
- Weatherproof enclosure for outdoor nodes.
- High mounting point for backbone nodes.

## Status

**Current version: v5.0.0**

| Feature | State |
|---------|-------|
| Coded PHY S8 | Done |
| Adaptive PHY switching | Done |
| Hybrid mesh routing | Done |
| AES-128-GCM encryption | Done |
| BLE pairing with MITM | Done |
| OTA with button gating | Done |
| Multi-connection GATT | Done |
| OLED display | Planned |
| Flutter client app | Planned |

## License

GNU General Public License v3.0 or later. See [LICENSE](LICENSE).
