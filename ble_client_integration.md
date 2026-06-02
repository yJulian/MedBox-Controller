# MedBox BLE Client Integration Guide

This guide describes how to integrate and use the [medbox_ble_client.js](file:///C:/Users/yJuli/Development/PlatformIO/MedBox-Controller/medbox_ble_client.js) library to interact with the MedBox offline control BLE server.

## Overview
The `MedBoxBleClient` class uses the **Web Bluetooth API** to scan for, connect to, and control the MedBox. It handles service/characteristic discovery and payload formatting (JSON) under the hood.

## Service & Characteristic UUIDs
* **Service UUID**: `0000ff01-0000-1000-8000-00805f9b34fb`
* **Control Characteristic UUID**: `0000ff02-0000-1000-8000-00805f9b34fb` (supports Read, Write, Notify)

---

## API Documentation

### Constructor
```javascript
const client = new MedBoxBleClient();
```

### Methods

#### `connect()`
* **Type**: `async`
* **Description**: Requests the BLE device named `MedBox BLE Control` and establishes a GATT connection. Also enables notifications.
* **Important**: **Must be triggered by a direct user action** (like a click event) due to browser security restrictions.
* **Throws**: An error if the Web Bluetooth API is unsupported or if the connection fails/is cancelled.

#### `disconnect()`
* **Type**: `async`
* **Description**: Safely disconnects the GATT server and cleans up references.

#### `dispense(compartmentPosition, amountOfPillsToDispense = 1)`
* **Type**: `async`
* **Parameters**:
  * `compartmentPosition` *(number)*: The index of the compartment (e.g., `0`, `1`, `2`, `3`).
  * `amountOfPillsToDispense` *(number)*: Number of dispensations/pills. Default is `1`.
* **Description**: Sends a JSON command to dispense pills.

#### `rotateFunnel(targetCompartmentNumber)`
* **Type**: `async`
* **Parameters**:
  * `targetCompartmentNumber` *(number)*: The target compartment position index to align the funnel to.
* **Description**: Rotates the funnel to the specified compartment.

#### `isConnected()`
* **Returns**: `boolean`
* **Description**: Checks if there is an active connection to the MedBox device.

---

## Callbacks / Event Hooks

You can register functions on these hooks to listen to connection and feedback events:

* `onConnected = () => {}`
  Triggered when connection is successfully established.
* `onDisconnected = () => {}`
  Triggered when connection is disconnected (manually or unexpectedly).
* `onNotification = (statusText) => {}`
  Triggered when the MedBox notifies back with a status string (e.g., `"OK"`, `"ERROR: ..."`).

---

## Complete Quickstart Template

```html
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>MedBox BLE Controller</title>
</head>
<body>
  <button id="connectBtn">Connect to MedBox</button>
  <button id="dispenseBtn" disabled>Dispense Compartment 0</button>
  <button id="disconnectBtn" disabled>Disconnect</button>

  <script src="medbox_ble_client.js"></script>
  <script>
    const client = new MedBoxBleClient();
    
    const connectBtn = document.getElementById('connectBtn');
    const dispenseBtn = document.getElementById('dispenseBtn');
    const disconnectBtn = document.getElementById('disconnectBtn');

    // Callback Setup
    client.onConnected = () => {
      connectBtn.disabled = true;
      dispenseBtn.disabled = false;
      disconnectBtn.disabled = false;
      console.log('Connected to MedBox BLE!');
    };

    client.onDisconnected = () => {
      connectBtn.disabled = false;
      dispenseBtn.disabled = true;
      disconnectBtn.disabled = true;
      console.log('Disconnected!');
    };

    client.onNotification = (data) => {
      console.log('Feedback from Box:', data);
    };

    // UI Action Event Handlers
    connectBtn.addEventListener('click', async () => {
      try {
        await client.connect();
      } catch (err) {
        alert('Connection failed: ' + err.message);
      }
    });

    dispenseBtn.addEventListener('click', async () => {
      try {
        // Dispense 1 pill from compartment 0
        await client.dispense(0, 1);
      } catch (err) {
        console.error('Dispense failed:', err);
      }
    });

    disconnectBtn.addEventListener('click', async () => {
      await client.disconnect();
    });
  </script>
</body>
</html>
```
