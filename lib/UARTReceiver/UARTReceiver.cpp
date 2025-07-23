/**
 * @file UARTReceiver.cpp
 * @brief Implementierung der UART-Empfänger Library
 * @author Basierend auf UART-Empfänger Hauptprogramm
 * @date 2025-07-15
 */

#include "UARTReceiver.h"

/**
 * @brief Konstruktor der UARTReceiver Klasse
 */
UARTReceiver::UARTReceiver(HardwareSerial* serial, Stream* debugSerial, 
                          int txPin, int rxPin, uint32_t baudrate, int ledPin) :
    _serial(serial),
    _debugSerial(debugSerial),
    _txPin(txPin),
    _rxPin(rxPin),
    _baudrate(baudrate),
    _ledPin(ledPin),
    _inputBuffer(""),
    _lastDataReceived(0),
    _lastStatusUpdate(0),
    _lastTimeoutMessage(0),
    _lastHeartbeat(0),
    _totalMessagesReceived(0),
    _totalBytesReceived(0),
    _dataReceivedSinceLastCheck(false),
    _initialized(false),
    _systemReady(false),
    _binaryMode(false),
    _messageCallback(nullptr),
    _jsonCallback(nullptr),
    _textCallback(nullptr),
    _timeoutCallback(nullptr),
    _statusCallback(nullptr),
    _binaryCallback(nullptr),
    _bufferIndex(0),
    _expectedPayloadSize(28),
    _lastBinaryDataReceived(0),
    _bufferSize(UART_BUFFER_SIZE),
    _timeoutMs(UART_TIMEOUT_MS),
    _statusUpdateMs(UART_STATUS_UPDATE_MS),
    _heartbeatInterval(UART_HEARTBEAT_INTERVAL) {
}

/**
 * @brief Initialisiert die UART-Empfänger-Hardware
 */
bool UARTReceiver::begin() {
    if (_debugSerial) {
        _debugSerial->println("=== UARTReceiver Hardware-Initialisierung ===");
    }
    
    // LED-Pin initialisieren (falls vorhanden)
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
    
    // Kurze Pause für Hardware-Stabilisierung
    delay(100);
    
    // Eingangspuffer leeren
    clearBuffer();
    
    // Funktionstest
    if (_debugSerial) {
        _debugSerial->println("Führe UART Funktionstest durch...");
    }
    
    // Test-String senden
    String testString = "UART_TEST";
    _serial->print(testString);
    _serial->flush();
    
    delay(50);
    
    // Prüfen ob Daten zurückkommen (nur bei Loopback-Verdrahtung)
    if (_serial->available() > 0) {
        if (_debugSerial) {
            _debugSerial->println("UART Loopback erkannt - Test erfolgreich");
        }
        // Empfangene Daten löschen
        clearBuffer();
    } else {
        if (_debugSerial) {
            _debugSerial->println("Kein UART Loopback - normale Betriebssituation");
        }
    }
    
    // Zeitstempel initialisieren
    unsigned long currentTime = millis();
    _lastDataReceived = currentTime;
    _lastStatusUpdate = currentTime;
    _lastTimeoutMessage = currentTime;
    _lastHeartbeat = currentTime;
    
    _initialized = true;
    _systemReady = true;
    
    if (_debugSerial) {
        _debugSerial->println("Hardware-Initialisierung abgeschlossen");
        _debugSerial->println("================================");
    }
    
    return true;
}

/**
 * @brief Hauptverarbeitungsschleife
 */
void UARTReceiver::process() {
    if (!_systemReady) {
        return;
    }
    
    // UART-Daten verarbeiten (höchste Priorität)
    if (_serial->available() > 0) {
        _lastDataReceived = millis();
        _dataReceivedSinceLastCheck = true;
        
        if (_binaryMode) {
            // Binärdaten-Modus - sammle komplette Payload
            while (_serial->available() > 0) {
                uint8_t inByte = _serial->read();
                _totalBytesReceived++;
                
                // Debug-Ausgabe (optional)
                if (_debugSerial) {
                    _debugSerial->print(inByte, HEX);
                    _debugSerial->print(" ");
                }
                
                // Füge Byte zum Puffer hinzu
                _binaryBuffer[_bufferIndex] = inByte;
                _bufferIndex++;
                
                // Prüfe auf komplette Payload (28 Bytes erwartet)
                if (_bufferIndex >= _expectedPayloadSize) {
                    if (_debugSerial) {
                        _debugSerial->println("\n[INFO] Komplette Payload empfangen");
                    }
                    
                    // Prüfe verschiedene Formate
                    bool messageProcessed = false;
                    
                    // Format 1: Device-Name-Prefix ("DEVICE_NAME: " + payload)
                    for (size_t i = 0; i < _bufferIndex - 1; i++) {
                        if (_binaryBuffer[i] == ':' && _binaryBuffer[i + 1] == ' ') {
                            size_t payloadStart = i + 2;
                            size_t payloadSize = _bufferIndex - payloadStart;
                            
                            if (payloadSize > 0) {
                                if (_debugSerial) {
                                    _debugSerial->println("[INFO] Bridge-Format erkannt");
                                }
                                processBinaryPayload(_binaryBuffer + payloadStart, payloadSize);
                                messageProcessed = true;
                                break;
                            }
                        }
                    }
                    
                    // Format 2: Direkte Payload (beginnt mit Sensor-Typ)
                    if (!messageProcessed) {
                        if (_debugSerial) {
                            _debugSerial->println("[INFO] Direct-Payload erkannt");
                        }
                        processBinaryPayload(_binaryBuffer, _bufferIndex);
                    }
                    
                    // Puffer zurücksetzen
                    _bufferIndex = 0;
                }
                
                // Pufferüberlauf verhindern
                if (_bufferIndex >= MAX_PAYLOAD_SIZE) {
                    if (_debugSerial) {
                        _debugSerial->println("ERROR: Buffer overflow!");
                    }
                    _bufferIndex = 0;
                }
            }
        } else {
            // Text-Modus (ursprüngliche Implementierung)
            while (_serial->available() > 0) {
                char inChar = (char)_serial->read();
                _totalBytesReceived++;
                
                if(inChar == '\n' || inChar == '\r') {
                    // Komplette Nachricht empfangen
                    if (_inputBuffer.length() > 0) {
                        _totalMessagesReceived++;
                        
                        // Debug-Information ausgeben
                        if (_debugSerial) {
                            _debugSerial->print("[MSG #");
                            _debugSerial->print(_totalMessagesReceived);
                            _debugSerial->print(", ");
                            _debugSerial->print(_inputBuffer.length());
                            _debugSerial->println(" Bytes]");
                            
                            _debugSerial->println("\n=== UART EMPFANGEN ===");
                            _debugSerial->print("Länge: ");
                            _debugSerial->print(_inputBuffer.length());
                            _debugSerial->println(" Bytes");
                            _debugSerial->println("Rohdaten:");
                            _debugSerial->println(_inputBuffer);
                            _debugSerial->println("=== ENDE UART ===");
                        }
                        
                        // Callback für alle Nachrichten
                        if (_messageCallback) {
                            _messageCallback(_inputBuffer);
                        }
                        
                        // Prüfe ob es JSON ist und verarbeite entsprechend
                        if (_inputBuffer.startsWith("{") && _inputBuffer.endsWith("}")) {
                            if (_debugSerial) {
                                _debugSerial->println("\n-> JSON erkannt - verarbeite als JSON");
                            }
                            processMessage(_inputBuffer);
                        } else {
                            if (_debugSerial) {
                                _debugSerial->println("\n-> Kein JSON - einfache Textausgabe");
                            }
                            
                            // Text-Callback
                            if (_textCallback) {
                                _textCallback(_inputBuffer);
                            }
                        }
                        
                        _inputBuffer = "";
                    }
                } else if (inChar != '\r') {  // Ignoriere \r Zeichen
                    _inputBuffer += inChar;
                    
                    // Pufferüberlauf verhindern
                    if(_inputBuffer.length() > _bufferSize) {
                        if (_debugSerial) {
                            _debugSerial->println("ERROR:BUFFER_OVERFLOW");
                            _debugSerial->print("Buffer content start: ");
                            _debugSerial->println(_inputBuffer.substring(0, 100));
                        }
                        _inputBuffer = "";
                    }
                }
            }
        }
    }
    
    // Timeout-Prüfung
    checkDataTimeout();
    
    // Payload-Timeout-Prüfung (nur im Binärmodus)
    if (_binaryMode) {
        checkPayloadTimeout();
    }
    
    // Periodische Status-Anzeige
    displayPeriodicStatus();
    
    // Heartbeat senden
    sendHeartbeat();
}

/**
 * @brief Beendet die UART-Verbindung
 */
void UARTReceiver::end() {
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
 * @brief Verarbeitet eine empfangene JSON-Nachricht
 */
void UARTReceiver::processMessage(const String& message) {
    // LED blinken lassen bei Nachrichtenempfang
    if (_ledPin >= 0) {
        digitalWrite(_ledPin, HIGH);
    }
    
    // JSON parsen
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, message);
    
    if(error) {
        if (_debugSerial) {
            _debugSerial->print("JSON_ERROR:");
            _debugSerial->print(error.c_str());
            _debugSerial->print(" - Message length: ");
            _debugSerial->println(message.length());
            _debugSerial->print("First 200 chars: ");
            _debugSerial->println(message.substring(0, 200));
        }
        
        if (_ledPin >= 0) {
            digitalWrite(_ledPin, LOW);
        }
        return;
    }
    
    if (_debugSerial) {
        _debugSerial->println("JSON successfully parsed!");
    }
    
    // Message-ID extrahieren für ACK
    if(doc.containsKey("msg_id")) {
        long msgId = doc["msg_id"];
        if (_debugSerial) {
            _debugSerial->print("ACK:");
            _debugSerial->println(msgId);
        }
    }
    
    // JSON-Callback aufrufen
    if (_jsonCallback) {
        _jsonCallback(doc.as<JsonObject>());
    }
    
    // Daten verarbeiten
    if(doc["type"] == "uplink_data") {
        processUplinkData(doc["data"]);
    } else {
        if (_debugSerial) {
            _debugSerial->print("Unknown message type: ");
            _debugSerial->println(doc["type"].as<String>());
        }
    }
    
    if (_ledPin >= 0) {
        digitalWrite(_ledPin, LOW);
    }
}

/**
 * @brief Verarbeitet Uplink-Daten
 */
void UARTReceiver::processUplinkData(JsonObject data) {
    if (!_debugSerial) return;
    
    _debugSerial->println("=== UPLINK DATA RECEIVED ===");
    
    if(data.containsKey("dev_eui")) {
        _debugSerial->print("Device EUI: ");
        _debugSerial->println(data["dev_eui"].as<String>());
    }
    
    if(data.containsKey("data_hex")) {
        _debugSerial->print("Payload (HEX): ");
        _debugSerial->println(data["data_hex"].as<String>());
    }
    
    if(data.containsKey("data_size")) {
        _debugSerial->print("Payload Size: ");
        _debugSerial->print(data["data_size"].as<int>());
        _debugSerial->println(" bytes");
    }
    
    if(data.containsKey("rssi")) {
        _debugSerial->print("RSSI: ");
        _debugSerial->print(data["rssi"].as<int>());
        _debugSerial->println(" dBm");
    }
    
    if(data.containsKey("snr")) {
        _debugSerial->print("SNR: ");
        _debugSerial->print(data["snr"].as<float>());
        _debugSerial->println(" dB");
    }
    
    if(data.containsKey("frequency")) {
        _debugSerial->print("Frequency: ");
        _debugSerial->print(data["frequency"].as<long>());
        _debugSerial->println(" Hz");
    }
    
    _debugSerial->println("=== END DATA ===");
}

/**
 * @brief Prüft Timeout für Datenempfang
 */
void UARTReceiver::checkDataTimeout() {
    unsigned long currentTime = millis();
    
    // Prüfen ob Timeout erreicht wurde
    if ((currentTime - _lastDataReceived) >= _timeoutMs) {
        // Nur alle _timeoutMs eine Meldung ausgeben
        if ((currentTime - _lastTimeoutMessage) >= _timeoutMs) {
            if (_debugSerial) {
                _debugSerial->print("[INFO] Keine Daten seit ");
                _debugSerial->print((currentTime - _lastDataReceived) / 1000);
                _debugSerial->println(" Sekunden");
            }
            
            // Timeout-Callback aufrufen
            if (_timeoutCallback) {
                _timeoutCallback(currentTime - _lastDataReceived);
            }
            
            _lastTimeoutMessage = currentTime;
        }
    }
}

/**
 * @brief Zeigt periodische Statistik-Informationen an
 */
void UARTReceiver::displayPeriodicStatus() {
    unsigned long currentTime = millis();
    
    // Status-Update nur alle _statusUpdateMs
    if ((currentTime - _lastStatusUpdate) >= _statusUpdateMs) {
        // Nur anzeigen wenn seit dem letzten Check Daten empfangen wurden
        if (_dataReceivedSinceLastCheck) {
            if (_debugSerial) {
                _debugSerial->print("\n[STATISTIK] Nachrichten: ");
                _debugSerial->print(_totalMessagesReceived);
                _debugSerial->print(", Bytes gesamt: ");
                _debugSerial->print(_totalBytesReceived);
                _debugSerial->print(", Uptime: ");
                _debugSerial->print(currentTime / 1000);
                _debugSerial->println(" s");
            }
            
            // Status-Callback aufrufen
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
void UARTReceiver::sendHeartbeat() {
    if(millis() - _lastHeartbeat > _heartbeatInterval) {
        if (_debugSerial) {
            _debugSerial->println("HEARTBEAT");
        }
        _lastHeartbeat = millis();
    }
}

/**
 * @brief Setzt Callback-Funktion für alle empfangenen Nachrichten
 */
void UARTReceiver::setMessageCallback(UARTMessageCallback callback) {
    _messageCallback = callback;
}

/**
 * @brief Setzt Callback-Funktion für JSON-Daten
 */
void UARTReceiver::setJSONCallback(UARTJSONCallback callback) {
    _jsonCallback = callback;
}

/**
 * @brief Setzt Callback-Funktion für Text-Daten
 */
void UARTReceiver::setTextCallback(UARTTextCallback callback) {
    _textCallback = callback;
}

/**
 * @brief Setzt Callback-Funktion für Timeout-Events
 */
void UARTReceiver::setTimeoutCallback(UARTTimeoutCallback callback) {
    _timeoutCallback = callback;
}

/**
 * @brief Setzt Callback-Funktion für Status-Updates
 */
void UARTReceiver::setStatusCallback(UARTStatusCallback callback) {
    _statusCallback = callback;
}

/**
 * @brief Konfiguriert die Puffergröße
 */
void UARTReceiver::setBufferSize(size_t size) {
    _bufferSize = size;
}

/**
 * @brief Konfiguriert das Timeout für Datenempfang
 */
void UARTReceiver::setTimeout(unsigned long timeoutMs) {
    _timeoutMs = timeoutMs;
}

/**
 * @brief Konfiguriert das Intervall für Status-Updates
 */
void UARTReceiver::setStatusUpdateInterval(unsigned long intervalMs) {
    _statusUpdateMs = intervalMs;
}

/**
 * @brief Konfiguriert das Heartbeat-Intervall
 */
void UARTReceiver::setHeartbeatInterval(unsigned long intervalMs) {
    _heartbeatInterval = intervalMs;
}

/**
 * @brief Testet verschiedene Baudraten
 */
void UARTReceiver::testBaudrates() {
    uint32_t baudraten[] = {9600, 19200, 38400, 57600, 115200, 230400, 460800};
    int anzahlBaudraten = sizeof(baudraten) / sizeof(baudraten[0]);
    
    if (_debugSerial) {
        _debugSerial->println("=== BAUDRATE TEST ===");
    }
    
    for (int i = 0; i < anzahlBaudraten; i++) {
        if (_debugSerial) {
            _debugSerial->print("Teste Baudrate: ");
            _debugSerial->println(baudraten[i]);
        }
        
        // UART mit neuer Baudrate initialisieren
        _serial->end();
        delay(100);
        _serial->begin(baudraten[i], SERIAL_8N1);
        delay(100);
        
        // Puffer leeren
        clearBuffer();
        
        // 2 Sekunden warten und schauen was ankommt
        unsigned long startTime = millis();
        bool dataReceived = false;
        
        while (millis() - startTime < 2000) {
            if (_serial->available() > 0) {
                dataReceived = true;
                char c = _serial->read();
                if (_debugSerial) {
                    _debugSerial->print("Empfangen: 0x");
                    if (c < 16) _debugSerial->print("0");
                    _debugSerial->print(c, HEX);
                    if (c >= 32 && c <= 126) {
                        _debugSerial->print(" ('");
                        _debugSerial->print(c);
                        _debugSerial->println("')");
                    } else {
                        _debugSerial->println();
                    }
                }
            }
        }
        
        if (!dataReceived && _debugSerial) {
            _debugSerial->println("Keine Daten empfangen");
        }
        
        if (_debugSerial) {
            _debugSerial->println();
        }
    }
    
    // Zurück zur ursprünglichen Baudrate
    _serial->end();
    delay(100);
    _serial->begin(_baudrate, SERIAL_8N1);
    
    if (_debugSerial) {
        _debugSerial->println("Zurück zur ursprünglichen Baudrate");
        _debugSerial->println("=== BAUDRATE TEST ENDE ===");
    }
}

/**
 * @brief Gibt aktuelle Statistiken zurück
 */
void UARTReceiver::getStatistics(uint32_t& messages, uint32_t& bytes, unsigned long& uptime) {
    messages = _totalMessagesReceived;
    bytes = _totalBytesReceived;
    uptime = millis();
}

/**
 * @brief Prüft ob System bereit ist
 */
bool UARTReceiver::isReady() const {
    return _systemReady;
}

/**
 * @brief Zeigt Systeminformationen an
 */
void UARTReceiver::displaySystemInfo() {
    if (!_debugSerial) return;
    
    _debugSerial->println("\n=== UARTReceiver Library Info ===");
    _debugSerial->println("Funktion: UART-zu-USB Bridge");
    _debugSerial->print("Build: ");
    _debugSerial->print(__DATE__);
    _debugSerial->print(" ");
    _debugSerial->println(__TIME__);
    _debugSerial->println("");
    
    _debugSerial->println("=== Konfiguration ===");
    _debugSerial->print("UART Baudrate: ");
    _debugSerial->print(_baudrate);
    _debugSerial->println(" Baud");
    _debugSerial->print("Daten-Timeout: ");
    _debugSerial->print(_timeoutMs / 1000);
    _debugSerial->println(" Sekunden");
    _debugSerial->print("Puffergröße: ");
    _debugSerial->print(_bufferSize);
    _debugSerial->println(" Bytes");
    _debugSerial->println("");
    
    _debugSerial->println("=== Pin-Zuordnung ===");
    _debugSerial->print("UART TX: Pin ");
    _debugSerial->println(_txPin);
    _debugSerial->print("UART RX: Pin ");
    _debugSerial->println(_rxPin);
    if (_ledPin >= 0) {
        _debugSerial->print("Status LED: Pin ");
        _debugSerial->println(_ledPin);
    }
    _debugSerial->println("");
    
    _debugSerial->println("=== Status ===");
    _debugSerial->println("Bereit für Datenempfang...");
    _debugSerial->println("JSON-Verarbeitung aktiviert");
    _debugSerial->println("=====================================\n");
}

/**
 * @brief Sendet Testdaten über UART
 */
void UARTReceiver::sendTestData(const String& testData) {
    if (_initialized) {
        _serial->println(testData);
        _serial->flush();
        
        if (_debugSerial) {
            _debugSerial->print("Testdaten gesendet: ");
            _debugSerial->println(testData);
        }
    }
}

/**
 * @brief Leert den Eingangspuffer
 */
void UARTReceiver::clearBuffer() {
    _inputBuffer = "";
    _bufferIndex = 0;
    
    // Hardware-Puffer leeren
    while (_serial->available() > 0) {
        _serial->read();
    }
}

/**
 * @brief Verarbeitet eine empfangene Binär-Payload
 */
void UARTReceiver::processBinaryPayload(const uint8_t* payload, size_t size) {
    if (_debugSerial) {
        _debugSerial->println("=== BINÄRE PAYLOAD EMPFANGEN ===");
        _debugSerial->print("Größe: ");
        _debugSerial->println(size);
        
        // Hex-Dump ausgeben
        for (size_t i = 0; i < size; i++) {
            if (payload[i] < 16) _debugSerial->print("0");
            _debugSerial->print(payload[i], HEX);
            _debugSerial->print(" ");
        }
        _debugSerial->println();
        
        // Vollständige Daten ausgeben
        _debugSerial->println("\n[VOLLSTÄNDIGE DEKODIERUNG]");
        size_t index = 0;
        while (index < size) {
            char typ = (char)payload[index];
            index++;
            
            if (typ == 'T') {  // Temperatur
                if (index + 8 <= size) {
                    float temp1 = readFloat(payload, index);
                    index += 4;
                    float temp2 = readFloat(payload, index);
                    index += 4;
                    
                    _debugSerial->print("Temp1: ");
                    _debugSerial->print(temp1);
                    _debugSerial->print(", Temp2: ");
                    _debugSerial->println(temp2);
                }
            } else if (typ == 'D') {  // Deflection
                if (index + 4 <= size) {
                    float deflection = readFloat(payload, index);
                    index += 4;
                    
                    _debugSerial->print("Deflection: ");
                    _debugSerial->println(deflection);
                }
            } else if (typ == 'P') {  // Pressure and Misc
                if (index + 4 <= size) {
                    float pressure = readFloat(payload, index);
                    index += 4;
                    
                    _debugSerial->print("Pressure: ");
                    _debugSerial->println(pressure);
                }
                if (index + 8 <= size) {
                    // Next section - presumably Misc
                    float misc1 = readFloat(payload, index);
                    index += 4;
                    float misc2 = readFloat(payload, index);
                    index += 4;
                    
                    _debugSerial->print("Misc1: ");
                    _debugSerial->print(misc1);
                    _debugSerial->print(", Misc2: ");
                    _debugSerial->println(misc2);
                }
            }
        }
        _debugSerial->println("[ENDE VOLLSTÄNDIGE DEKODIERUNG]");
    }
    
    // Dekodiere Sensordaten (basierend auf deiner Endnode)
    size_t index = 0;
    while (index < size) {
        char typ = (char)payload[index];
        index++;
        
        if (typ == 'T') {  // Temperatur
            if (index + 8 <= size) {
                float temp1 = readFloat(payload, index);
                index += 4;
                float temp2 = readFloat(payload, index);
                index += 4;
                
                if (_debugSerial) {
                    _debugSerial->print("Temp1: ");
                    _debugSerial->println(temp1);
                    _debugSerial->print("Temp2: ");
                    _debugSerial->println(temp2);
                }
            } else {
                if (_debugSerial) {
                    _debugSerial->println("ERROR: Unvollständige Temperaturdaten");
                }
                break;
            }
        } else if (typ == 'D') {  // Deflection
            if (index + 4 <= size) {
                float deflection = readFloat(payload, index);
                index += 4;
                
                if (_debugSerial) {
                    _debugSerial->print("Deflection: ");
                    _debugSerial->println(deflection);
                }
            } else {
                if (_debugSerial) {
                    _debugSerial->println("ERROR: Unvollständige Deflection-Daten");
                }
                break;
            }
        } else if (typ == 'P') {  // Pressure - variabel 1 oder 2 Float-Werte
            // Prüfe ob P+S Kombination (P hat nur 1 Float wenn gefolgt von S)
            if (index + 4 < size && payload[index + 4] == 'S') {
                // P+S Kombination: P hat nur 1 Float
                if (index + 4 <= size) {
                    float pressure = readFloat(payload, index);
                    index += 4;
                    
                    if (_debugSerial) {
                        _debugSerial->print("Pressure: ");
                        _debugSerial->println(pressure);
                    }
                } else {
                    if (_debugSerial) {
                        _debugSerial->println("ERROR: Unvollständige Pressure-Daten (P+S)");
                    }
                    break;
                }
            } else {
                // Standard P Block mit 2 Float-Werten
                if (index + 8 <= size) {
                    float pressure1 = readFloat(payload, index);
                    index += 4;
                    float pressure2 = readFloat(payload, index);
                    index += 4;
                    
                    if (_debugSerial) {
                        _debugSerial->print("Pressure1: ");
                        _debugSerial->println(pressure1);
                        _debugSerial->print("Pressure2: ");
                        _debugSerial->println(pressure2);
                    }
                } else {
                    if (_debugSerial) {
                        _debugSerial->println("ERROR: Unvollständige Pressure-Daten");
                    }
                    break;
                }
            }
        } else if (typ == 'S') {  // Sonstiges - 2 Float-Werte
            if (index + 8 <= size) {
                float misc1 = readFloat(payload, index);
                index += 4;
                float misc2 = readFloat(payload, index);
                index += 4;
                
                if (_debugSerial) {
                    _debugSerial->print("Misc1: ");
                    _debugSerial->println(misc1);
                    _debugSerial->print("Misc2: ");
                    _debugSerial->println(misc2);
                }
            } else {
                if (_debugSerial) {
                    _debugSerial->println("ERROR: Unvollständige Sonstiges-Daten");
                }
                break;
            }
        } else {
            if (_debugSerial) {
                _debugSerial->print("Unbekannter Typ: ");
                _debugSerial->println(typ);
            }
            break;  // Stoppe Verarbeitung bei unbekanntem Typ
        }
    }
    
    // Callback für verarbeitete Daten
    if (_binaryCallback) {
        _binaryCallback(payload, size);
    }
    
    _totalMessagesReceived++;
    _lastBinaryDataReceived = millis();
    
    if (_debugSerial) {
        _debugSerial->println("=== ENDE BINÄRE PAYLOAD ===");
    }
}

/**
 * @brief Konvertiert 4 Bytes zu Float (Little Endian)
 */
float UARTReceiver::readFloat(const uint8_t* data, size_t startIndex) {
    // Konvertiere 4 Bytes zu Float (Little Endian)
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
void UARTReceiver::checkPayloadTimeout() {
    // Prüfe ob unvollständige Daten zu lange im Puffer sind
    if (_bufferIndex > 0 && (millis() - _lastDataReceived) > PAYLOAD_TIMEOUT) {
        if (_debugSerial) {
            _debugSerial->println("WARNING: Unvollständige Payload - Puffer zurückgesetzt");
        }
        _bufferIndex = 0;
    }
}

/**
 * @brief Setzt Callback-Funktion für Binärdaten
 */
void UARTReceiver::setBinaryCallback(UARTBinaryCallback callback) {
    _binaryCallback = callback;
}

/**
 * @brief Konfiguriert die erwartete Binärdaten-Payload-Größe
 */
void UARTReceiver::setExpectedPayloadSize(size_t size) {
    _expectedPayloadSize = size;
}

/**
 * @brief Aktiviert/Deaktiviert den Binärdaten-Modus
 */
void UARTReceiver::setBinaryMode(bool enabled) {
    _binaryMode = enabled;
    if (enabled) {
        _bufferIndex = 0;
        _inputBuffer = "";
        if (_debugSerial) {
            _debugSerial->println("Binärdaten-Modus aktiviert");
        }
    } else {
        if (_debugSerial) {
            _debugSerial->println("Text-Modus aktiviert");
        }
    }
}

