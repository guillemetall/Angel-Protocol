# 📡 Xiaomesh

**Offline mesh messaging firmware for the Seeed Studio XIAO ESP32-C6.**

Xiaomesh turns a small, low-power ESP32-C6 board into a node of a decentralized messaging network that works without internet, cell towers, Wi-Fi, or LoRa. It uses Bluetooth LE Coded PHY S8 to reach distances of 200-500 m per hop, and chains multiple nodes together to cover kilometers.

Designed for emergencies, remote areas, hiking, sailing, festivals, and any situation where the usual networks fail.

## ✨ Highlights

- 📻 **Extended range**: Coded PHY S8 for up to 500 m per hop.
- 🛰️ **Works offline**: no internet, no cell towers, no infrastructure.
- 🔋 **Low power**: solar + LiPo friendly, with deep sleep support.
- 🔐 **End-to-end encryption**: AES-128-GCM for all messages.
- 🕸️ **Self-organizing mesh**: hybrid routing with composite node scoring.
- 📱 **Works with any phone**: Android, iOS, and PC via BLE GATT.
- 🆓 **Open source**: GPLv3 licensed.

## 🕸️ How the mesh works

<details>
<summary><b>Click to expand</b></summary>

Nodes discover each other through Bluetooth Low Energy advertisements. When a message is sent, the originating node uses a **flooding mechanism** to find a route to the destination. Once the route is learned, subsequent messages travel **directly** along it.

Each node evaluates its neighbors using a **composite score**:

| Weight | Metric |
|--------|--------|
| 40% | RSSI (signal strength) |
| 30% | PDR (packet delivery ratio) |
| 10% | Battery level |
| 20% | Hop count |

The most reliable next hop is chosen at every step. If a link degrades, the score drops and another neighbor takes over automatically.

</details>

## 📻 Adaptive radio

<details>
<summary><b>Click to expand</b></summary>

The firmware automatically switches between Bluetooth LE PHYs based on link quality:

| PHY | Range | Throughput | When used |
|-----|-------|------------|-----------|
| **1M** | ~100 m | 1 Mbps | Default, efficient |
| **2M** | ~50 m | 2 Mbps | Very close links |
| **Coded S8** | **200-500 m** | 125 kbps | Long or weak links |

S8 is used **only when needed**, preserving battery on short links. Long links get the extended range they require.

</details>

## 🔐 Security

<details>
<summary><b>Click to expand</b></summary>

- **Message encryption**: AES-128-GCM for every payload.
- **BLE pairing**: passkey + MITM protection required to connect.
- **Authenticated sessions**: only paired devices can change configuration.
- **OTA gated by physical button**: firmware updates require pressing GPIO9 on the board.
- **No Secure Boot / Flash Encryption**: the board stays fully reusable.

</details>

## 📱 Compatible devices

| Device | Role | Notes |
|--------|------|-------|
| **Android** (API 26+) | Full node | BLE 5.0 with Coded PHY required |
| **iOS** | Client only | Via GATT bridge (CoreBluetooth limitation) |
| **PC** | Full node or client | Depends on BLE adapter |
| **XIAO ESP32-C6** | Full node | The reference hardware |

## 🎯 Use cases

- 🚨 Emergency communications when infrastructure fails.
- 🏔️ Remote areas without cellular coverage.
- 🥾 Hiking, sailing, and outdoor activities.
- 🎪 Festivals and large events where cell networks collapse.
- 🌐 Community mesh networks.

## 🛠️ Hardware

| Component | Minimum |
|-----------|---------|
| Board | Seeed Studio XIAO ESP32-C6 |
| Antenna | External U.FL 2.4 GHz |
| Power | USB-C or LiPo + solar panel |
| Framework | ESP-IDF 5.3 |

### 🔌 Pinout

| GPIO | Function |
|------|----------|
| GPIO15 | Status LED |
| GPIO3 | Antenna switch (LOW = enabled) |
| GPIO14 | Antenna select (HIGH = external) |
| GPIO9 | Recovery / OTA button |
| GPIO0 | Battery ADC |
| GPIO1 | Solar panel ADC |
| GPIO2 | Wakeup GPIO |

### 🏗️ Recommended setup

- XIAO ESP32-C6 + external U.FL antenna.
- LiPo 2000-5000 mAh + 5-10 W solar panel.
- Weatherproof enclosure for outdoor nodes.
- High mounting point for backbone nodes.

## 🚀 Quick start

1. **Download the firmware** from the [Actions tab](https://github.com/guillemetall/Meshxiaoo-v2/actions) (`xiaomesh-firmware.zip`).
2. **Flash** with [ESP32_Flasher](https://play.google.com/store/apps/details?id=com.espressif.flasher) (Android):

   | File | Offset |
   |------|--------|
   | `bootloader.bin` | `0x0` |
   | `partition-table.bin` | `0x8000` |
   | `xiaomesh.bin` | `0x10000` |

3. **Pair** with `nRF Connect` — the passkey appears on the serial monitor at first boot.
4. **Configure** via BLE GATT (service `0x00FF`).

📖 **Compiling from source?** See [BUILD.md](BUILD.md).

## ⚙️ Configuration

<details>
<summary><b>BLE GATT</b></summary>

Service UUID: `0x00FF`

| Characteristic | UUID | Permissions |
|----------------|------|-------------|
| Config | `0xFF01` | Read + Write |
| Messages | `0xFF02` | Write + Notify |
| OTA | `0xFF03` | Write + Notify |
| Status | `0xFF04` | Read + Notify |

**Config commands** (write to `0xFF01`):

| Bytes | Action |
|-------|--------|
| `[0x01, v]` | PHY: 0=auto, 1=1M, 2=2M, 3=S8 |
| `[0x02, v]` | Mode: 0=repeater, 1=bridge, 2=room |
| `[0x03, v]` | TX power: -12 to +20 dBm |
| `[0x04, 16B]` | Custom AES-128 key |

**Send message** (write to `0xFF02`): `[text][4 bytes destination]`

</details>

<details>
<summary><b>UART commands</b></summary>

Serial at 115200 baud:

cd ~/Meshxiaoo-v2 && cat >> README.md << 'EOF5'

## ⚙️ Configuration

<details>
<summary><b>BLE GATT</b></summary>

Service UUID: `0x00FF`

| Characteristic | UUID | Permissions |
|----------------|------|-------------|
| Config | `0xFF01` | Read + Write |
| Messages | `0xFF02` | Write + Notify |
| OTA | `0xFF03` | Write + Notify |
| Status | `0xFF04` | Read + Notify |

**Config commands** (write to `0xFF01`):

| Bytes | Action |
|-------|--------|
| `[0x01, v]` | PHY: 0=auto, 1=1M, 2=2M, 3=S8 |
| `[0x02, v]` | Mode: 0=repeater, 1=bridge, 2=room |
| `[0x03, v]` | TX power: -12 to +20 dBm |
| `[0x04, 16B]` | Custom AES-128 key |

**Send message** (write to `0xFF02`): `[text][4 bytes destination]`

</details>

<details>
<summary><b>UART commands</b></summary>

Serial at 115200 baud:


</details>

<details>
<summary><b>Recovery mode</b></summary>

Hold GPIO9 to GND at boot. The node creates a Wi-Fi AP:

- **SSID**: `XIAO-MESH-OTA`
- **Password**: `mesh1234`
- **URL**: `http://192.168.4.1/ota`

Upload the firmware and the node reboots.

</details>

## 📶 LED status

| Pattern | Meaning |
|---------|---------|
| Fast blink (5 Hz) | Booting |
| Slow blink (1 Hz) | Searching for mesh |
| Solid on | Connected |
| Double blink | Relaying |
| Triple blink | Low battery |

## 📋 Project status

**Version: v5.0.0**

| Feature | State |
|---------|-------|
| 📻 Coded PHY S8 | ✅ |
| 📡 Adaptive PHY switching | ✅ |
| 🕸️ Hybrid mesh routing | ✅ |
| 🔐 AES-128-GCM encryption | ✅ |
| 🔑 BLE pairing with MITM | ✅ |
| ⬆️ OTA with button gating | ✅ |
| 📱 Multi-connection GATT | ✅ |
| 🖥️ OLED display | 🔜 Planned |
| 📲 Flutter client app | 🔜 Planned |

## 📜 License

GNU General Public License v3.0 or later. See [LICENSE](LICENSE).
