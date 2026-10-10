**English** | [Espanol](README.es.md)

# Xiaomesh

**Firmware de mensajeria mesh offline para la Seeed Studio XIAO ESP32-C6.**

Xiaomesh convierte una placa ESP32-C6 de bajo consumo en un nodo de una red de mensajeria descentralizada que funciona sin internet, torres celulares, Wi-Fi ni LoRa. Usa Bluetooth LE Coded PHY S8 para alcanzar distancias de 200-500 m por salto, y encadena varios nodos para cubrir kilometros.

Disenado para emergencias, zonas remotas, senderismo, nautica, festivales y cualquier situacion donde las redes habituales fallen.

## Lo mas destacado

- **Alcance extendido**: Coded PHY S8 para hasta 500 m por salto.
- **Funciona offline**: sin internet, sin torres, sin infraestructura.
- **Bajo consumo**: compatible con solar + LiPo, con deep sleep.
- **Cifrado de extremo a extremo**: AES-128-GCM para todos los mensajes.
- **Mesh auto-organizada**: enrutamiento hibrido con puntuacion compuesta.
- **Compatible con cualquier telefono**: Android, iOS y PC via BLE GATT.
- **Codigo abierto**: licencia GPLv3.

<details>
<summary><b>Descripcion completa</b></summary>

### Que es Xiaomesh

Xiaomesh es un firmware para la Seeed Studio XIAO ESP32-C6 que convierte la placa en un nodo de malla autonomo. Cada nodo puede actuar como repetidor, puente para telefonos, o buzon para usuarios desconectados.

### Como funciona la malla

Los nodos se descubren entre si mediante anuncios Bluetooth Low Energy. Cuando se envia un mensaje, el nodo de origen usa un mecanismo de inundacion para encontrar una ruta al destino. Una vez aprendida la ruta, los mensajes siguientes viajan directamente por ella. Cada nodo evalua a sus vecinos con una puntuacion compuesta (RSSI, tasa de entrega, bateria, saltos) y elige el siguiente salto mas fiable.

### Radio adaptativa

El firmware conmuta automaticamente entre los PHY de Bluetooth LE (1M, 2M y Coded PHY S8) segun la calidad del enlace. S8 se usa solo cuando es necesario, preservando bateria en enlaces cortos. Los enlaces largos obtienen el alcance extendido que necesitan.

### Seguridad

Todos los mensajes van cifrados con AES-128-GCM. La configuracion y el OTA estan protegidos por emparejamiento BLE (passkey + proteccion MITM) y, para OTA, un boton fisico. Ningun dispositivo no autorizado puede reconfigurar un nodo.

### Dispositivos compatibles

- **Android**: cualquier dispositivo con BLE 5.0 y API 26+.
- **iOS**: via puente GATT (limitacion de CoreBluetooth).
- **PC**: adaptador BLE con soporte Coded PHY, o puente GATT.
- **Otra XIAO ESP32-C6**: nodo completo de malla.

### Casos de uso

- Comunicaciones de emergencia cuando falla la infraestructura.
- Zonas remotas sin cobertura celular.
- Senderismo, nautica y actividades al aire libre.
- Festivales y eventos masivos donde colapsa la red celular.
- Redes mesh comunitarias.

</details>

## Requisitos

| Componente | Minimo |
|-----------|--------|
| Placa | Seeed Studio XIAO ESP32-C6 |
| Antena | Externa U.FL 2.4 GHz |
| Energia | USB-C o LiPo + panel solar |
| Framework | ESP-IDF 5.3 |

## Inicio rapido

### 1. Compilar sin PC

El firmware se compila automaticamente en GitHub Actions al hacer push al repositorio.

- Push a `main`.
- Espera 3-5 minutos en la pestana **Actions**.
- Descarga el artefacto `xiaomesh-firmware.zip`.

### 2. Flashear

Con **ESP32_Flasher** (Android) o **esptool.py** (PC):

| Archivo | Offset |
|---------|--------|
| `bootloader.bin` | `0x0` |
| `partition-table.bin` | `0x8000` |
| `xiaomesh.bin` | `0x10000` |

### 3. Configurar

Conecta a `XIAO-MESH` desde **nRF Connect** (o cualquier app compatible).

- UUID del servicio: `0x00FF`
- Config: `0xFF01`
- Mensajes: `0xFF02`
- OTA: `0xFF03`
- Estado: `0xFF04`

La primera vez que enciendas la placa, anota el **passkey** que aparece en el monitor serie. Lo necesitaras para emparejar.

### 4. Estado del LED

| Patron | Significado |
|--------|-------------|
| Parpadeo rapido (5 Hz) | Arrancando |
| Parpadeo lento (1 Hz) | Buscando malla |
| Encendido fijo | Conectado |
| Doble parpadeo | Retransmitiendo |
| Triple parpadeo | Bateria baja |

<details>
<summary><b>Configuracion detallada</b></summary>

### Comandos de configuracion

Escribe en `0xFF01`:

| Bytes | Accion |
|-------|--------|
| `[0x01, v]` | PHY: 0=auto, 1=1M, 2=2M, 3=S8 |
| `[0x02, v]` | Modo: 0=repetidor, 1=puente, 2=buzon |
| `[0x03, v]` | Potencia TX: -12 a +20 dBm |
| `[0x04, 16B]` | Clave AES-128 personalizada |

### Enviar mensajes

Escribe en `0xFF02`:


Usa `[0xFF,0xFF,0xFF,0xFF]` para broadcast.

### Comandos UART

Serie a 115200 baudios:


### Modo recuperacion

Manten GPIO9 a GND al arrancar. El nodo crea un AP Wi-Fi:

- SSID: `XIAO-MESH-OTA`
- Password: `mesh1234`
- URL: `http://192.168.4.1/ota`

Sube el firmware y el nodo se reinicia.

</details>

## Hardware

### Conexiones

| GPIO | Funcion |
|------|---------|
| GPIO15 | LED de estado |
| GPIO3 | Conmutador de antena (LOW = activo) |
| GPIO14 | Seleccion de antena (HIGH = externa) |
| GPIO9 | Boton recuperacion / OTA |
| GPIO0 | ADC bateria |
| GPIO1 | ADC panel solar |
| GPIO2 | GPIO de wakeup |

### Montaje recomendado

- XIAO ESP32-C6 + antena externa U.FL.
- LiPo 2000-5000 mAh + panel solar 5-10 W.
- Carcasa impermeable para nodos exteriores.
- Punto elevado para nodos troncales.

## Estado

**Version actual: v5.0.0**

| Caracteristica | Estado |
|----------------|--------|
| Coded PHY S8 | Hecho |
| Conmutacion adaptativa de PHY | Hecho |
| Enrutamiento mesh hibrido | Hecho |
| Cifrado AES-128-GCM | Hecho |
| Emparejamiento BLE con MITM | Hecho |
| OTA con boton fisico | Hecho |
| GATT multi-conexion | Hecho |
| Pantalla OLED | Planificado |
| App cliente Flutter | Planificado |

## Licencia

GNU General Public License v3.0 o posterior. Ver [LICENSE](LICENSE).
