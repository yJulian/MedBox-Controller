/**
 * MedBoxBleClient - A JavaScript helper library for controlling MedBox
 * over Bluetooth Low Energy using the Web Bluetooth API.
 * 
 * Can be used in modern browsers (Chrome, Edge, Opera, Safari on iOS with compatible browsers)
 * to connect directly to the MedBox offline control interface.
 */
class MedBoxBleClient {
  constructor() {
    this.serviceUuid = '0000ff01-0000-1000-8000-00805f9b34fb';
    this.characteristicUuid = '0000ff02-0000-1000-8000-00805f9b34fb';
    
    this.device = null;
    this.server = null;
    this.service = null;
    this.characteristic = null;

    // Callbacks for client applications
    this.onConnected = null;
    this.onDisconnected = null;
    this.onNotification = null;
  }

  /**
   * Check if Web Bluetooth is supported by the user's browser.
   * @returns {boolean} True if supported
   */
  isSupported() {
    return navigator.bluetooth !== undefined;
  }

  /**
   * Check if currently connected to a MedBox device.
   * @returns {boolean} True if connected
   */
  isConnected() {
    return this.device && this.device.gatt.connected;
  }

  /**
   * Request user permission to connect to a MedBox BLE device and establish a connection.
   * Note: This MUST be called inside a user gesture (e.g. click event handler).
   * @returns {Promise<void>} Resolves when connection is successful
   */
  async connect() {
    if (!this.isSupported()) {
      throw new Error('Web Bluetooth API is not supported in this browser.');
    }

    try {
      console.log('Requesting MedBox Bluetooth device...');
      this.device = await navigator.bluetooth.requestDevice({
        filters: [
          { name: 'MedBox BLE Control' },
          { services: [this.serviceUuid] }
        ]
      });

      // Listen for unexpected disconnections
      this.device.addEventListener('gattserverdisconnected', (event) => {
        console.log('GATT server disconnected');
        this._cleanup();
        if (this.onDisconnected) {
          this.onDisconnected(event);
        }
      });

      console.log('Connecting to GATT Server...');
      this.server = await this.device.gatt.connect();

      console.log('Getting Primary Service...');
      this.service = await this.server.getPrimaryService(this.serviceUuid);

      console.log('Getting Control Characteristic...');
      this.characteristic = await this.service.getCharacteristic(this.characteristicUuid);

      // Setup notifications for status feedback from the MedBox
      console.log('Starting notifications...');
      await this.characteristic.startNotifications();
      this.characteristic.addEventListener('characteristicvaluechanged', (event) => {
        const decoder = new TextDecoder('utf-8');
        const value = decoder.decode(event.target.value);
        console.log('[MedBox BLE Client] Notification received:', value);
        if (this.onNotification) {
          this.onNotification(value);
        }
      });

      console.log('MedBox BLE connection established successfully!');
      if (this.onConnected) {
        this.onConnected();
      }
    } catch (error) {
      console.error('BLE Connection failed:', error);
      this._cleanup();
      throw error;
    }
  }

  /**
   * Disconnect from the MedBox BLE device.
   * @returns {Promise<void>} Resolves when disconnected
   */
  async disconnect() {
    if (!this.isConnected()) {
      console.log('Already disconnected');
      return;
    }
    
    console.log('Disconnecting from MedBox device...');
    await this.device.gatt.disconnect();
    this._cleanup();
  }

  /**
   * Send a JSON command payload to the MedBox control characteristic.
   * @param {Object} commandPayload The payload object to send
   * @returns {Promise<void>} Resolves when write is complete
   */
  async sendCommand(commandPayload) {
    if (!this.isConnected()) {
      throw new Error('Not connected to MedBox device. Call connect() first.');
    }

    try {
      const jsonString = JSON.stringify(commandPayload);
      console.log('Sending command:', jsonString);
      const encoder = new TextEncoder('utf-8');
      const dataBuffer = encoder.encode(jsonString);

      // Write value to GATT characteristic
      await this.characteristic.writeValue(dataBuffer);
    } catch (error) {
      console.error('Failed to send command:', error);
      throw error;
    }
  }

  /**
   * Trigger a pill dispense action.
   * @param {number} compartmentPosition The compartment to dispense from (e.g. 0, 1, 2, 3...)
   * @param {number} amountOfPillsToDispense Number of pills/dispensations to trigger
   * @returns {Promise<void>} Resolves when command is sent
   */
  async dispense(compartmentPosition, amountOfPillsToDispense = 1) {
    const payload = {
      messageType: 3,
      message: {
        compartmentPosition: Number(compartmentPosition),
        amountOfPillsToDispense: Number(amountOfPillsToDispense)
      }
    };
    return this.sendCommand(payload);
  }

  /**
   * Rotate the funnel to a specific compartment position.
   * @param {number} targetCompartmentNumber The compartment target index/position
   * @returns {Promise<void>} Resolves when command is sent
   */
  async rotateFunnel(targetCompartmentNumber) {
    const payload = {
      messageType: 4,
      message: {
        targetCompartmentNumber: Number(targetCompartmentNumber)
      }
    };
    return this.sendCommand(payload);
  }

  /**
   * Clean up local BLE references.
   * @private
   */
  _cleanup() {
    this.device = null;
    this.server = null;
    this.service = null;
    this.characteristic = null;
  }
}

// Export for ES modules / CommonJS compatibility if used in bundlers
if (typeof module !== 'undefined' && module.exports) {
  module.exports = MedBoxBleClient;
} else if (typeof window !== 'undefined') {
  window.MedBoxBleClient = MedBoxBleClient;
}
