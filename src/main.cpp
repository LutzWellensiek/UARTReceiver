/**
 * @file main.cpp
 * @brief ChirpStack MQTT to UART Bridge Receiver für AVR-IoT Cellular Mini Board
 * @author Smart Wire Industries
 * @version 2.0.0
 * @date 2025-01-22
 * 
 * @details
 * Empfängt und dekodiert LoRaWAN-Sensordaten von der ChirpStack MQTT-to-UART Bridge.
 * Unterstützt sowohl Text- als auch Binärformat-Nachrichten.
 * 
 * Hardware-Konfiguration:
 * - Board: AVR128DB48 (AVR-IoT Cellular Mini)
 * - UART2: Bridge-Kommunikation (115200 baud)
 * - USB Serial: Debug-Monitor (115200 baud)
 */

#include <Arduino.h>
#include <ArduinoJson.h>
#include "UARTReceiver.h"
#include "SMART_WI_Libs/KitConfig.h"
#include "SMART_WI_Libs/SerialMon.h"
#include "SMART_WI_Libs/LoraWAN/Payload_Builder.h"

// ========================================
// Konstanten und Konfiguration
// ========================================

namespace Config {
    constexpr uint8_t LED_PIN = 13;
    constexpr uint32_t STATS_INTERVAL_MS = 300000;  // 5 Minuten
    constexpr uint16_t PAYLOAD_BUFFER_SIZE = 256;  // Umbenannt um Konflikt zu vermeiden
    constexpr uint8_t EXPECTED_PAYLOAD_SIZE = 28;
    constexpr uint16_t SERIAL_INIT_DELAY_MS = 100;
    constexpr bool DEBUG_RAW_UART = false;  // Debug-Modus ausgeschaltet
}

// ========================================
// Globale Objekte
// ========================================

UARTReceiver uartReceiver(&Serial2, &SerialMon, SerialPIN_TX, SerialPIN_RX, 
                          UART2_BAUDRATE, Config::LED_PIN);

// ========================================
// Statistik-Verwaltung
// ========================================

class Statistics {
public:
    void recordReceived() { messagesReceived++; }
    void recordProcessed() { messagesProcessed++; }
    void recordError() { parseErrors++; }
    
    void begin() {
        startTime = millis();
        lastDisplayTime = startTime;
    }
    
    bool shouldDisplay() {
        return (millis() - lastDisplayTime) > Config::STATS_INTERVAL_MS;
    }
    
    void display() {
        const auto uptime = millis() - startTime;
        const auto uptimeSeconds = uptime / 1000;
        
        SerialMon.println(F("\n=== STATISTIKEN ==="));
        SerialMon.print(F("Uptime: "));
        printUptime(uptimeSeconds);
        
        SerialMon.print(F("Empfangen: "));
        SerialMon.println(messagesReceived);
        
        SerialMon.print(F("Verarbeitet: "));
        SerialMon.println(messagesProcessed);
        
        SerialMon.print(F("Fehler: "));
        SerialMon.println(parseErrors);
        
        if (uptimeSeconds > 0) {
            const float rate = static_cast<float>(messagesReceived) / uptimeSeconds * 60;
            SerialMon.print(F("Rate: "));
            SerialMon.print(rate, 1);
            SerialMon.println(F(" msg/min"));
        }
        
        SerialMon.println(F("==================\n"));
        lastDisplayTime = millis();
    }
    
private:
    uint32_t messagesReceived = 0;
    uint32_t messagesProcessed = 0;
    uint32_t parseErrors = 0;
    unsigned long startTime = 0;
    unsigned long lastDisplayTime = 0;
    
    void printUptime(unsigned long seconds) {
        const auto hours = seconds / 3600;
        const auto minutes = (seconds % 3600) / 60;
        const auto secs = seconds % 60;
        
        SerialMon.print(hours);
        SerialMon.print(F("h "));
        SerialMon.print(minutes);
        SerialMon.print(F("m "));
        SerialMon.print(secs);
        SerialMon.println(F("s"));
    }
};

Statistics stats;

// ========================================
// Nachrichtenverarbeitung
// ========================================

namespace MessageProcessor {
    
    /**
     * @brief Konvertiert Hex-String in Byte-Array
     */
    size_t hexToBytes(const String& hex, uint8_t* buffer, size_t maxSize) {
        const size_t hexLen = hex.length();
        
        if (hexLen % 2 != 0) {
            SerialMon.println(F("Fehler: Hex-String hat ungerade Länge"));
            return 0;
        }
        
        const size_t byteCount = hexLen / 2;
        if (byteCount > maxSize) {
            SerialMon.println(F("Fehler: Hex-String zu lang"));
            return 0;
        }
        
        for (size_t i = 0; i < byteCount; i++) {
            const String byteStr = hex.substring(i * 2, i * 2 + 2);
            buffer[i] = static_cast<uint8_t>(strtol(byteStr.c_str(), nullptr, 16));
        }
        
        return byteCount;
    }
    
    /**
     * @brief Dekodiert und zeigt Sensor-Daten
     */
    void decodeSensorData(const uint8_t* data, size_t size) {
        SerialMon.println(F("\n=== SENSOR-DEKODIERUNG ==="));
        
        // Dekodierung mit Payload_Builder
        decodePayload(data, size);
        
        // Hex-Ausgabe für Debugging
        printPayloadHex(data, size);
        
        SerialMon.println(F("========================\n"));
    }
    
    /**
     * @brief Device-Information für empfangene Nachrichten
     */
    struct DeviceInfo {
        String deviceName;       // Text-basierter Device-Name
        const uint8_t* payloadStart;
        size_t payloadSize;
        bool valid;
    };
    
    /**
     * @brief Extrahiert Device-Name aus Text+Binary Format
     * Format: "DEVICE_NAME: " gefolgt von binären Daten
     * HINWEIS: Die UARTReceiver Library extrahiert bereits den Device-Namen,
     * sodass wir hier nur die reine Payload erhalten.
     */
    DeviceInfo extractDeviceInfo(const uint8_t* data, size_t size) {
        DeviceInfo info = {"", nullptr, 0, false};
        
        // Da die UARTReceiver Library bereits den Device-Namen extrahiert hat,
        // erhalten wir hier nur die reine Payload
        // Die gesamte empfangene Nachricht ist die Sensor-Payload
        info.deviceName = "Bridge-Device"; // Placeholder, da Name bereits extrahiert wurde
        info.payloadStart = data;
        info.payloadSize = size;
        info.valid = true;
        
        // Debug-Ausgabe
        SerialMon.println(F("[DEBUG] Verarbeite reine Sensor-Payload"));
        SerialMon.print(F("[DEBUG] Payload-Größe: "));
        SerialMon.print(info.payloadSize);
        SerialMon.println(F(" Bytes"));
        
        // Zeige erste Bytes der Payload
        SerialMon.print(F("[DEBUG] Erste Payload-Bytes: "));
        for (size_t i = 0; i < min(info.payloadSize, (size_t)10); i++) {
            if (info.payloadStart[i] < 16) SerialMon.print(F("0"));
            SerialMon.print(info.payloadStart[i], HEX);
            SerialMon.print(F(" "));
        }
        SerialMon.println();
        
        return info;
    }
    
    
    /**
     * @brief Verarbeitet Text-Format Nachrichten
     */
    void processTextMessage(const String& message) {
        stats.recordReceived();
        
        const int colonIndex = message.indexOf(':');
        if (colonIndex <= 0) {
            SerialMon.println(F("Fehler: Ungültiges Format"));
            stats.recordError();
            return;
        }
        
        String deviceName = message.substring(0, colonIndex);
        String payloadHex = message.substring(colonIndex + 1);
        
        deviceName.trim();
        payloadHex.trim();
        
        SerialMon.println(F("\n=== CHIRPSTACK TEXT MESSAGE ==="));
        SerialMon.print(F("Device: "));
        SerialMon.println(deviceName);
        SerialMon.print(F("Payload (Hex): "));
        SerialMon.println(payloadHex);
        
        uint8_t buffer[Config::PAYLOAD_BUFFER_SIZE];
        const size_t payloadSize = hexToBytes(payloadHex, buffer, sizeof(buffer));
        
        if (payloadSize == 0) {
            SerialMon.println(F("Fehler: Hex-Konvertierung fehlgeschlagen"));
            stats.recordError();
            return;
        }
        
        SerialMon.print(F("Payload Size: "));
        SerialMon.print(payloadSize);
        SerialMon.println(F(" bytes"));
        
        decodeSensorData(buffer, payloadSize);
        stats.recordProcessed();
    }
}
// ========================================
// Callbacks
// ========================================

namespace Callbacks {
    
    /**
     * @brief Verarbeitet binäre Nachrichten von ChirpStack Bridge
     * HINWEIS: Die UARTReceiver Library hat bereits den Device-Namen extrahiert,
     * sodass 'data' hier nur die reine Payload enthält (ohne Device-Name und ': ')
     */
    void onBinaryData(const uint8_t* data, size_t size) {
        using namespace MessageProcessor;
        
        stats.recordReceived();
        
        SerialMon.println(F("\n=== CHIRPSTACK BRIDGE PAYLOAD ==="));
        SerialMon.print(F("Payload-Größe: "));
        SerialMon.print(size);
        SerialMon.println(F(" Bytes (ohne Device-Name)"));
        
        // Debug: Zeige die Payload-Daten
        SerialMon.println(F("\n[DEBUG] Payload-Inhalt:"));
        SerialMon.print(F("HEX: "));
        for (size_t i = 0; i < size; i++) {
            if (data[i] < 16) SerialMon.print(F("0"));
            SerialMon.print(data[i], HEX);
            SerialMon.print(F(" "));
            if ((i + 1) % 16 == 0) SerialMon.println();
        }
        SerialMon.println();
        
        // Prüfe erstes Byte auf Sensortyp
        if (size > 0) {
            uint8_t firstByte = data[0];
            SerialMon.print(F("\n[INFO] Erstes Payload-Byte: 0x"));
            if (firstByte < 16) SerialMon.print(F("0"));
            SerialMon.print(firstByte, HEX);
            
            if (firstByte == 'T') {
                SerialMon.println(F(" = 'T' (Temperatur) ✓"));
            } else if (firstByte == 'D') {
                SerialMon.println(F(" = 'D' (Deflection) ✓"));
            } else if (firstByte == 'P') {
                SerialMon.println(F(" = 'P' (Pressure) ✓"));
            } else if (firstByte == 'S') {
                SerialMon.println(F(" = 'S' (Sonstiges) ✓"));
            } else {
                SerialMon.print(F(" = '"));
                if (firstByte >= 32 && firstByte <= 126) {
                    SerialMon.print((char)firstByte);
                } else {
                    SerialMon.print("?");
                }
                SerialMon.println(F("' [WARNUNG: Kein gültiger Sensortyp!]"));
            }
        }
        
        // Die UARTReceiver Library hat bereits den Device-Namen extrahiert
        // Wir verarbeiten hier direkt die Sensor-Payload
        SerialMon.println(F("\n[INFO] Verarbeite Sensor-Payload direkt..."));
        
        // Dekodiere die Sensor-Daten direkt
        decodeSensorData(data, size);
        stats.recordProcessed();
        
        SerialMon.println(F("==============================\n"));
    }
    
    /**
     * @brief Verarbeitet JSON-Nachrichten
     */
    void onJsonData(JsonObject data) {
        SerialMon.println(F("\n=== JSON MESSAGE ==="));
        String jsonString;
        serializeJson(data, jsonString);
        SerialMon.println(jsonString);
        SerialMon.println(F("==================\n"));
    }
    
    /**
     * @brief Timeout-Handler
     */
    void onTimeout(unsigned long timeout) {
        SerialMon.print(F("Timeout nach "));
        SerialMon.print(timeout / 1000);
        SerialMon.println(F(" Sekunden"));
    }
    
    /**
     * @brief Status-Update Handler
     */
    void onStatus(uint32_t messages, uint32_t bytes, unsigned long uptime) {
        SerialMon.print(F("[Status] Nachrichten: "));
        SerialMon.print(messages);
        SerialMon.print(F(", Bytes: "));
        SerialMon.print(bytes);
        SerialMon.print(F(", Uptime: "));
        SerialMon.print(uptime / 1000);
        SerialMon.println(F("s"));
    }
}

// ========================================
// Initialisierung
// ========================================

namespace System {
    
    /**
     * @brief Initialisiert die serielle Kommunikation
     */
    void initializeSerial() {
        SerialMon.begin(SERIAL_MON_BAUDRATE);
        while (!SerialMon) {
            delay(10);
        }
        SerialMon.println(F("\n=== System Start ==="));
        SerialMon.print(F("Version: "));
        SerialMon.println(F("2.0.0"));
        delay(Config::SERIAL_INIT_DELAY_MS);
    }
    
    /**
     * @brief Initialisiert den UART Receiver
     */
    void initializeUART() {
        SerialMon.println(F("\nInitialisiere UART..."));
        
        if (Config::DEBUG_RAW_UART) {
            // Im Debug-Modus: Direkte Serial2 Initialisierung
            SerialMon.println(F("[DEBUG] Initialisiere Serial2 direkt (ohne UARTReceiver)"));
            
            // Pin-Konfiguration explizit setzen
            pinMode(SerialPIN_TX, OUTPUT);
            pinMode(SerialPIN_RX, INPUT_PULLUP);
            
            // WICHTIG: Pin-Zuordnung für Serial2
            Serial2.pins(SerialPIN_TX, SerialPIN_RX);
            
            Serial2.begin(UART2_BAUDRATE);
            SerialMon.println(F("[DEBUG] Serial2 initialisiert mit 115200 Baud"));
            SerialMon.print(F("[DEBUG] Serial2 Status: "));
            SerialMon.println(Serial2 ? "OK" : "FEHLER");
            
            // Warte kurz und teste Empfang
            delay(100);
            SerialMon.print(F("[DEBUG] Serial2.available() nach Init: "));
            SerialMon.println(Serial2.available());
            
            return;
        }
        
        // Normaler Modus: UARTReceiver verwenden
        if (!uartReceiver.begin()) {
            SerialMon.println(F("FEHLER: UART-Initialisierung fehlgeschlagen!"));
            while (true) {
                delay(1000);
            }
        }
        
        uartReceiver.displaySystemInfo();
        
        // Konfiguration
        SerialMon.println(F("\nKonfiguriere UART-Receiver..."));
        uartReceiver.setBinaryMode(true);
        // Keine feste Payload-Größe setzen - variable Längen erlauben
        // uartReceiver.setExpectedPayloadSize(Config::EXPECTED_PAYLOAD_SIZE);
        
        // Callbacks registrieren
        uartReceiver.setBinaryCallback(Callbacks::onBinaryData);
        uartReceiver.setJSONCallback(Callbacks::onJsonData);
        uartReceiver.setTimeoutCallback(Callbacks::onTimeout);
        uartReceiver.setStatusCallback(Callbacks::onStatus);
        
        SerialMon.println(F("UART-Receiver bereit"));
    }
    
    /**
     * @brief Zeigt Willkommensnachricht
     */
    void displayWelcome() {
        SerialMon.println(F("\n========================================"));
        SerialMon.println(F("ChirpStack MQTT-to-UART Bridge Receiver"));
        SerialMon.println(F("========================================"));
        SerialMon.println(F("Format: [4-Byte Device-ID] + [Payload mit Sensor-Typen]"));
        
        if (Config::DEBUG_RAW_UART) {
            SerialMon.println(F("\n[DEBUG MODUS AKTIV]"));
            SerialMon.println(F("- Zeige Rohdaten von Serial2"));
            SerialMon.println(F("- Ümgehe UARTReceiver Klasse"));
            SerialMon.print(F("- Serial2 TX Pin: "));
            SerialMon.println(SerialPIN_TX);
            SerialMon.print(F("- Serial2 RX Pin: "));
            SerialMon.println(SerialPIN_RX);
            SerialMon.print(F("- Baudrate: "));
            SerialMon.println(UART2_BAUDRATE);
        }
        
        SerialMon.println(F("\nWarte auf Nachrichten...\n"));
    }
}

// ========================================
// Arduino Hauptfunktionen
// ========================================

void setup() {
    // Statistiken initialisieren
    stats.begin();
    
    // System initialisieren
    System::initializeSerial();
    System::initializeUART();
    System::displayWelcome();
}

void loop() {
    
    // Debug-Modus: Direkte Serial2 Ausgabe
    if (Config::DEBUG_RAW_UART) {
        if (Serial2.available() > 0) {
            SerialMon.println(F("\n[RAW UART DATA]"));
            SerialMon.print(F("Bytes verfügbar: "));
            SerialMon.println(Serial2.available());
            
            int count = 0;
            while (Serial2.available() > 0 && count < 100) {
                uint8_t b = Serial2.read();
                
                // Hex-Ausgabe
                if (b < 16) SerialMon.print(F("0"));
                SerialMon.print(b, HEX);
                SerialMon.print(F(" "));
                
                // ASCII-Ausgabe nach 16 Bytes
                if ((count + 1) % 16 == 0) {
                    SerialMon.print(F("  |"));
                    for (int i = count - 15; i <= count; i++) {
                        // Gehe 16 Bytes zurück und drucke ASCII
                        SerialMon.print(F(" "));
                    }
                    SerialMon.println();
                }
                
                count++;
            }
            
            if (count % 16 != 0) {
                SerialMon.println();
            }
            
            SerialMon.print(F("Total gelesen: "));
            SerialMon.print(count);
            SerialMon.println(F(" Bytes"));
            SerialMon.println(F("[END RAW DATA]\n"));
        }
    } else {
        // Normale UART-Nachrichten verarbeiten
        uartReceiver.process();
    }
    
    // Statistiken periodisch anzeigen
    if (stats.shouldDisplay()) {
        stats.display();
    }
    
    // Minimale Pause für Stabilität
    delay(1);
}
