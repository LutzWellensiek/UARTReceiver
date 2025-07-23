/**
 * @file Payload_Builder.cpp
 * @brief Implementation of the Payload_Builder class
 * @author Smart Wire Industries
 * @version 1.0.1
 * @date 2025-01-17
 * @license MIT
 */

#include "Payload_Builder.h"

Payload_Builder::Payload_Builder(Stream* debugSerial) : _debugSerial(debugSerial) {
    // Initialize configuration with default values
    _config.temperatureSensorsEnabled = false;
    _config.deflectionSensorsEnabled = false;
    _config.pressureSensorsEnabled = false;
    _config.miscSensorsEnabled = false;
    _config.ultraCompactMode = false;
    _config.debugEnabled = false;
    _config.maxPayloadSize = PAYLOAD_DEFAULT_MAX_SIZE;
    memset(_config.deviceId, 0, MAX_DEVICE_ID_LENGTH);
    reset();
}

Payload_Builder::~Payload_Builder() {}

// ================================================================
// PRIVATE DEBUG METHODS
// ================================================================

void Payload_Builder::debugPrint(const char* message) {
    if (_debugSerial) {
        _debugSerial->print(message);
    }
}

void Payload_Builder::debugPrintln(const char* message) {
    if (_debugSerial) {
        _debugSerial->println(message);
    }
}

void Payload_Builder::debugPrintFloat(float value, int decimals) {
    if (_debugSerial) {
        _debugSerial->print(value, decimals);
    }
}

void Payload_Builder::debugPrintHex(uint8_t value) {
    if (_debugSerial) {
        if (value < 0x10) {
            _debugSerial->print("0");
        }
        _debugSerial->print(value, HEX);
    }
}

// ================================================================
// CONFIGURATION METHODS
// ================================================================

bool Payload_Builder::enableTemperatureSensors(uint8_t count) {
    if (count > 0 && count <= MAX_TEMPERATURE_SENSORS) {
        _config.temperatureSensorsEnabled = true;
        _config.temperatureSensorCount = count;
        return true;
    }
    return false;
}

bool Payload_Builder::enableDeflectionSensors(uint8_t count) {
    if (count > 0 && count <= MAX_DEFLECTION_SENSORS) {
        _config.deflectionSensorsEnabled = true;
        _config.deflectionSensorCount = count;
        return true;
    }
    return false;
}

bool Payload_Builder::enablePressureSensors(uint8_t count) {
    if (count > 0 && count <= MAX_PRESSURE_SENSORS) {
        _config.pressureSensorsEnabled = true;
        _config.pressureSensorCount = count;
        return true;
    }
    return false;
}

bool Payload_Builder::enableMiscSensors(uint8_t count) {
    if (count > 0 && count <= MAX_MISC_SENSORS) {
        _config.miscSensorsEnabled = true;
        _config.miscSensorCount = count;
        return true;
    }
    return false;
}

bool Payload_Builder::setDeviceId(const char* deviceId) {
    size_t len = strlen(deviceId);
    if (len < MAX_DEVICE_ID_LENGTH) {
        strcpy(_config.deviceId, deviceId);
        return true;
    }
    return false;
}

void Payload_Builder::setUltraCompactMode(bool enabled) {
    _config.ultraCompactMode = enabled;
}

void Payload_Builder::setMaxPayloadSize(size_t maxSize) {
    _config.maxPayloadSize = maxSize;
}

void Payload_Builder::setDebugEnabled(bool enabled) {
    _config.debugEnabled = enabled;
}

// ================================================================
// SENSOR DATA METHODS
// ================================================================

bool Payload_Builder::addTemperature(float temperature) {
    if (_data.temperatureCount < _config.temperatureSensorCount) {
        _data.temperatureValues[_data.temperatureCount++] = temperature;
        return true;
    }
    return false;
}

bool Payload_Builder::addDeflection(float deflection) {
    if (_data.deflectionCount < _config.deflectionSensorCount) {
        _data.deflectionValues[_data.deflectionCount++] = deflection;
        return true;
    }
    return false;
}

bool Payload_Builder::addPressure(float pressure) {
    if (_data.pressureCount < _config.pressureSensorCount) {
        _data.pressureValues[_data.pressureCount++] = pressure;
        return true;
    }
    return false;
}

bool Payload_Builder::addMiscData(float value) {
    if (_data.miscCount < _config.miscSensorCount) {
        _data.miscValues[_data.miscCount++] = value;
        return true;
    }
    return false;
}

void Payload_Builder::setTimestamp(uint32_t timestamp) {
    _data.timestamp = timestamp;
}

// ================================================================
// PAYLOAD BUILDING METHODS
// ================================================================

size_t Payload_Builder::build(uint8_t* buffer, size_t bufferSize) {
    if (bufferSize < _config.maxPayloadSize) {
        return 0;
    }

    size_t offset = 0;

    // Add device ID if not in ultra compact mode
    if (!_config.ultraCompactMode) {
        strcpy((char*)buffer, _config.deviceId);
        offset += strlen(_config.deviceId);
        buffer[offset++] = '\0';
    }

    // Add sensor data in order
    if (_config.temperatureSensorsEnabled) {
        offset = addTemperatureData(buffer, bufferSize, offset);
    }
    if (_config.deflectionSensorsEnabled) {
        offset = addDeflectionData(buffer, bufferSize, offset);
    }
    if (_config.pressureSensorsEnabled) {
        offset = addPressureData(buffer, bufferSize, offset);
    }
    if (_config.miscSensorsEnabled) {
        offset = addMiscData(buffer, bufferSize, offset);
    }

    // Debug output if enabled
    if (_config.debugEnabled) {
        debugPrintPayload(buffer, offset);
    }

    return offset;
}

size_t Payload_Builder::getEstimatedSize() {
    size_t size = 0;
    
    // Add device ID size if not in ultra compact mode
    if (!_config.ultraCompactMode) {
        size += strlen(_config.deviceId) + 1;
    }
    
    // Add sensor data sizes (4 bytes per float + 1 byte type identifier)
    if (_config.temperatureSensorsEnabled) {
        size += 1 + (_data.temperatureCount * sizeof(float));
    }
    if (_config.deflectionSensorsEnabled) {
        size += 1 + (_data.deflectionCount * sizeof(float));
    }
    if (_config.pressureSensorsEnabled) {
        size += 1 + (_data.pressureCount * sizeof(float));
    }
    if (_config.miscSensorsEnabled) {
        size += 1 + (_data.miscCount * sizeof(float));
    }
    
    return size;
}

void Payload_Builder::reset() {
    _data.temperatureCount = 0;
    _data.deflectionCount = 0;
    _data.pressureCount = 0;
    _data.miscCount = 0;
    _data.timestamp = 0;
}

// ================================================================
// GETTER METHODS
// ================================================================

PayloadConfig Payload_Builder::getConfig() {
    return _config;
}

SensorData Payload_Builder::getSensorData() {
    return _data;
}

// ================================================================
// DEBUG METHODS
// ================================================================

void Payload_Builder::debugPrintPayload(const uint8_t* buffer, size_t size) {
    if (_debugSerial) {
        _debugSerial->println("[DEBUG] Payload:");
        for (size_t i = 0; i < size; i++) {
            debugPrintHex(buffer[i]);
            _debugSerial->print(" ");
        }
        _debugSerial->println();
    }
}

void Payload_Builder::debugPrintConfig() {
    if (_debugSerial) {
        _debugSerial->println("[DEBUG] Payload_Config:");
        _debugSerial->print("Temperature Sensors: ");
        _debugSerial->println(_config.temperatureSensorCount);
        _debugSerial->print("Deflection Sensors: ");
        _debugSerial->println(_config.deflectionSensorCount);
        _debugSerial->print("Pressure Sensors: ");
        _debugSerial->println(_config.pressureSensorCount);
        _debugSerial->print("Misc Sensors: ");
        _debugSerial->println(_config.miscSensorCount);
        _debugSerial->print("Device ID: ");
        _debugSerial->println(_config.deviceId);
        _debugSerial->print("Max Payload Size: ");
        _debugSerial->println(_config.maxPayloadSize);
        _debugSerial->print("Ultra Compact Mode: ");
        _debugSerial->println(_config.ultraCompactMode ? "Enabled" : "Disabled");
        _debugSerial->print("Debug Enabled: ");
        _debugSerial->println(_config.debugEnabled ? "Yes" : "No");
    }
}

void Payload_Builder::debugPrintSensorData() {
    if (_debugSerial) {
        _debugSerial->println("[DEBUG] Sensor Data:");
        for (uint8_t i = 0; i < _data.temperatureCount; i++) {
            _debugSerial->print("Temperature ");
            _debugSerial->print(i + 1);
            _debugSerial->print(": ");
            debugPrintFloat(_data.temperatureValues[i]);
            _debugSerial->println("°C");
        }
        for (uint8_t i = 0; i < _data.deflectionCount; i++) {
            _debugSerial->print("Deflection ");
            _debugSerial->print(i + 1);
            _debugSerial->print(": ");
            debugPrintFloat(_data.deflectionValues[i], 4);
            _debugSerial->println(" mm");
        }
        for (uint8_t i = 0; i < _data.pressureCount; i++) {
            _debugSerial->print("Pressure ");
            _debugSerial->print(i + 1);
            _debugSerial->print(": ");
            debugPrintFloat(_data.pressureValues[i], 1);
            _debugSerial->println(" hPa");
        }
        for (uint8_t i = 0; i < _data.miscCount; i++) {
            _debugSerial->print("Misc Data ");
            _debugSerial->print(i + 1);
            _debugSerial->print(": ");
            debugPrintFloat(_data.miscValues[i]);
            _debugSerial->println();
        }
    }
}

void Payload_Builder::debugDecodePayload(const uint8_t* buffer, size_t size) {
    if (!_debugSerial) return;
    
    _debugSerial->println("[DEBUG] Decode Payload:");
    size_t offset = 0;
    
    // Skip Device ID in non-compact mode
    if (!_config.ultraCompactMode) {
        offset += strlen((const char*)buffer) + 1;
    }
    
    // Decode sensor data by type
    while (offset < size) {
        char sensorType = buffer[offset++];
        switch (sensorType) {
            case SENSOR_TYPE_TEMPERATURE:
                _debugSerial->print("Temperature data found\n");
                break;
            case SENSOR_TYPE_DEFLECTION:
                _debugSerial->print("Deflection data found\n");
                break;
            case SENSOR_TYPE_PRESSURE:
                _debugSerial->print("Pressure data found\n");
                break;
            case SENSOR_TYPE_MISC:
                _debugSerial->print("Misc data found\n");
                break;
        }
        // Skip float data (4 bytes per value)
        offset += 4;
    }
}

// ================================================================
// PRIVATE PAYLOAD BUILDING METHODS
// ================================================================

size_t Payload_Builder::addTemperatureData(uint8_t* buffer, size_t bufferSize, size_t offset) {
    if (offset + 1 + (_data.temperatureCount * sizeof(float)) > bufferSize) {
        return offset;
    }
    
    buffer[offset++] = SENSOR_TYPE_TEMPERATURE;
    for (uint8_t i = 0; i < _data.temperatureCount; i++) {
        floatToBytes(_data.temperatureValues[i], &buffer[offset]);
        offset += sizeof(float);
    }
    return offset;
}

size_t Payload_Builder::addDeflectionData(uint8_t* buffer, size_t bufferSize, size_t offset) {
    if (offset + 1 + (_data.deflectionCount * sizeof(float)) > bufferSize) {
        return offset;
    }
    
    buffer[offset++] = SENSOR_TYPE_DEFLECTION;
    for (uint8_t i = 0; i < _data.deflectionCount; i++) {
        floatToBytes(_data.deflectionValues[i], &buffer[offset]);
        offset += sizeof(float);
    }
    return offset;
}

size_t Payload_Builder::addPressureData(uint8_t* buffer, size_t bufferSize, size_t offset) {
    if (offset + 1 + (_data.pressureCount * sizeof(float)) > bufferSize) {
        return offset;
    }
    
    buffer[offset++] = SENSOR_TYPE_PRESSURE;
    for (uint8_t i = 0; i < _data.pressureCount; i++) {
        floatToBytes(_data.pressureValues[i], &buffer[offset]);
        offset += sizeof(float);
    }
    return offset;
}

size_t Payload_Builder::addMiscData(uint8_t* buffer, size_t bufferSize, size_t offset) {
    if (offset + 1 + (_data.miscCount * sizeof(float)) > bufferSize) {
        return offset;
    }
    
    buffer[offset++] = SENSOR_TYPE_MISC;
    for (uint8_t i = 0; i < _data.miscCount; i++) {
        floatToBytes(_data.miscValues[i], &buffer[offset]);
        offset += sizeof(float);
    }
    return offset;
}

// ================================================================
// UTILITY METHODS
// ================================================================

void Payload_Builder::floatToBytes(float value, uint8_t* bytes) {
    memcpy(bytes, &value, sizeof(float));
}

float Payload_Builder::bytesToFloat(const uint8_t* bytes) {
    float value;
    memcpy(&value, bytes, sizeof(float));
    return value;
}
