/**
 * @file ChirpStackReceiver.h
 * @brief ChirpStack MQTT to UART Bridge Receiver Library
 * @author Smart Wire Industries
 * @version 1.0.0
 * @date 2025-01-23
 * 
 * @details
 * Library zum Empfangen und Dekodieren von LoRaWAN-Sensordaten von der ChirpStack MQTT-to-UART Bridge.
 * Unterstützt sowohl Text- als auch Binärformat-Nachrichten.
 */

#ifndef CHIRPSTACK_RECEIVER_H
#define CHIRPSTACK_RECEIVER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <UARTReceiver.h>
#include <vector>
#include "../../KitConfig.h"
#include "../../SerialMon.h"
#include "../Payload_Builder.h"

// ========================================
// Sensordaten-Strukturen
// ========================================

/**
 * @brief Struktur für einen einzelnen Sensorwert
 */
struct SensorValue {
    uint8_t tag;           // TLV Tag (0x01-0x04)
    uint8_t index;         // Index innerhalb des Typs (0-3)
    float value;           // Sensorwert
    unsigned long timestamp; // Zeitstempel des Empfangs
    
    SensorValue() : tag(0), index(0), value(0.0f), timestamp(0) {}
    SensorValue(uint8_t t, uint8_t i, float v) 
        : tag(t), index(i), value(v), timestamp(millis()) {}
};

/**
 * @brief Struktur für alle dekodierten Sensordaten
 */
struct SensorData {
    String deviceId;                    // Device ID (falls vorhanden)
    std::vector<SensorValue> values;    // Alle Sensorwerte
    unsigned long lastUpdate;           // Zeitpunkt der letzten Aktualisierung
    size_t rawPayloadSize;              // Größe der Rohdaten
    
    SensorData() : deviceId(""), lastUpdate(0), rawPayloadSize(0) {}
    
    // Hilfsfunktionen für einfachen Zugriff
    float getTemperature(uint8_t index = 0) const;
    float getDeflection(uint8_t index = 0) const;
    float getPressure(uint8_t index = 0) const;
    float getMisc(uint8_t index = 0) const;
    std::vector<float> getAllTemperatures() const;
    std::vector<float> getAllDeflections() const;
    std::vector<float> getAllPressures() const;
    std::vector<float> getAllMisc() const;
    bool hasData() const { return !values.empty(); }
    void clear();
};

// ========================================
// Konfigurationskonstanten
// ========================================

namespace ChirpStackConfig {
    constexpr uint8_t DEFAULT_LED_PIN = 13;
    constexpr uint32_t DEFAULT_STATS_INTERVAL_MS = 300000;  // 5 Minuten
    constexpr uint16_t PAYLOAD_BUFFER_SIZE = 256;
    constexpr uint8_t EXPECTED_PAYLOAD_SIZE = 28;
    constexpr uint16_t SERIAL_INIT_DELAY_MS = 100;
}

// ========================================
// Statistik-Klasse
// ========================================

class ChirpStackStatistics {
public:
    void recordReceived();
    void recordProcessed();
    void recordError();
    void begin(Stream* debug = nullptr);
    bool shouldDisplay();
    void display();
    
private:
    uint32_t messagesReceived = 0;
    uint32_t messagesProcessed = 0;
    uint32_t parseErrors = 0;
    unsigned long startTime = 0;
    unsigned long lastDisplayTime = 0;
    Stream* debugSerial = nullptr;
    
    void printUptime(unsigned long seconds);
};

// ========================================
// Nachrichtenverarbeitung
// ========================================

namespace ChirpStackMessageProcessor {
    
    /**
     * @brief Konvertiert Hex-String in Byte-Array
     */
    size_t hexToBytes(const String& hex, uint8_t* buffer, size_t maxSize);
    
    /**
     * @brief Dekodiert und zeigt Sensor-Daten
     */
    void decodeSensorData(const uint8_t* data, size_t size);
    
    /**
     * @brief Dekodiert Sensordaten und speichert sie in SensorData-Struktur
     */
    SensorData decodeSensorDataToStruct(const uint8_t* data, size_t size, const String& deviceId = "");
    
    /**
     * @brief Device-Information für empfangene Nachrichten
     */
    struct DeviceInfo {
        String deviceName;
        const uint8_t* payloadStart;
        size_t payloadSize;
        bool valid;
    };
    
    /**
     * @brief Extrahiert Device-Name aus Text+Binary Format
     */
    DeviceInfo extractDeviceInfo(const uint8_t* data, size_t size);
    
    /**
     * @brief Verarbeitet Text-Format Nachrichten
     */
    void processTextMessage(const String& message, ChirpStackStatistics& stats);
}

// ========================================
// ChirpStackReceiver Hauptklasse
// ========================================

class ChirpStackReceiver {
public:
    /**
     * @brief Konstruktor
     * @param serial Hardware-Serial für UART-Kommunikation
     * @param debugSerial Debug-Serial (z.B. USB Serial)
     * @param txPin TX-Pin für UART
     * @param rxPin RX-Pin für UART
     * @param baudrate Baudrate für UART
     * @param ledPin LED-Pin für Statusanzeige (optional)
     */
    ChirpStackReceiver(HardwareSerial* serial, Stream* debugSerial,
                       int txPin, int rxPin, uint32_t baudrate, 
                       int ledPin = ChirpStackConfig::DEFAULT_LED_PIN);
    
    /**
     * @brief Initialisiert den ChirpStack Receiver
     * @param debugMode Aktiviert Debug-Modus (direkte UART-Ausgabe)
     * @return true wenn erfolgreich
     */
    bool begin(bool debugMode = false);
    
    /**
     * @brief Hauptverarbeitungsschleife - muss in loop() aufgerufen werden
     */
    void process();
    
    /**
     * @brief Setzt das Statistik-Anzeigeintervall
     * @param intervalMs Intervall in Millisekunden
     */
    void setStatsInterval(uint32_t intervalMs);
    
    /**
     * @brief Aktiviert/Deaktiviert Debug-Modus
     * @param enabled true für Debug-Modus
     */
    void setDebugMode(bool enabled);
    
    /**
     * @brief Zeigt aktuelle Statistiken an
     */
    void displayStats();
    
    /**
     * @brief Callback für empfangene Binärdaten
     */
    void onBinaryData(const uint8_t* data, size_t size);
    
    /**
     * @brief Callback für empfangene JSON-Daten
     */
    void onJsonData(JsonObject data);
    
    /**
     * @brief Callback für Timeout
     */
    void onTimeout(unsigned long timeout);
    
    /**
     * @brief Callback für Status-Updates
     */
    void onStatus(uint32_t messages, uint32_t bytes, unsigned long uptime);
    
    /**
     * @brief Gibt die zuletzt empfangenen Sensordaten zurück
     * @return SensorData-Struktur mit allen Sensorwerten
     */
    SensorData getLastSensorData() const { return lastSensorData; }
    
    /**
     * @brief Prüft ob neue Daten seit dem letzten Abruf empfangen wurden
     * @param lastCheck Zeitstempel des letzten Abrufs
     * @return true wenn neue Daten vorhanden sind
     */
    bool hasNewData(unsigned long lastCheck) const {
        return lastSensorData.lastUpdate > lastCheck;
    }
    
    /**
     * @brief Gibt alle Sensordaten als JSON-String zurück
     * @return JSON-formatierter String mit allen Sensordaten
     */
    String getSensorDataAsJson() const;
    
private:
    UARTReceiver uartReceiver;
    ChirpStackStatistics stats;
    Stream* debugSerial;
    bool debugMode;
    uint32_t statsIntervalMs;
    SensorData lastSensorData;  // Speichert die zuletzt empfangenen Sensordaten
    
    void initializeSerial();
    void initializeUART();
    void displayWelcome();
    void processDebugMode();
    
    // Statische Callback-Wrapper
    static ChirpStackReceiver* instance;
    static void staticOnBinaryData(const uint8_t* data, size_t size);
    static void staticOnJsonData(JsonObject data);
    static void staticOnTimeout(unsigned long timeout);
    static void staticOnStatus(uint32_t messages, uint32_t bytes, unsigned long uptime);
};

#endif // CHIRPSTACK_RECEIVER_H
