# MedBox Core Controller

PlatformIO Project for the Core Controller of the **MedBox CAD Project** at **Karlsruhe Institute of Technology (KIT)**.

This project manages communication between controllers via **UART** and controls pill dispensing through **Stepper Motors** based on feedback from the backend system.

---

## 📖 Overview
The MedBox Core Controller is the central unit responsible for:
- Handling UART communication between distributed controllers.
- Receiving backend feedback signals.
- Driving stepper motors to dispense pills reliably and precisely.

---

## ⚙️ Features
- **WiFi Connectivity**: Automatic WiFi connection with BLE configuration fallback.
- **WebSocket Communication**: Real-time bidirectional communication with backend server.
  - Automatic connection and reconnection (5-second interval)
  - Keep-alive mechanism with heartbeat (30-second ping interval)
  - Configurable endpoint parameters
- **UART Communication**: Robust data exchange between multiple controllers.
- **Backend Integration**: Processes commands and feedback from the server/backend.
- **Stepper Motor Control**: Accurate pill dispensing mechanism.
- **PlatformIO Support**: Easy build, upload, and debugging workflow.

---

## 🛠️ Requirements
- [PlatformIO](https://platformio.org/) installed in your development environment.
- Supported microcontroller (e.g., ESP32 or similar uC depending on hardware setup).
- Hardware setup with:
  - UART-enabled controllers
  - Stepper motors and drivers
  - Backend communication interface

---

## 🚀 Getting Started

### Clone the Repository
```bash
git clone https://github.com/<your-org>/medbox-core-controller.git
cd medbox-core-controller
```

### Configure WebSocket Connection
Edit `src/defines.hpp` to configure the WebSocket connection parameters:

```cpp
// WebSocket Configuration
#define WS_HOST "192.168.1.100"          // WebSocket server host/IP
#define WS_PORT 8080                      // WebSocket server port
#define WS_PATH "/ws"                     // WebSocket endpoint path
#define WS_RECONNECT_INTERVAL 5000        // Reconnection interval in milliseconds
#define WS_PING_INTERVAL 30000            // Ping interval for keep-alive in milliseconds
```

### Build and Upload
```bash
# Build the project
pio run -e esp32dev

# Upload to device
pio run -e esp32dev -t upload

# Monitor serial output
pio device monitor
```

---

## 🔧 Configuration

### WiFi Setup
On first boot or when GPIO 15 (RESET_PIN) is held LOW during startup:
1. The device creates a BLE GATT server named "MedBox Controller"
2. Connect via BLE and send WiFi credentials
3. Device saves credentials and connects automatically on subsequent boots

### WebSocket Connection
- The ESP32 automatically establishes a WebSocket connection after WiFi is connected
- Connection parameters are configured in `src/defines.hpp`
- The connection includes:
  - Automatic reconnection on disconnect (5-second retry interval)
  - Keep-alive heartbeat every 30 seconds
  - Event handlers for connection state and incoming messages

---

## 📝 Project Structure
```
src/
├── main.cpp                    # Main program entry point with FreeRTOS LED task
├── defines.hpp                 # Configuration parameters (WiFi, WebSocket, GPIO pins, LED codes)
├── gpio.hpp/cpp                # GPIO initialization (Reset pin, LED pin)
├── wifi_helper.hpp/cpp         # WiFi connection manager with NVS storage
├── ble_helper.hpp/cpp          # BLE GATT server for WiFi configuration
└── websocket_helper.hpp/cpp    # WebSocket client with automatic reconnection and heartbeat
```

### Module Overview

**main.cpp**
- Minimal setup and loop orchestration
- FreeRTOS task for non-blocking LED status display
- Coordinates WiFi, BLE, and WebSocket initialization

**defines.hpp**
- Central configuration for all pins and constants
- WebSocket endpoint configuration
- LED state pattern documentation

**gpio.hpp/.cpp**
- GPIO pin initialization with proper header structure
- Configures reset button and status LED

**wifi_helper.hpp/.cpp**
- WiFi credential management using ESP32 Preferences (NVS)
- Automatic connection with timeout
- Reset button support for credential clearing
- Delegates to BLE helper when credentials are missing

**ble_helper.hpp/.cpp**
- BLE GATT server for mobile app configuration
- State machine for WiFi credential collection
- Commands: GET_MAC, SCAN_WIFI, CON_WIFI
- Automatic ESP restart after successful configuration

**websocket_helper.hpp/.cpp**
- WebSocket client with keep-alive heartbeat
- Automatic reconnection on disconnect
- JSON message parsing with error handling
- Event-driven architecture for connection state

# Limitations
Due to the fact that UART has an IDLE state of HIGH/'1' the backchannel of the communication does not work in reverse direction when more than one slave is connected.
It reproducably work with two ESPs but does not work with more than two.
The UART reverse channel code should be edited to support tri-state drivers or use a chained uart configuration to officially support more than one slave.