# MedBox Controller - AI Agent Instructions

## Project Overview
ESP32-based controller for the MedBox CAD project at KIT. Handles UART communication between controllers and drives stepper motors for pill dispensing based on backend commands.

## Architecture

### Module Structure
- `main.cpp`: Minimal setup/loop - delegates to modular components
- `gpio.hpp/.cpp`: GPIO initialization (pins defined in `defines.hpp`)
- `wifi_helper.hpp/.cpp`: WiFiManager integration with reset capability
- `defines.hpp`: Central configuration (pins, default values)

### Key Design Patterns
1. **Header guards**: Use `#ifndef FILENAME_HPP` / `#pragma once` consistently
2. **Include hierarchy**: `defines.hpp` → component headers → main.cpp
3. **Separation**: `.hpp` for declarations, `.cpp` for implementations with header guards
4. **Arduino framework**: Use Arduino.h primitives (pinMode, digitalRead, Serial, etc.)

## Development Workflow

### PlatformIO Commands
```bash
# Build project
pio run -e esp32dev

# Upload to device
pio run -e esp32dev -t upload

# Serial monitor (115200 baud - defined in main.cpp)
pio device monitor

# Clean build
pio run -t clean
```

### WiFi Reset Feature
- Hold GPIO 5 (RESET_PIN) LOW during boot to clear WiFi credentials
- Device creates AP "MedBoxController" for reconfiguration
- Uses WiFiManager library for captive portal setup

## Project-Specific Conventions

### GPIO Configuration
- All pins defined in `defines.hpp` with descriptive names
- Initialize with 100ms stabilization delay in `initializeGPIO()`
- Use INPUT_PULLUP for reset/button pins

### WiFi Connection Pattern
- `connectWifi()` returns bool but restarts ESP on failure
- Check RESET_PIN before autoConnect to allow credential reset
- Serial messages use informal tone ("connected...yeey :)")

### Dependencies
- WiFiManager (tzapu/WiFiManager@^2.0.17) for network config
- Future: Stepper motor library (not yet implemented)
- UART communication (planned, not yet implemented)

## Related Projects

### Testbench Project
Separate ESP32 project (`../Testbench`) for BLE GATT server testing. Uses similar PlatformIO structure but different functionality (BLE instead of WiFi). Not a dependency of MedBox Controller.

## Implementation Notes
- Stepper motor control: Not yet implemented (mentioned in README)
- Backend integration: Communication interface TBD
- UART between controllers: Planned feature, no code yet
- Error handling: Currently restarts ESP on WiFi failure

## When Adding Features
1. Define hardware pins in `defines.hpp`
2. Create separate .hpp/.cpp module for new functionality
3. Keep main.cpp minimal - just setup/loop orchestration
4. Add library dependencies to `platformio.ini` lib_deps
5. Use 115200 baud for Serial debug output
