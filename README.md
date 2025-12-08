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