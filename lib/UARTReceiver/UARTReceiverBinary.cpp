/**
 * @file UARTReceiverBinary.cpp
 * @brief Implementation der UART-Empfänger Library für Binärdaten
 * @author UARTReceiver Library
 * @version 2.0
 * @date 2025-07-15
 */

#include "UARTReceiverBinary.h"

/**
 * @brief Konstruktor
 */
UARTReceiverBinary::UARTReceiverBinary(HardwareSerial* serial, Stream* debugSerial, 
                                     int txPin, int rxPin, uint32_t baudrate, int ledPin) :
    _serial(serial),
    _debugSerial(debugSerial),
    _txPin(txPin),
    _rxPin(rxPin),
    _baudrate(baudrate),
    _ledPin(ledPin),
    _bufferIndex(0),
    _expectedPayloadSize(24),
    _lastDataReceived(0),
    _lastStatusUpdate(0),
    _lastHeartbeat(0),
    _totalMessagesReceived(0),
    _totalBytesReceived(0),
    _dataReceivedSinceLastCheck(false),
    _initialized(false),
    _systemReady(false),
    _binaryCallback(nullptr),
    _timeoutCallback(nullptr),
    _statusCallback(nullptr) {
}

/**
 * @brief Initialisiert die Hardware
 */
bool UARTReceiverBinary::begin() {
    if (_debugSerial) {
        _debugSerial->println("=== UARTReceiverBinary Hardware-Initialisierung ===");
    }
    
    // LED-Pin initialisieren
    if (_ledPin >= 0) {
        pinMode(_ledPin, OUTPUT);
        
        // Startup-Signal
        for(int i = 0; i < 3; i++) {
            digitalWrite(_ledPin, HIGH);
            delay(200);
            digitalWrite(_ledPin, LOW);
            delay(200);
        }
    }
    
    // UART-Pins setzen
    _serial->pins(_txPin, _rxPin);
    if (_debugSerial) {
        _debugSerial->println("UART Pins gesetzt:");
        _debugSerial->print("  TX Pin: ");
        _debugSerial->println(_txPin);
        _debugSerial->print("  RX Pin: ");
        _debugSerial->println(_rxPin);
    }
    
    // UART initialisieren
    _serial->begin(_baudrate, SERIAL_8N1);
    if (_debugSerial) {
        _debugSerial->print("UART initialisiert mit ");
        _debugSerial->print(_baudrate);
        _debugSerial->println(" Baud, 8N1");
    }
    
    delay(100);
    
    // Puffer leeren
    clearBuffer();
    
    // Zeitstempel initialisieren
    unsigned long currentTime = millis();
    _lastDataReceived = currentTime;
    _lastStatusUpdate = currentTime;
    _lastHeartbeat = currentTime;
    
    _initialized = true;
    _systemReady = true;
    
    if (_debugSerial) {
        _debugSerial->println("Hardware-Initialisierung abgeschlossen");
        _debugSerial->println("Binärdaten-Modus aktiv");
        _debugSerial->println("================================");
    }
    
    return true;
}

/**
 * @brief Hauptverarbeitungsschleife
 */
void UARTReceiverBinary::process() {
    if (!_systemReady) {
        return;
    }
    
    // Binärdaten verarbeiten
    if (_serial->available() > 0) {
        _lastDataReceived = millis();
        _dataReceivedSinceLastCheck = true;
        
        while (_serial->available() > 0) {
            uint8_t inByte = _serial->read();
            _totalBytesReceived++;
            
            // Debug-Ausgabe (optional)
            if (_debugSerial) {
                if (inByte < 16) _debugSerial->print("0");
                _debugSerial->print(inByte, HEX);
                _debugSerial->print(" ");
            }
            
            // Füge Byte zum Puffer hinzu
            _binaryBuffer[_bufferIndex] = inByte;
            _bufferIndex++;
            
            // Prüfe ob komplette Payload empfangen
            if (_bufferIndex >= _expectedPayloadSize) {
                processBinaryPayload(_binaryBuffer, _bufferIndex);
                _bufferIndex = 0;  // Reset für nächste Payload
            }
            
            // Pufferüberlauf verhindern
            if (_bufferIndex >= MAX_PAYLOAD_SIZE) {
                if (_debugSerial) {
                    _debugSerial->println("\nERROR: Buffer overflow!");
                }
                _bufferIndex = 0;
            }
        }
        
        if (_debugSerial) {
            _debugSerial->println();
        }
    }
    
    // Timeout-Prüfungen
    checkPayloadTimeout();
    checkDataTimeout();
    
    // Status-Updates
    displayPeriodicStatus();
    sendHeartbeat();
}

/**
 * @brief Beendet die Verbindung
 */
void UARTReceiverBinary::end() {
    if (_initialized) {
        _serial->end();
        _initialized = false;
        _systemReady = false;
        
        if (_debugSerial) {
            _debugSerial->println("UART-Verbindung beendet");
        }
    }
}

/**
 * @brief Verarbeitet eine empfangene Binär-Payload
 */
void UARTReceiverBinary::processBinaryPayload(const uint8_t* payload, size_t size) {
    if (_debugSerial) {
        _debugSerial->println("\n=== BINÄRE PAYLOAD EMPFANGEN ===");
        _debugSerial->print("Größe: ");
        _debugSerial->println(size);
        
        // Hex-Dump
        for (size_t i = 0; i < size; i++) {
            if (payload[i] < 16) _debugSerial->print("0");
            _debugSerial->print(payload[i], HEX);
            _debugSerial->print(" ");
        }
        _debugSerial->println();
    }
    
    // Dekodiere und zeige Sensordaten
    SensorData sensorData = decodeSensorData(payload, size);
    
    if (_debugSerial) {
        _debugSerial->println("Dekodierte Sensordaten:");
        
        if (sensorData.hasTemperature) {
            _debugSerial->print("  Temp1: ");
            _debugSerial->print(sensorData.temperature1);
            _debugSerial->print("°C, Temp2: ");
            _debugSerial->print(sensorData.temperature2);
            _debugSerial->println("°C");
        }
        
        if (sensorData.hasDeflection) {
            _debugSerial->print("  Deflection: ");
            _debugSerial->println(sensorData.deflection);
        }
        
        if (sensorData.hasPressure) {
            _debugSerial->print("  Pressure: ");
            _debugSerial->println(sensorData.pressure);
        }
        
        if (sensorData.hasPicTemp) {
            _debugSerial->print("  PIC Temp: ");
            _debugSerial->print(sensorData.picTemp);
            _debugSerial->println("°C");
        }
        
        _debugSerial->println("=== ENDE PAYLOAD ===");
    }
    
    // Callback aufrufen
    if (_binaryCallback) {
        _binaryCallback(payload, size);
    }
    
    _totalMessagesReceived++;
    
    // LED blinken lassen
    if (_ledPin >= 0) {
        digitalWrite(_ledPin, HIGH);
        delay(50);
        digitalWrite(_ledPin, LOW);
    }
}

/**
 * @brief Dekodiert Sensordaten
 */
SensorData UARTReceiverBinary::decodeSensorData(const uint8_t* data, size_t size) {
    SensorData result;
    result.timestamp = millis();
    
    size_t index = 0;
    while (index < size) {
        if (index + 1 > size) break;
        
        char sensorType = (char)data[index];
        index++;
        
        if (sensorType == 'T' && index + 8 <= size) {
            // Temperatur - 2 Float-Werte
            result.temperature1 = readFloat(data, index);
            index += 4;
            result.temperature2 = readFloat(data, index);
            index += 4;
            result.hasTemperature = true;
            
        } else if (sensorType == 'D' && index + 4 <= size) {
            // Deflection
            result.deflection = readFloat(data, index);
            index += 4;
            result.hasDeflection = true;
            
        } else if (sensorType == 'P' && index + 4 <= size) {
            // Pressure
            result.pressure = readFloat(data, index);
            index += 4;
            result.hasPressure = true;
            
        } else if (sensorType == 'S' && index + 4 <= size) {
            // PIC Temperature
            result.picTemp = readFloat(data, index);
            index += 4;
            result.hasPicTemp = true;
            
        } else {
            // Unbekannter Typ oder unvollständige Daten
            if (_debugSerial) {
                _debugSerial->print("Unbekannter Sensor-Typ: ");
                _debugSerial->println(sensorType);
            }
            break;
        }
    }
    
    return result;
}

/**
 * @brief Konvertiert 4 Bytes zu Float (Little Endian)
 */
float UARTReceiverBinary::readFloat(const uint8_t* data, size_t startIndex) {
    union {
        float f;
        uint8_t bytes[4];
    } converter;
    
    for (int i = 0; i < 4; i++) {
        converter.bytes[i] = data[startIndex + i];
    }
    
    return converter.f;
}

/**
 * @brief Prüft Timeout für unvollständige Payloads
 */
void UARTReceiverBinary::checkPayloadTimeout() {
    if (_bufferIndex > 0 && (millis() - _lastDataReceived) > PAYLOAD_TIMEOUT) {
        if (_debugSerial) {
            _debugSerial->println("WARNING: Unvollständige Payload - Puffer zurückgesetzt");
        }
        _bufferIndex = 0;
    }
}

/**
 * @brief Prüft Timeout für Datenempfang
 */
void UARTReceiverBinary::checkDataTimeout() {
    unsigned long currentTime = millis();
    
    if ((currentTime - _lastDataReceived) >= UART_TIMEOUT_MS) {
        if (_timeoutCallback) {
            _timeoutCallback(currentTime - _lastDataReceived);
        }
    }
}

/**
 * @brief Zeigt periodische Status-Informationen
 */
void UARTReceiverBinary::displayPeriodicStatus() {
    unsigned long currentTime = millis();
    
    if ((currentTime - _lastStatusUpdate) >= UART_STATUS_UPDATE_MS) {
        if (_dataReceivedSinceLastCheck) {
            if (_debugSerial) {
                _debugSerial->print("\n[STATISTIK] Nachrichten: ");
                _debugSerial->print(_totalMessagesReceived);
                _debugSerial->print(", Bytes: ");
                _debugSerial->print(_totalBytesReceived);
                _debugSerial->print(", Uptime: ");
                _debugSerial->print(currentTime / 1000);
                _debugSerial->println(" s");
            }
            
            if (_statusCallback) {
                _statusCallback(_totalMessagesReceived, _totalBytesReceived, currentTime);
            }
            
            _dataReceivedSinceLastCheck = false;
        }
        
        _lastStatusUpdate = currentTime;
    }
}

/**
 * @brief Sendet Heartbeat-Signal
 */
void UARTReceiverBinary::sendHeartbeat() {
    if(millis() - _lastHeartbeat > UART_HEARTBEAT_INTERVAL) {
        if (_debugSerial) {
            _debugSerial->println("HEARTBEAT");
        }
        _lastHeartbeat = millis();
    }
}

/**
 * @brief Setzt erwartete Payload-Größe
 */
void UARTReceiverBinary::setExpectedPayloadSize(size_t size) {
    _expectedPayloadSize = size;
    if (_debugSerial) {
        _debugSerial->print("Erwartete Payload-Größe: ");
        _debugSerial->print(size);
        _debugSerial->println(" Bytes");
    }
}

/**
 * @brief Setzt Binärdaten-Callback
 */
void UARTReceiverBinary::setBinaryCallback(BinaryDataCallback callback) {
    _binaryCallback = callback;
}

/**
 * @brief Setzt Timeout-Callback
 */
void UARTReceiverBinary::setTimeoutCallback(TimeoutCallback callback) {
    _timeoutCallback = callback;
}

/**
 * @brief Setzt Status-Callback
 */
void UARTReceiverBinary::setStatusCallback(StatusCallback callback) {
    _statusCallback = callback;
}

/**
 * @brief Gibt Statistiken zurück
 */
void UARTReceiverBinary::getStatistics(uint32_t& messages, uint32_t& bytes, unsigned long& uptime) {
    messages = _totalMessagesReceived;
    bytes = _totalBytesReceived;
    uptime = millis();
}

/**
 * @brief Prüft ob System bereit ist
 */
bool UARTReceiverBinary::isReady() const {
    return _systemReady;
}

/**
 * @brief Leert den Puffer
 */
void UARTReceiverBinary::clearBuffer() {
    _bufferIndex = 0;
    
    // Hardware-Puffer leeren
    while (_serial->available() > 0) {
        _serial->read();
    }
}

/**
 * @brief Zeigt Systeminformationen
 */
void UARTReceiverBinary::displaySystemInfo() {
    if (!_debugSerial) return;
    
    _debugSerial->println("\n=== UARTReceiverBinary Library Info ===");
    _debugSerial->println("Funktion: UART-Binärdaten-Empfänger");
    _debugSerial->print("Build: ");
    _debugSerial->print(__DATE__);
    _debugSerial->print(" ");
    _debugSerial->println(__TIME__);
    
    _debugSerial->println("\n=== Konfiguration ===");
    _debugSerial->print("UART Baudrate: ");
    _debugSerial->print(_baudrate);
    _debugSerial->println(" Baud");
    _debugSerial->print("Erwartete Payload-Größe: ");
    _debugSerial->print(_expectedPayloadSize);
    _debugSerial->println(" Bytes");
    
    _debugSerial->println("\n=== Pin-Zuordnung ===");
    _debugSerial->print("UART TX: Pin ");
    _debugSerial->println(_txPin);
    _debugSerial->print("UART RX: Pin ");
    _debugSerial->println(_rxPin);
    if (_ledPin >= 0) {
        _debugSerial->print("Status LED: Pin ");
        _debugSerial->println(_ledPin);
    }
    
    _debugSerial->println("\n=== Unterstützte Sensor-Typen ===");
    _debugSerial->println("T: Temperatur (2 Float-Werte)");
    _debugSerial->println("D: Deflection (1 Float-Wert)");
    _debugSerial->println("P: Pressure (1 Float-Wert)");
    _debugSerial->println("S: PIC Temperature (1 Float-Wert)");
    
    _debugSerial->println("\n=== Status ===");
    _debugSerial->println("Bereit für Binärdaten-Empfang...");
    _debugSerial->println("=====================================\n");
}

/**
 * @brief Sendet Testdaten
 */
void UARTReceiverBinary::sendTestData(const uint8_t* data, size_t size) {
    if (_initialized && data && size > 0) {
        _serial->write(data, size);
        _serial->flush();
        
        if (_debugSerial) {
            _debugSerial->print("Testdaten gesendet: ");
            _debugSerial->print(size);
            _debugSerial->println(" Bytes");
        }
    }
}
