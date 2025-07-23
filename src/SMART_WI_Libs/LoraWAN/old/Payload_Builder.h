/**
 * @file Payload_Builder.h
 * @brief LoRaWAN Payload Builder Library for Smart Wire Industries
 * @author Smart Wire Industries
 * @version 1.0.1
 * @date 2025-01-17
 * @license MIT
 * 
 * Universal payload builder library for LoRaWAN nodes.
 * Supports modular sensor types with automatic payload size optimization.
 * Compatible with WioE5 and SX1262 modules on AVR128DB48 microcontrollers.
 */

#ifndef PAYLOAD_BUILDER_H
#define PAYLOAD_BUILDER_H

#include "Arduino.h"

// ================================================================
// KONSTANTEN UND KONFIGURATION
// ================================================================

// Payload-Limits für EU868 (abhängig von Data Rate)
#define PAYLOAD_MAX_SIZE_DR0 51        // SF12: Max. 51 bytes
#define PAYLOAD_MAX_SIZE_DR1 51        // SF11: Max. 51 bytes  
#define PAYLOAD_MAX_SIZE_DR2 51        // SF10: Max. 51 bytes
#define PAYLOAD_MAX_SIZE_DR3 115       // SF9:  Max. 115 bytes
#define PAYLOAD_MAX_SIZE_DR4 242       // SF8:  Max. 242 bytes
#define PAYLOAD_MAX_SIZE_DR5 242       // SF7:  Max. 242 bytes (EMPFOHLEN)

// Standard-Payload-Limit
#define PAYLOAD_DEFAULT_MAX_SIZE PAYLOAD_MAX_SIZE_DR5

// Sensor-Typ-Kennungen
#define SENSOR_TYPE_TEMPERATURE 'T'    // 0x54
#define SENSOR_TYPE_DEFLECTION  'D'    // 0x44
#define SENSOR_TYPE_PRESSURE    'P'    // 0x50
#define SENSOR_TYPE_MISC        'S'    // 0x53 (Sonstiges)

// Maximale Anzahl von Sensoren pro Typ
#define MAX_TEMPERATURE_SENSORS 8
#define MAX_DEFLECTION_SENSORS  4
#define MAX_PRESSURE_SENSORS    4
#define MAX_MISC_SENSORS        4

// Device-ID Limits
#define MAX_DEVICE_ID_LENGTH 32

// ================================================================
// STRUKTUREN
// ================================================================

struct PayloadConfig {
    bool temperatureSensorsEnabled;
    bool deflectionSensorsEnabled;
    bool pressureSensorsEnabled;
    bool miscSensorsEnabled;
    
    uint8_t temperatureSensorCount;
    uint8_t deflectionSensorCount;
    uint8_t pressureSensorCount;
    uint8_t miscSensorCount;
    
    bool ultraCompactMode;
    bool debugEnabled;
    
    char deviceId[MAX_DEVICE_ID_LENGTH];
    size_t maxPayloadSize;
};

struct SensorData {
    float temperatureValues[MAX_TEMPERATURE_SENSORS];
    float deflectionValues[MAX_DEFLECTION_SENSORS];
    float pressureValues[MAX_PRESSURE_SENSORS];
    float miscValues[MAX_MISC_SENSORS];
    
    uint8_t temperatureCount;
    uint8_t deflectionCount;
    uint8_t pressureCount;
    uint8_t miscCount;
    
    uint32_t timestamp;
};

// ================================================================
// HAUPTKLASSE
// ================================================================

class Payload_Builder {
private:
    PayloadConfig _config;
    SensorData _data;
    Stream* _debugSerial;
    
    // Private Hilfsmethoden
    void debugPrint(const char* message);
    void debugPrintln(const char* message);
    void debugPrintFloat(float value, int decimals = 2);
    void debugPrintHex(uint8_t value);
    
    // Payload-Builder-Methoden
    size_t addTemperatureData(uint8_t* buffer, size_t bufferSize, size_t offset);
    size_t addDeflectionData(uint8_t* buffer, size_t bufferSize, size_t offset);
    size_t addPressureData(uint8_t* buffer, size_t bufferSize, size_t offset);
    size_t addMiscData(uint8_t* buffer, size_t bufferSize, size_t offset);
    
    // Hilfsmethoden für Float-Serialisierung
    void floatToBytes(float value, uint8_t* bytes);
    float bytesToFloat(const uint8_t* bytes);

public:
    // ================================================================
    // KONSTRUKTOR UND DESTRUKTOR
    // ================================================================
    
    /**
     * Konstruktor für Payload Builder
     * @param debugSerial Pointer zur Debug-Serial-Schnittstelle (optional)
     */
    Payload_Builder(Stream* debugSerial = nullptr);
    
    /**
     * Destruktor
     */
    ~Payload_Builder();
    
    // ================================================================
    // KONFIGURATION
    // ================================================================
    
    /**
     * Aktiviert Temperatursensoren
     * @param count Anzahl der Temperatursensoren (1-8)
     * @return true wenn erfolgreich
     */
    bool enableTemperatureSensors(uint8_t count);
    
    /**
     * Aktiviert Deflection-Sensoren
     * @param count Anzahl der Deflection-Sensoren (1-4)
     * @return true wenn erfolgreich
     */
    bool enableDeflectionSensors(uint8_t count);
    
    /**
     * Aktiviert Drucksensoren
     * @param count Anzahl der Drucksensoren (1-4)
     * @return true wenn erfolgreich
     */
    bool enablePressureSensors(uint8_t count);
    
    /**
     * Aktiviert Sonstige Sensoren
     * @param count Anzahl der sonstigen Sensoren (1-4)
     * @return true wenn erfolgreich
     */
    bool enableMiscSensors(uint8_t count);
    
    /**
     * Setzt die Device-ID
     * @param deviceId Device-ID String
     * @return true wenn erfolgreich
     */
    bool setDeviceId(const char* deviceId);
    
    /**
     * Aktiviert/Deaktiviert Ultra-Compact-Modus
     * @param enabled Ultra-Compact-Modus aktivieren (keine Typ-Kennungen)
     */
    void setUltraCompactMode(bool enabled);
    
    /**
     * Setzt die maximale Payload-Größe
     * @param maxSize Maximale Payload-Größe in Bytes
     */
    void setMaxPayloadSize(size_t maxSize);
    
    /**
     * Aktiviert/Deaktiviert Debug-Ausgaben
     * @param enabled Debug-Ausgaben aktivieren
     */
    void setDebugEnabled(bool enabled);
    
    // ================================================================
    // SENSORDATEN HINZUFÜGEN
    // ================================================================
    
    /**
     * Fügt einen Temperaturwert hinzu
     * @param temperature Temperatur in °C
     * @return true wenn erfolgreich hinzugefügt
     */
    bool addTemperature(float temperature);
    
    /**
     * Fügt einen Deflection-Wert hinzu
     * @param deflection Deflection in mm
     * @return true wenn erfolgreich hinzugefügt
     */
    bool addDeflection(float deflection);
    
    /**
     * Fügt einen Druckwert hinzu
     * @param pressure Druck in hPa
     * @return true wenn erfolgreich hinzugefügt
     */
    bool addPressure(float pressure);
    
    /**
     * Fügt einen sonstigen Wert hinzu
     * @param value Beliebiger Wert
     * @return true wenn erfolgreich hinzugefügt
     */
    bool addMiscData(float value);
    
    /**
     * Setzt den Zeitstempel
     * @param timestamp Zeitstempel in Sekunden
     */
    void setTimestamp(uint32_t timestamp);
    
    // ================================================================
    // PAYLOAD ERSTELLEN
    // ================================================================
    
    /**
     * Erstellt das Payload
     * @param buffer Buffer für das Payload
     * @param bufferSize Größe des Buffers
     * @return Größe des erstellten Payloads (0 bei Fehler)
     */
    size_t build(uint8_t* buffer, size_t bufferSize);
    
    /**
     * Gibt die geschätzte Payload-Größe zurück
     * @return Geschätzte Payload-Größe in Bytes
     */
    size_t getEstimatedSize();
    
    // ================================================================
    // HILFSMETHODEN
    // ================================================================
    
    /**
     * Setzt alle Sensordaten zurück
     */
    void reset();
    
    /**
     * Gibt die aktuelle Konfiguration zurück
     * @return Konfigurations-Struktur
     */
    PayloadConfig getConfig();
    
    /**
     * Gibt die aktuellen Sensordaten zurück
     * @return Sensordaten-Struktur
     */
    SensorData getSensorData();
    
    // ================================================================
    // DEBUG-FUNKTIONEN
    // ================================================================
    
    /**
     * Gibt das Payload in lesbarer Form aus
     * @param buffer Payload-Buffer
     * @param size Größe des Payloads
     */
    void debugPrintPayload(const uint8_t* buffer, size_t size);
    
    /**
     * Gibt die Payload-Konfiguration aus
     */
    void debugPrintConfig();
    
    /**
     * Gibt die aktuellen Sensordaten aus
     */
    void debugPrintSensorData();
    
    /**
     * Dekodiert ein empfangenes Payload für Debug-Zwecke
     * @param buffer Empfangenes Payload
     * @param size Größe des Payloads
     */
    void debugDecodePayload(const uint8_t* buffer, size_t size);
};

// ================================================================
// GLOBALE HILFSFUNKTIONEN
// ================================================================

/**
 * Erstellt ein JSON-String aus Payload-Daten (für Legacy-Unterstützung)
 * @param builder Payload Builder Instanz
 * @param jsonBuffer Buffer für JSON-String
 * @param bufferSize Größe des JSON-Buffers
 * @return Länge des JSON-Strings
 */
size_t payloadToJson(Payload_Builder& builder, char* jsonBuffer, size_t bufferSize);

/**
 * Berechnet die CRC16-Prüfsumme für ein Payload
 * @param buffer Payload-Buffer
 * @param size Größe des Payloads
 * @return CRC16-Prüfsumme
 */
uint16_t calculatePayloadCRC(const uint8_t* buffer, size_t size);

#endif // PAYLOAD_BUILDER_H
