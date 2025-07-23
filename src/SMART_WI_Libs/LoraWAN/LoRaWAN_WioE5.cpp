/**
 * LoRaWAN WioE5 Library Implementation
 * 
 * Implementierung der LoRaWAN WioE5 Library für die Kommunikation
 * mit dem Grove Wio-E5 LoRaWAN Modul über UART-Schnittstelle.
 * 
 * @author: Automatisch generiert
 * @date: 2025-01-04
 */

#include "LoRaWAN_WioE5.h"

// ================================================================
// KONSTRUKTOR UND DESTRUKTOR
// ================================================================

LoRaWAN_WioE5::LoRaWAN_WioE5(HardwareSerial* serial, Stream* debugSerial) {
    _serial = serial;
    _debugSerial = debugSerial;
    
    // Struktur-Initialisierung
    memset(&_config, 0, sizeof(_config));
    memset(&_status, 0, sizeof(_status));
    memset(_responseBuffer, 0, sizeof(_responseBuffer));
    
    // Standard-Konfiguration setzen
    _config.mode = LORAWAN_MODE_ABP;
    _config.deviceClass = LORAWAN_CLASS_A;
    _config.region = LORAWAN_REGION_EU868;
    _config.dataRate = LORAWAN_DR5;
    _config.adaptiveDataRate = true;
    _config.customFrequencies[0] = LORAWAN_FREQ_CH0;
    _config.customFrequencies[1] = LORAWAN_FREQ_CH1;
    _config.customFrequencies[2] = LORAWAN_FREQ_CH2;
    
    // Status initialisieren
    _status.moduleReady = false;
    _status.networkJoined = false;
    _status.messageCounter = 0;
    _status.lastSendTime = 0;
    _status.hasDownlink = false;
    _status.downlinkSize = 0;
    _status.downlinkPort = 0;
}

LoRaWAN_WioE5::~LoRaWAN_WioE5() {
    // Destruktor - aktuell nichts zu bereinigen
}

// ================================================================
// PRIVATE HILFSMETHODEN
// ================================================================

bool LoRaWAN_WioE5::sendATCommand(const char* command, const char* expectedResponse, 
                                 unsigned long timeout) {
    return sendATCommandSilent(command, expectedResponse, timeout);
}

bool LoRaWAN_WioE5::sendATCommandSilent(const char* command, const char* expectedResponse, 
                                       unsigned long timeout) {
    if (!_serial) return false;
    
    // Buffer leeren
    memset(_responseBuffer, 0, sizeof(_responseBuffer));
    clearInputBuffer();
    
    // Kommando senden (ohne Debug-Ausgabe)
    _serial->print(command);
    _serial->flush();
    
    unsigned long startTime = millis();
    int bufferIndex = 0;
    bool dataReceived = false;
    
    // Auf Antwort warten
    while (millis() - startTime < timeout) {
        while (_serial->available() > 0 && bufferIndex < sizeof(_responseBuffer) - 1) {
            char c = _serial->read();
            _responseBuffer[bufferIndex++] = c;
            _responseBuffer[bufferIndex] = '\0';
            dataReceived = true;
        }
        
        // Prüfen ob erwartete Antwort empfangen
        if (expectedResponse != nullptr && strstr(_responseBuffer, expectedResponse) != nullptr) {
            return true;
        }
        
        delay(10);
    }
    
    return false;
}

bool LoRaWAN_WioE5::waitForResponse(const char* expectedResponse, unsigned long timeout) {
    if (!_serial || !expectedResponse) return false;
    
    unsigned long startTime = millis();
    int bufferIndex = strlen(_responseBuffer);
    
    while (millis() - startTime < timeout) {
        while (_serial->available() > 0 && bufferIndex < sizeof(_responseBuffer) - 1) {
            char c = _serial->read();
            _responseBuffer[bufferIndex++] = c;
            _responseBuffer[bufferIndex] = '\0';
            
            // Debug-Ausgabe
            if (_debugSerial) {
                if (c >= 32 && c <= 126) {
                    _debugSerial->print(c);
                } else {
                    _debugSerial->print("[");
                    _debugSerial->print((int)c);
                    _debugSerial->print("]");
                }
            }
        }
        
        // Prüfen ob erwartete Antwort empfangen
        if (strstr(_responseBuffer, expectedResponse) != nullptr) {
            if (_debugSerial) {
                _debugSerial->println();
                _debugSerial->print("[RX] SUCCESS: ");
                _debugSerial->println(expectedResponse);
            }
            return true;
        }
        
        delay(10);
    }
    
    return false;
}

void LoRaWAN_WioE5::clearInputBuffer() {
    if (!_serial) return;
    
    while (_serial->available() > 0) {
        _serial->read();
    }
}

void LoRaWAN_WioE5::debugPrint(const char* message) {
    if (_debugSerial) {
        _debugSerial->print(message);
    }
}

void LoRaWAN_WioE5::debugPrintln(const char* message) {
    if (_debugSerial) {
        _debugSerial->println(message);
    }
}

// ================================================================
// PRIVATE KONFIGURATIONSMETHODEN
// ================================================================

bool LoRaWAN_WioE5::setMode(LoRaWAN_Mode mode) {
    const char* modeStr = (mode == LORAWAN_MODE_OTAA) ? "LWOTAA" : "LWABP";
    char command[32];
    sprintf(command, "AT+MODE=%s\r\n", modeStr);
    
    char expectedResponse[32];
    sprintf(expectedResponse, "+MODE: %s", modeStr);
    
    return sendATCommandSilent(command, expectedResponse);
}

bool LoRaWAN_WioE5::setRegion(LoRaWAN_Region region) {
    const char* regionStr;
    switch (region) {
        case LORAWAN_REGION_EU868: regionStr = "EU868"; break;
        case LORAWAN_REGION_US915: regionStr = "US915"; break;
        case LORAWAN_REGION_AS923: regionStr = "AS923"; break;
        default: return false;
    }
    
    char command[32];
    sprintf(command, "AT+DR=%s\r\n", regionStr);
    
    char expectedResponse[32];
    sprintf(expectedResponse, "+DR: %s", regionStr);
    
    return sendATCommandSilent(command, expectedResponse);
}

bool LoRaWAN_WioE5::setClass(LoRaWAN_Class deviceClass) {
    const char* classStr;
    switch (deviceClass) {
        case LORAWAN_CLASS_A: classStr = "A"; break;
        case LORAWAN_CLASS_B: classStr = "B"; break;
        case LORAWAN_CLASS_C: classStr = "C"; break;
        default: return false;
    }
    
    char command[32];
    sprintf(command, "AT+CLASS=%s\r\n", classStr);
    
    char expectedResponse[32];
    sprintf(expectedResponse, "+CLASS: %s", classStr);
    
    return sendATCommandSilent(command, expectedResponse);
}

bool LoRaWAN_WioE5::setDataRate(LoRaWAN_DataRate dataRate) {
    char command[32];
    sprintf(command, "AT+DR=%d\r\n", (int)dataRate);
    
    char expectedResponse[32];
    sprintf(expectedResponse, "+DR: DR%d", (int)dataRate);
    
    return sendATCommandSilent(command, expectedResponse);
}

bool LoRaWAN_WioE5::setAdaptiveDataRate(bool enabled) {
    const char* command = enabled ? "AT+ADR=ON\r\n" : "AT+ADR=OFF\r\n";
    const char* expectedResponse = enabled ? "+ADR: ON" : "+ADR: OFF";
    
    return sendATCommandSilent(command, expectedResponse);
}

bool LoRaWAN_WioE5::setCustomFrequencies() {
    // EU868 Standard-Kanäle werden automatisch gesetzt
    return true;
}

// ================================================================
// SPREADING FACTOR KONFIGURATION
// ================================================================

bool LoRaWAN_WioE5::setSpreadingFactor(int sf) {
    // Prüfe gültigen SF-Bereich (7-12)
    if (sf < 7 || sf > 12) {
        debugPrintln("[ERROR] Ungültiger Spreading Factor. Gültig: 7-12");
        return false;
    }
    
    // AT-Kommando für Spreading Factor erstellen
    char command[32];
    sprintf(command, "AT+DR=%d\r\n", 12 - sf); // DR ist umgekehrt zu SF: SF7=DR5, SF12=DR0
    
    // Erwartete Antwort
    char expectedResponse[32];
    sprintf(expectedResponse, "+DR: DR%d", 12 - sf);
    
    debugPrint("[INFO] Setze Spreading Factor SF");
    debugPrint(String(sf).c_str());
    debugPrint(" (entspricht DR");
    debugPrint(String(12 - sf).c_str());
    debugPrintln(")");
    
    // Kommando senden
    if (sendATCommand(command, expectedResponse)) {
        debugPrint("[OK] Spreading Factor SF");
        debugPrint(String(sf).c_str());
        debugPrintln(" erfolgreich gesetzt");
        return true;
    } else {
        debugPrint("[ERROR] Konnte Spreading Factor SF");
        debugPrint(String(sf).c_str());
        debugPrintln(" nicht setzen");
        return false;
    }
}

// ================================================================
// ABP-SPEZIFISCHE METHODEN
// ================================================================

bool LoRaWAN_WioE5::setDeviceAddr(const char* deviceAddr) {
    char command[64];
    sprintf(command, "AT+ID=DevAddr,%s\r\n", deviceAddr);
    return sendATCommandSilent(command, "+ID: DevAddr");
}

bool LoRaWAN_WioE5::setNetworkSessionKey(const char* networkSessionKey) {
    char command[64];
    sprintf(command, "AT+KEY=NWKSKEY,%s\r\n", networkSessionKey);
    return sendATCommandSilent(command, "+KEY: NWKSKEY");
}

bool LoRaWAN_WioE5::setAppSessionKey(const char* appSessionKey) {
    char command[64];
    sprintf(command, "AT+KEY=APPSKEY,%s\r\n", appSessionKey);
    return sendATCommandSilent(command, "+KEY: APPSKEY");
}

// ================================================================
// OTAA-SPEZIFISCHE METHODEN
// ================================================================

bool LoRaWAN_WioE5::setDeviceEUI(const char* deviceEUI) {
    char command[64];
    sprintf(command, "AT+ID=DevEui,%s\r\n", deviceEUI);
    return sendATCommandSilent(command, "+ID: DevEui");
}

bool LoRaWAN_WioE5::setApplicationEUI(const char* applicationEUI) {
    char command[64];
    sprintf(command, "AT+ID=AppEui,%s\r\n", applicationEUI);
    return sendATCommandSilent(command, "+ID: AppEui");
}

bool LoRaWAN_WioE5::setApplicationKey(const char* applicationKey) {
    char command[64];
    sprintf(command, "AT+KEY=APPKEY,%s\r\n", applicationKey);
    return sendATCommandSilent(command, "+KEY: APPKEY");
}

// ================================================================
// ÖFFENTLICHE METHODEN - INITIALISIERUNG
// ================================================================

bool LoRaWAN_WioE5::begin(unsigned long baudRate) {
    if (!_serial) return false;
    
    // Silent initialization - reduce logs
    _serial->begin(baudRate);
    delay(1000);
    
    // Test AT-Verbindung
    if (!sendATCommandSilent("AT\r\n", "+AT: OK", 2000)) {
        debugPrintln("[ERROR] Wio-E5 Modul antwortet nicht");
        return false;
    }
    
    // Firmware-Version lesen (silent)
    getFirmwareVersion(_status.firmwareVersion);
    
    // Echte DevEUI lesen (silent)
    getRealDeviceEUI(_status.realDeviceEUI);
    
    _status.moduleReady = true;
    
    return true;
}

bool LoRaWAN_WioE5::configureABP(const char* deviceAddr, const char* networkSessionKey, 
                                 const char* appSessionKey) {
    // Parameter speichern
    strncpy(_config.deviceAddr, deviceAddr, sizeof(_config.deviceAddr) - 1);
    strncpy(_config.networkSessionKey, networkSessionKey, sizeof(_config.networkSessionKey) - 1);
    strncpy(_config.appSessionKey, appSessionKey, sizeof(_config.appSessionKey) - 1);
    _config.mode = LORAWAN_MODE_ABP;
    
    // Modus setzen
    if (!setMode(LORAWAN_MODE_ABP)) {
        debugPrintln("[ERROR] Konnte ABP-Modus nicht setzen");
        return false;
    }
    
    // Parameter konfigurieren
    if (!setDeviceAddr(deviceAddr)) {
        debugPrintln("[ERROR] Konnte DevAddr nicht setzen");
        return false;
    }
    
    if (!setNetworkSessionKey(networkSessionKey)) {
        debugPrintln("[ERROR] Konnte NwkSKey nicht setzen");
        return false;
    }
    
    if (!setAppSessionKey(appSessionKey)) {
        debugPrintln("[ERROR] Konnte AppSKey nicht setzen");
        return false;
    }
    
    return true;
}

bool LoRaWAN_WioE5::configureOTAA(const char* deviceEUI, const char* applicationEUI, 
                                  const char* applicationKey) {
    debugPrintln("=== Konfiguriere OTAA-Modus ===");
    
    // Parameter speichern
    strncpy(_config.deviceEUI, deviceEUI, sizeof(_config.deviceEUI) - 1);
    strncpy(_config.applicationEUI, applicationEUI, sizeof(_config.applicationEUI) - 1);
    strncpy(_config.applicationKey, applicationKey, sizeof(_config.applicationKey) - 1);
    _config.mode = LORAWAN_MODE_OTAA;
    
    // Modus setzen
    if (!setMode(LORAWAN_MODE_OTAA)) {
        debugPrintln("[ERROR] Konnte OTAA-Modus nicht setzen");
        return false;
    }
    debugPrintln("[OK] OTAA-Modus gesetzt");
    
    // Parameter konfigurieren
    if (!setDeviceEUI(deviceEUI)) {
        debugPrintln("[ERROR] Konnte DevEUI nicht setzen");
        return false;
    }
    debugPrintln("[OK] DevEUI gesetzt");
    
    if (!setApplicationEUI(applicationEUI)) {
        debugPrintln("[ERROR] Konnte AppEUI nicht setzen");
        return false;
    }
    debugPrintln("[OK] AppEUI gesetzt");
    
    if (!setApplicationKey(applicationKey)) {
        debugPrintln("[ERROR] Konnte AppKey nicht setzen");
        return false;
    }
    debugPrintln("[OK] AppKey gesetzt");
    
    debugPrintln("[OK] OTAA-Konfiguration erfolgreich");
    return true;
}

bool LoRaWAN_WioE5::configureAdvanced(LoRaWAN_Region region, LoRaWAN_Class deviceClass,
                                      LoRaWAN_DataRate dataRate, bool adaptiveDataRate) {
    // Parameter speichern
    _config.region = region;
    _config.deviceClass = deviceClass;
    _config.dataRate = dataRate;
    _config.adaptiveDataRate = adaptiveDataRate;
    
    // Region setzen
    if (!setRegion(region)) {
        debugPrintln("[ERROR] Konnte Region nicht setzen");
        return false;
    }
    
    // Klasse setzen (silent if successful)
    if (!setClass(deviceClass)) {
        debugPrintln("[WARNING] Konnte Klasse nicht setzen - verwende Standard");
    }
    
    // Data Rate setzen (silent if successful)
    if (!setDataRate(dataRate)) {
        debugPrintln("[WARNING] Konnte Data Rate nicht setzen");
    }
    
    // Adaptive Data Rate setzen (silent if successful)
    if (!setAdaptiveDataRate(adaptiveDataRate)) {
        debugPrintln("[WARNING] Konnte ADR nicht setzen");
    }
    
    // Benutzerdefinierte Frequenzen setzen (silent)
    setCustomFrequencies();
    
    return true;
}

bool LoRaWAN_WioE5::setCustomFrequencies(float ch0, float ch1, float ch2) {
    _config.customFrequencies[0] = ch0;
    _config.customFrequencies[1] = ch1;
    _config.customFrequencies[2] = ch2;
    
    return setCustomFrequencies();
}

bool LoRaWAN_WioE5::configureWithDefaults(LoRaWAN_Mode mode) {
    debugPrintln("=== Konfiguriere mit Standard-Werten ===");
    
    bool success = true;
    
    if (mode == LORAWAN_MODE_ABP) {
        // ABP mit Standard-Werten konfigurieren
        if (!configureABP(LORAWAN_DEVICE_ADDR, LORAWAN_NETWORK_SESSION_KEY, LORAWAN_APP_SESSION_KEY)) {
            debugPrintln("[ERROR] ABP-Konfiguration mit Standard-Werten fehlgeschlagen");
            success = false;
        } else {
            debugPrintln("[OK] ABP mit Standard-Werten konfiguriert");
        }
    } else {
        // OTAA mit Standard-Werten konfigurieren
        if (!configureOTAA(LORAWAN_DEVICE_EUI, LORAWAN_APPLICATION_EUI, LORAWAN_APPLICATION_KEY)) {
            debugPrintln("[ERROR] OTAA-Konfiguration mit Standard-Werten fehlgeschlagen");
            success = false;
        } else {
            debugPrintln("[OK] OTAA mit Standard-Werten konfiguriert");
        }
    }
    
    // Erweiterte Konfiguration mit Standard-Werten
    if (!configureAdvanced(LORAWAN_DEFAULT_REGION, LORAWAN_DEFAULT_CLASS, 
                          LORAWAN_DEFAULT_DATA_RATE, LORAWAN_DEFAULT_ADR)) {
        debugPrintln("[WARNING] Erweiterte Konfiguration teilweise fehlgeschlagen");
    } else {
        debugPrintln("[OK] Erweiterte Konfiguration mit Standard-Werten gesetzt");
    }
    
    // Standard-Frequenzen setzen
    if (!setCustomFrequencies(LORAWAN_FREQUENCY_CH0, LORAWAN_FREQUENCY_CH1, LORAWAN_FREQUENCY_CH2)) {
        debugPrintln("[WARNING] Standard-Frequenzen konnten nicht vollständig gesetzt werden");
    } else {
        debugPrintln("[OK] Standard-Frequenzen gesetzt");
    }
    
    debugPrintln("[OK] Konfiguration mit Standard-Werten abgeschlossen");
    return success;
}

// ================================================================
// NETZWERK-VERBINDUNG
// ================================================================

bool LoRaWAN_WioE5::joinNetwork() {
    debugPrintln("=== LoRaWAN Netzwerk Join ===");
    
    if (_config.mode == LORAWAN_MODE_ABP) {
        debugPrintln("[INFO] ABP-Modus - kein Join erforderlich");
        _status.networkJoined = true;
        return true;
    }
    
    // Join-Versuch starten (OTAA)
    if (!sendATCommand("AT+JOIN\r\n", "Start", 3000)) {
        debugPrintln("[ERROR] Join-Kommando fehlgeschlagen");
        return false;
    }
    
    // Auf Join-Bestätigung warten
    debugPrintln("[INFO] Warte auf Netzwerk-Join...");
    
    unsigned long joinStartTime = millis();
    memset(_responseBuffer, 0, sizeof(_responseBuffer));
    int index = 0;
    
    while (millis() - joinStartTime < LORAWAN_JOIN_TIMEOUT_MS) {
        while (_serial->available() > 0 && index < sizeof(_responseBuffer) - 1) {
            char c = _serial->read();
            
            // Echo zum Debug-Serial
            if (_debugSerial) {
                if (c >= 32 && c <= 126) {
                    _debugSerial->print(c);
                } else {
                    _debugSerial->print("[");
                    _debugSerial->print((int)c);
                    _debugSerial->print("]");
                }
            }
            
            _responseBuffer[index++] = c;
            _responseBuffer[index] = '\0';
        }
        
        // Suche nach Erfolgs- oder Fehlermeldung
        if (strstr(_responseBuffer, "+JOIN: Network joined") != nullptr || 
            strstr(_responseBuffer, "+JOIN: Joined") != nullptr) {
            debugPrintln("\n[OK] Erfolgreich mit LoRaWAN Netzwerk verbunden");
            _status.networkJoined = true;
            return true;
        }
        
        if (strstr(_responseBuffer, "+JOIN: Join failed") != nullptr) {
            debugPrintln("\n[ERROR] Netzwerk-Join fehlgeschlagen");
            return false;
        }
        
        if (strstr(_responseBuffer, "+JOIN: Done") != nullptr) {
            debugPrintln("\n[ERROR] Join-Prozess beendet, aber kein Erfolg");
            return false;
        }
        
        delay(100);
    }
    
    debugPrintln("\n[ERROR] Timeout beim Warten auf Join-Antwort");
    return false;
}

bool LoRaWAN_WioE5::isNetworkJoined() {
    return _status.networkJoined;
}

// ================================================================
// NACHRICHTENVERSAND
// ================================================================

bool LoRaWAN_WioE5::sendMessage(const char* message, uint8_t port, bool confirmed) {
    if (!_status.moduleReady || !message) return false;
    
    debugPrint("[INFO] Sende Nachricht #");
    debugPrint(String(_status.messageCounter).c_str());
    debugPrint(": ");
    debugPrintln(message);
    
    // Nachricht in Hex umwandeln
    String hexMessage = "";
    for (int i = 0; i < strlen(message); i++) {
        char hex[3];
        sprintf(hex, "%02X", message[i]);
        hexMessage += hex;
    }
    
    return sendHexData(hexMessage.c_str(), port, confirmed);
}

bool LoRaWAN_WioE5::sendHexData(const char* hexData, uint8_t port, bool confirmed) {
    if (!_status.moduleReady || !hexData) return false;
    
    // Clear previous downlink data
    _status.hasDownlink = false;
    _status.downlinkSize = 0;
    _status.downlinkPort = 0;
    
    // AT-Kommando für Nachrichtenversand zusammenstellen
    char command[256];
    if (confirmed) {
        sprintf(command, "AT+CMSG=%s\r\n", hexData);
    } else {
        sprintf(command, "AT+MSGHEX=%s\r\n", hexData);
    }
    
    // Nachricht senden
    if (!sendATCommand(command, confirmed ? "+CMSG: Start" : "+MSGHEX: Start", 3000)) {
        debugPrintln("[ERROR] Nachricht konnte nicht gesendet werden");
        return false;
    }
    
    // Auf Sendbestätigung warten und auf Downlink prüfen
    memset(_responseBuffer, 0, sizeof(_responseBuffer));
    unsigned long startTime = millis();
    int index = 0;
    bool sendSuccess = false;
    
    while (millis() - startTime < LORAWAN_SEND_TIMEOUT_MS) {
        while (_serial->available() > 0 && index < sizeof(_responseBuffer) - 1) {
            char c = _serial->read();
            
            // Echo zum Debug-Serial
            if (_debugSerial) {
                if (c >= 32 && c <= 126) {
                    _debugSerial->print(c);
                } else {
                    _debugSerial->print("[");
                    _debugSerial->print((int)c);
                    _debugSerial->print("]");
                }
            }
            
            _responseBuffer[index++] = c;
            _responseBuffer[index] = '\0';
        }
        
        // Prüfe auf Sendbestätigung
        const char* expectedDone = confirmed ? "+CMSG: Done" : "+MSGHEX: Done";
        if (strstr(_responseBuffer, expectedDone) != nullptr) {
            sendSuccess = true;
            debugPrintln("[OK] Sendbestätigung erhalten");
            
            // Prüfe auf Downlink-Nachricht
            parseDownlinkMessage(_responseBuffer);
            break;
        }
        
        // Prüfe auf Fehler
        if (strstr(_responseBuffer, "ERROR") != nullptr) {
            debugPrintln("[ERROR] Sendung fehlgeschlagen");
            return false;
        }
        
        delay(100);
    }
    
    if (!sendSuccess) {
        debugPrintln("[ERROR] Timeout beim Warten auf Sendbestätigung");
        return false;
    }
    
    _status.messageCounter++;
    _status.lastSendTime = millis();
    
    // Zeige Downlink-Informationen an
    if (_status.hasDownlink) {
        debugPrint("[INFO] Downlink empfangen auf Port ");
        debugPrint(String(_status.downlinkPort).c_str());
        debugPrint(", Größe: ");
        debugPrint(String(_status.downlinkSize).c_str());
        debugPrintln(" bytes");
        
        if (_debugSerial) {
            _debugSerial->print("[DEBUG] Downlink Hex: ");
            for (size_t i = 0; i < _status.downlinkSize; i++) {
                if (_status.downlinkData[i] < 0x10) _debugSerial->print("0");
                _debugSerial->print(_status.downlinkData[i], HEX);
                _debugSerial->print(" ");
            }
            _debugSerial->println();
        }
    }
    
    debugPrintln("[OK] Nachricht erfolgreich gesendet");
    return true;
}

bool LoRaWAN_WioE5::sendBinaryData(const uint8_t* data, size_t length, uint8_t port, bool confirmed) {
    if (!data || length == 0) return false;
    
    // Binärdaten in Hex-String umwandeln
    String hexString = "";
    for (size_t i = 0; i < length; i++) {
        char hex[3];
        sprintf(hex, "%02X", data[i]);
        hexString += hex;
    }
    
    return sendHexData(hexString.c_str(), port, confirmed);
}

// ================================================================
// STATUS UND INFORMATION
// ================================================================

LoRaWAN_Status LoRaWAN_WioE5::getStatus() {
    return _status;
}

LoRaWAN_Config LoRaWAN_WioE5::getConfig() {
    return _config;
}

bool LoRaWAN_WioE5::getRealDeviceEUI(char* buffer) {
    if (!buffer) return false;
    
    // DevEUI-Abfrage mit erweitertem Timeout
    memset(_responseBuffer, 0, sizeof(_responseBuffer));
    _serial->print("AT+ID=DevEui\r\n");
    
    unsigned long startTime = millis();
    int index = 0;
    
    while (millis() - startTime < 8000 && index < sizeof(_responseBuffer) - 1) {
        while (_serial->available()) {
            char c = _serial->read();
            _responseBuffer[index++] = c;
            _responseBuffer[index] = '\0';
        }
        
        // Stoppe wenn komplette DevEUI empfangen
        if (strstr(_responseBuffer, "+ID: DevEui, ") != nullptr) {
            char* deveuiStart = strstr(_responseBuffer, "+ID: DevEui, ");
            if (deveuiStart != nullptr && strlen(deveuiStart) >= 36) {
                // Zusätzlich warten für eventuelle nachkommende Zeichen
                delay(500);
                while (_serial->available() && index < sizeof(_responseBuffer) - 1) {
                    char c = _serial->read();
                    _responseBuffer[index++] = c;
                    _responseBuffer[index] = '\0';
                }
                break;
            }
        }
        
        delay(10);
    }
    
    if (strstr(_responseBuffer, "+ID: DevEui") != nullptr) {
        // DevEUI extrahieren
        char* deveuiStart = strstr(_responseBuffer, "+ID: DevEui, ");
        if (deveuiStart != nullptr) {
            deveuiStart += 13; // Überspringe "+ID: DevEui, "
            
            // Kopiere bis zu 16 Zeichen oder bis Zeilenende
            int i = 0;
            while (i < 16 && deveuiStart[i] != '\r' && deveuiStart[i] != '\n' && deveuiStart[i] != '\0') {
                buffer[i] = deveuiStart[i];
                i++;
            }
            buffer[i] = '\0';
            
            return true;
        }
    }
    
    return false;
}

bool LoRaWAN_WioE5::getFirmwareVersion(char* buffer) {
    if (!buffer) return false;
    
    if (sendATCommand("AT+VER\r\n", "+VER:", 3000)) {
        // Version extrahieren
        char* versionStart = strstr(_responseBuffer, "+VER:");
        if (versionStart != nullptr) {
            versionStart += 5; // Überspringe "+VER:"
            
            // Kopiere bis zu 31 Zeichen oder bis Zeilenende
            int i = 0;
            while (i < 31 && versionStart[i] != '\r' && versionStart[i] != '\n' && versionStart[i] != '\0') {
                buffer[i] = versionStart[i];
                i++;
            }
            buffer[i] = '\0';
            
            return true;
        }
    }
    
    return false;
}

bool LoRaWAN_WioE5::isModuleReady() {
    return _status.moduleReady;
}

uint16_t LoRaWAN_WioE5::getMessageCounter() {
    return _status.messageCounter;
}

bool LoRaWAN_WioE5::parseDownlinkMessage(const char* response) {
    if (!response) return false;
    
    // Suche nach Downlink-Pattern: "+CMSG: ACK Received" oder "+MSGHEX: RX" etc.
    // WioE5 sendet Downlinks im Format: "+CMSG: PORT: XX; RX: "HEXDATA""
    
    char* downlinkStart = nullptr;
    
    // Suche nach verschiedenen Downlink-Mustern
    if ((downlinkStart = strstr(response, "+CMSG: PORT:")) != nullptr ||
        (downlinkStart = strstr(response, "+MSGHEX: PORT:")) != nullptr) {
        
        // Port extrahieren
        char* portStart = strstr(downlinkStart, "PORT: ");
        if (portStart) {
            portStart += 6; // Skip "PORT: "
            _status.downlinkPort = (uint8_t)atoi(portStart);
        }
        
        // Downlink-Daten extrahieren
        char* rxStart = strstr(downlinkStart, "RX: \"");
        if (rxStart) {
            rxStart += 5; // Skip 'RX: "'
            
            // Hex-String bis zum schließenden Anführungszeichen lesen
            char* rxEnd = strchr(rxStart, '"');
            if (rxEnd) {
                size_t hexLen = rxEnd - rxStart;
                
                // Hex-String in Bytes konvertieren
                _status.downlinkSize = 0;
                for (size_t i = 0; i < hexLen && i < 510 && _status.downlinkSize < 255; i += 2) {
                    if (i + 1 < hexLen) {
                        char hexByte[3] = {rxStart[i], rxStart[i + 1], '\0'};
                        _status.downlinkData[_status.downlinkSize++] = (uint8_t)strtol(hexByte, nullptr, 16);
                    }
                }
                
                _status.hasDownlink = true;
                return true;
            }
        }
    }
    
    // Alternative Downlink-Formate prüfen
    if ((downlinkStart = strstr(response, "RX: \"")) != nullptr) {
        downlinkStart += 5; // Skip 'RX: "'
        
        char* rxEnd = strchr(downlinkStart, '"');
        if (rxEnd) {
            size_t hexLen = rxEnd - downlinkStart;
            
            _status.downlinkPort = 1; // Standard-Port wenn nicht spezifiziert
            _status.downlinkSize = 0;
            
            for (size_t i = 0; i < hexLen && i < 510 && _status.downlinkSize < 255; i += 2) {
                if (i + 1 < hexLen) {
                    char hexByte[3] = {downlinkStart[i], downlinkStart[i + 1], '\0'};
                    _status.downlinkData[_status.downlinkSize++] = (uint8_t)strtol(hexByte, nullptr, 16);
                }
            }
            
            _status.hasDownlink = true;
            return true;
        }
    }
    
    return false;
}

bool LoRaWAN_WioE5::hasDownlinkMessage() {
    return _status.hasDownlink;
}

size_t LoRaWAN_WioE5::getDownlinkSize() {
    return _status.downlinkSize;
}

const uint8_t* LoRaWAN_WioE5::getDownlinkData() {
    return _status.downlinkData;
}

uint8_t LoRaWAN_WioE5::getDownlinkPort() {
    return _status.downlinkPort;
}

void LoRaWAN_WioE5::clearDownlink() {
    _status.hasDownlink = false;
    _status.downlinkSize = 0;
    _status.downlinkPort = 0;
    memset(_status.downlinkData, 0, sizeof(_status.downlinkData));
}

// ================================================================
// ERWEITERTE FUNKTIONEN
// ================================================================

bool LoRaWAN_WioE5::resetModule() {
    debugPrintln("[INFO] Führe Modul-Reset durch...");
    return sendATCommand("AT+RESET\r\n", nullptr, 5000);
}

bool LoRaWAN_WioE5::enterDFUMode() {
    debugPrintln("=== Wio-E5 Firmware-Update Modus ===");
    debugPrintln("[INFO] Aktiviere DFU-Modus für Firmware-Update...");
    
    if (sendATCommand("AT+DFU=ON\r\n", "+DFU: ON", 5000)) {
        debugPrintln("[OK] DFU-Modus aktiviert");
        debugPrintln("[INFO] Wio-E5 ist jetzt im Bootloader-Modus");
        debugPrintln("[INFO] Bereit für Firmware-Update über UART");
        return true;
    } else {
        debugPrintln("[ERROR] Konnte DFU-Modus nicht aktivieren");
        return false;
    }
}

void LoRaWAN_WioE5::firmwareUpdateMode() {
    if (!_debugSerial) return;
    
    debugPrintln("\n=== Firmware-Update Passthrough-Modus ===");
    debugPrintln("[INFO] Starte UART-Passthrough für Firmware-Update");
    debugPrintln("[INFO] Alle Daten werden zwischen USB und Wio-E5 weitergeleitet");
    debugPrintln("[INFO] Sende 'EXIT' um Passthrough zu beenden\n");
    
    bool firmwareUpdateMode = true;
    String inputBuffer = "";
    
    while (firmwareUpdateMode) {
        // Von Debug-Serial zu Wio-E5
        if (_debugSerial->available()) {
            char c = _debugSerial->read();
            inputBuffer += c;
            
            // Prüfe auf EXIT-Kommando
            if (inputBuffer.indexOf("EXIT") != -1) {
                debugPrintln("\n[INFO] Beende Firmware-Update Modus");
                firmwareUpdateMode = false;
                break;
            }
            
            // Sende Zeichen an Wio-E5
            if (_serial) {
                _serial->write(c);
            }
        }
        
        // Von Wio-E5 zu Debug-Serial
        if (_serial && _serial->available()) {
            char c = _serial->read();
            _debugSerial->write(c);
        }
        
        // Buffer-Größe begrenzen
        if (inputBuffer.length() > 10) {
            inputBuffer = inputBuffer.substring(1);
        }
        
        delay(1);
    }
    
    debugPrintln("\n[OK] Firmware-Update Modus beendet");
}

bool LoRaWAN_WioE5::sendCustomATCommand(const char* command, const char* expectedResponse, 
                                       unsigned long timeout) {
    return sendATCommand(command, expectedResponse, timeout);
}

const char* LoRaWAN_WioE5::getLastResponse() {
    return _responseBuffer;
}

// ================================================================
// DEBUGGING
// ================================================================

void LoRaWAN_WioE5::setDebugEnabled(bool enabled) {
    // Debug-Ausgaben werden über den _debugSerial-Pointer gesteuert
    // Diese Funktion könnte erweitert werden für ein Debug-Flag
}

bool LoRaWAN_WioE5::testUARTConnection() {
    debugPrintln("=== UART-Verbindungstest ===");
    debugPrintln("[INFO] Teste Kommunikation...");
    
    // Puffer leeren
    clearInputBuffer();
    
    // Einfaches AT-Kommando senden
    _serial->print("AT\r\n");
    _serial->flush();
    delay(1000);
    
    // Antwort prüfen
    if (_serial->available() > 0) {
        debugPrint("[OK] Wio-E5 antwortet: ");
        while (_serial->available() > 0) {
            char ch = _serial->read();
            if (ch >= 32 && ch <= 126) {
                if (_debugSerial) _debugSerial->print(ch);
            } else {
                if (_debugSerial) {
                    _debugSerial->print("[");
                    _debugSerial->print((int)ch);
                    _debugSerial->print("]");
                }
            }
        }
        if (_debugSerial) _debugSerial->println();
        return true;
    } else {
        debugPrintln("[ERROR] Keine Antwort vom Wio-E5");
        return false;
    }
}

void LoRaWAN_WioE5::printModuleInfo() {
    if (!_debugSerial) return;
    
    _debugSerial->println("=== Wio-E5 Modul-Information ===");
    _debugSerial->print("Firmware-Version: ");
    _debugSerial->println(_status.firmwareVersion);
    _debugSerial->print("Device EUI: ");
    _debugSerial->println(_status.realDeviceEUI);
    _debugSerial->print("Modus: ");
    _debugSerial->println(_config.mode == LORAWAN_MODE_ABP ? "ABP" : "OTAA");
    _debugSerial->print("Klasse: ");
    switch (_config.deviceClass) {
        case LORAWAN_CLASS_A: _debugSerial->println("A"); break;
        case LORAWAN_CLASS_B: _debugSerial->println("B"); break;
        case LORAWAN_CLASS_C: _debugSerial->println("C"); break;
    }
    _debugSerial->print("Region: ");
    switch (_config.region) {
        case LORAWAN_REGION_EU868: _debugSerial->println("EU868"); break;
        case LORAWAN_REGION_US915: _debugSerial->println("US915"); break;
        case LORAWAN_REGION_AS923: _debugSerial->println("AS923"); break;
    }
    _debugSerial->print("Data Rate: DR");
    _debugSerial->println((int)_config.dataRate);
    _debugSerial->print("ADR: ");
    _debugSerial->println(_config.adaptiveDataRate ? "ON" : "OFF");
    _debugSerial->print("Modul bereit: ");
    _debugSerial->println(_status.moduleReady ? "JA" : "NEIN");
    _debugSerial->print("Netzwerk verbunden: ");
    _debugSerial->println(_status.networkJoined ? "JA" : "NEIN");
    _debugSerial->print("Nachrichten gesendet: ");
    _debugSerial->println(_status.messageCounter);
    _debugSerial->println("================================");
}

void LoRaWAN_WioE5::printChannelConfig() {
    if (!_debugSerial) return;
    
    _debugSerial->println("=== Kanal-Konfiguration ===");
    sendATCommand("AT+CH\r\n", nullptr, 3000);
    _debugSerial->println("===========================");
}

// ================================================================
// EINFACHE API - NUR ZWEI FUNKTIONEN FÜR MAIN!
// ================================================================

bool LoRaWAN_WioE5::initializeEverything() {
    // Initialisierung und Verbindung absichtlich ohne Logausgabe
    if (_serial) {
        _serial->pins(WIO_E5_TX_PIN, WIO_E5_RX_PIN);
    }

    if (!begin(WIO_E5_BAUDRATE)) {
        resetModule();
        delay(2000);
        if (!begin(WIO_E5_BAUDRATE)) {
            return false;
        }
    }

    if (!configureWithDefaults(LORAWAN_MODE_ABP)) {
        resetModule();
        delay(3000);
        if (!begin(WIO_E5_BAUDRATE)) {
            return false;
        }
        if (!configureWithDefaults(LORAWAN_MODE_ABP)) {
            return false;
        }
    }

    if (!joinNetwork()) {
        delay(5000);
        if (!joinNetwork()) {
            return false;
        }
    }

    if (_debugSerial) {
        _debugSerial->println("================================");
        _debugSerial->println("LoRaWAN Konfiguration");
        _debugSerial->println("================================");
        _debugSerial->println("Firmware-Version: " + String(_status.firmwareVersion));
        _debugSerial->println("Device EUI: " + String(_status.realDeviceEUI));
        _debugSerial->println("Modus: " + String(_config.mode == LORAWAN_MODE_ABP ? "ABP" : "OTAA"));
        _debugSerial->println("Klasse: " + String((_config.deviceClass == LORAWAN_CLASS_A) ? "A" : (_config.deviceClass == LORAWAN_CLASS_B) ? "B" : "C"));
        _debugSerial->println("Region: " + String((_config.region == LORAWAN_REGION_EU868) ? "EU868" : (_config.region == LORAWAN_REGION_US915) ? "US915" : "AS923"));
        _debugSerial->println("Data Rate: DR" + String((int)_config.dataRate));
        _debugSerial->println("ADR: " + String(_config.adaptiveDataRate ? "ON" : "OFF"));
        _debugSerial->println("Status: Bereit");
        _debugSerial->println("================================");
    }

    return true;
}

bool LoRaWAN_WioE5::sendJsonMessage(const char* jsonMessage) {
    if (!jsonMessage) {
        debugPrintln("[ERROR] Keine JSON-Nachricht angegeben!");
        return false;
    }
    
    if (!_status.moduleReady) {
        debugPrintln("[ERROR] Modul nicht bereit! Erst initializeEverything() aufrufen!");
        return false;
    }
    
    debugPrint("[INFO] Sende JSON #");
    if (_debugSerial) {
        _debugSerial->print(_status.messageCounter + 1);
        _debugSerial->print(": ");
        _debugSerial->println(jsonMessage);
    }
    
    // JSON-Nachricht senden
    bool success = sendMessage(jsonMessage);
    
    if (success) {
        debugPrintln("[SUCCESS] JSON-Nachricht erfolgreich gesendet!");
        if (_debugSerial) {
            _debugSerial->print("[INFO] Nächste Nachricht in mindestens ");
            _debugSerial->print(LORAWAN_SEND_INTERVAL_MS / 1000);
            _debugSerial->println(" Sekunden senden");
        }
    } else {
        debugPrintln("[ERROR] JSON-Nachricht konnte nicht gesendet werden!");
    }
    
    return success;
}

// ================================================================
// GLOBALE EINFACHE API-FUNKTIONEN
// ================================================================

// Globale Instanz für super einfache Verwendung
LoRaWAN_WioE5* g_loraWAN_instance = nullptr;

bool LoRaWAN_initEverything() {
    // Falls noch keine Instanz existiert, erstellen
    if (!g_loraWAN_instance) {
        // Globale Instanz mit Serial2 und SerialMon erstellen
        static LoRaWAN_WioE5 globalInstance(&Serial2, &Serial);
        g_loraWAN_instance = &globalInstance;
    }
    
    // Initialisierung durchführen
    return g_loraWAN_instance->initializeEverything();
}

bool LoRaWAN_sendJson(const char* jsonMessage) {
    if (!g_loraWAN_instance) {
        Serial.println("[ERROR] Erst LoRaWAN_initEverything() aufrufen!");
        return false;
    }
    
    return g_loraWAN_instance->sendJsonMessage(jsonMessage);
}
