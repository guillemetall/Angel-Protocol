# 📡 Angel Protocol

**Offline mesh messaging firmware for the Seeed Studio XIAO ESP32-C6.**

Angel Protocol turns a small, low-power ESP32-C6 board into a node of a decentralized messaging network that works without internet, cell towers, Wi-Fi, or LoRa. It uses Bluetooth LE Coded PHY S8 to reach distances of 200-500 m per hop, and chains multiple nodes together to cover kilometers.

Designed for emergencies, remote areas, hiking, sailing, festivals, and any situation where the usual networks fail.

## ✨ Highlights

- 📻 **Extended range**: Coded PHY S8 for up to 500 m per hop.
- 🛰️ **Works offline**: no internet, no cell towers, no infrastructure.
- 🔋 **Low power**: solar + LiPo friendly, with deep sleep support.
- 🔐 **Encrypted**: AES-128-GCM for every message.
- 🕸️ **Self-organizing mesh**: hybrid routing with composite node scoring.
- 📨 **Long messages**: up to 640 bytes per message.
- 📱 **Works with any phone**: Android, iOS, and PC via BLE GATT.
- 🔘 **Secure by design**: physical button required for pairing and OTA.
- 🆓 **Open source**: GPLv3 licensed.

## 🚀 Install the firmware

### Option A — Web installer (desktop only)

**[▶ Install Angel Protocol in your browser](https://guillemetall.github.io/Angel-Protocol/)**

> ⚠️ Requires Chrome or Edge on a desktop computer. Mobile browsers do not support Web Serial.

### Option B — Mobile (Android)

1. Install **ESP32_Flasher** from the Play Store.
2. Connect the board by USB-C OTG cable.
3. Download bootloader.bin, partition-table.bin, and angel.bin from the docs folder.
4. Flash in this order:

| File | Offset |
|------|--------|
| bootloader.bin | 0x0 |
| partition-table.bin | 0x8000 |
| angel.bin | 0x10000 |

### Option C — Command line


## 🔗 Pair with your phone

1. Open **nRF Connect** and scan for `XIAO-MESH`.
2. Tap **Connect**.
3. Enter passkey **`123456`**.
4. **Press the button (GPIO9) briefly** on the board to confirm.
5. Done.

> 💡 The passkey can be changed by UART before deployment.

## 🔐 Security

Angel Protocol uses a layered security model designed for devices without a screen.

- **BLE pairing**: passkey + MITM protection.
- **Physical confirmation**: pairing requires pressing the board button within 60 s.
- **Authenticated sessions**: only paired devices can change configuration.
- **OTA gate**: firmware updates require holding the button for 3 s.
- **No Secure Boot / Flash Encryption**: the board stays fully reflashable.

## 🕸️ How the mesh works

Nodes discover each other through BLE advertisements. The first message uses flooding to find a route; subsequent messages travel directly along it.

Each node scores its neighbors on:

| Weight | Metric |
|--------|--------|
| 40% | RSSI (signal strength) |
| 30% | PDR (packet delivery ratio) |
| 10% | Battery level |
| 20% | Hop count |

## 📻 Adaptive radio

| PHY | Range | Throughput | When used |
|-----|-------|------------|-----------|
| 1M | ~100 m | 1 Mbps | Default |
| 2M | ~50 m | 2 Mbps | Very close links |
| Coded S8 | 200-500 m | 125 kbps | Long or weak links |

S8 is used only when needed, preserving battery on short links.

## 🛠️ Hardware

| Component | Minimum |
|-----------|---------|
| Board | Seeed Studio XIAO ESP32-C6 |
| Antenna | External U.FL 2.4 GHz |
| Power | USB-C or LiPo + solar panel |

### Pinout

| GPIO | Function |
|------|----------|
| GPIO15 | Status LED |
| GPIO3 | Antenna switch |
| GPIO14 | Antenna select |
| GPIO9 | Pairing / OTA button |
| GPIO0 | Battery ADC |
| GPIO1 | Solar panel ADC |

## ⚙️ Configuration

Service UUID: `0x00FF`

| Characteristic | UUID | Permissions |
|----------------|------|-------------|
| Config | 0xFF01 | Read + Write (auth) |
| Messages | 0xFF02 | Write + Notify |
| OTA | 0xFF03 | Write (button + auth) |
| Status | 0xFF04 | Read + Notify |

### Config commands (write to 0xFF01)

| Bytes | Action |
|-------|--------|
| [0x01, v] | PHY: 0=auto, 1=1M, 2=2M, 3=S8 |
| [0x02, v] | Mode: 0=repeater, 1=bridge, 2=room |
| [0x03, v] | TX power: -12 to +20 dBm |
| [0x04, 16B] | Custom AES-128 key |

### UART commands (115200 baud)


### Recovery mode

Hold GPIO9 to GND at boot. The node creates a Wi-Fi AP:

- SSID: `XIAO-MESH-OTA`
- Password: `mesh1234`
- URL: `http://192.168.4.1/ota`

## 📶 LED status

| Pattern | Meaning |
|---------|---------|
| Fast blink | Booting |
| Slow blink | Searching for mesh |
| Solid on | Connected |
| Double blink | Relaying |
| Triple blink | Low battery |

## 📜 License

GNU General Public License v3.0 or later. See LICENSE.

