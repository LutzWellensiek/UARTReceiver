/**
 * @file Payload_Builder_Cayenne.h
 * @brief CayenneLPP-based LoRaWAN payload builder for UARTReceiver
 * @author Smart Wire Industries
 * @version 2.0.0
 * @date 2025-01-28
 * 
 * Diese Version nutzt CayenneLPP für standardisierte Payloads,
 * die automatisch von vielen LoRaWAN-Plattformen dekodiert werden können.
 */

#ifndef PAYLOAD_BUILDER_CAYENNE_H
#define PAYLOAD_BUILDER_CAYENNE_H

#include "Arduino.h"
#include <CayenneLPP.h>

// Special value to indicate "not used"
#define NO_VALUE NAN

// Standard Cayenne buffer size
#define CAYENNE_BUFFER_SIZE 200

// Include Payload_Builder.h for PayloadResult struct
#include "Payload_Builder.h"

// Forward declaration
struct SensorData;

/**
 * @class Payload_Builder_Cayenne
 * @brief Erstellt LoRaWAN-Payloads mit dem CayenneLPP-Format
 * 
 * Diese Klasse bietet eine einfache Schnittstelle zur Erstellung von
 * CayenneLPP-Payloads für verschiedene Sensortypen.
 */
class Payload_Builder_Cayenne {
private:
    CayenneLPP* lpp;
    uint8_t nextChannel;  // Automatische Kanalverwaltung
    
public:
    /**
     * Konstruktor
     * @param bufferSize Größe des internen Buffers (Standard: 200 Bytes)
     */
    Payload_Builder_Cayenne(uint16_t bufferSize = CAYENNE_BUFFER_SIZE);
    
    /**
     * Destruktor
     */
    ~Payload_Builder_Cayenne();
    
    /**
     * Setzt das Payload zurück und bereitet es für neue Daten vor
     */
    void reset();
    
    /**
     * Fügt Temperaturwerte hinzu (automatische Kanalzuweisung)
     * @param temperatures Array mit Temperaturwerten (°C)
     * @param count Anzahl der Temperaturwerte
     * @return true wenn erfolgreich hinzugefügt
     */
    bool addTemperatures(float* temperatures, uint8_t count);
    
    /**
     * Fügt einzelnen Temperaturwert hinzu
     * @param temperature Temperatur in °C
     * @param channel Optionaler Kanal (0 = automatisch)
     * @return true wenn erfolgreich hinzugefügt
     */
    bool addTemperature(float temperature, uint8_t channel = 0);
    
    /**
     * Fügt Druckwerte hinzu (automatische Kanalzuweisung)
     * @param pressures Array mit Druckwerten (hPa)
     * @param count Anzahl der Druckwerte
     * @return true wenn erfolgreich hinzugefügt
     */
    bool addPressures(float* pressures, uint8_t count);
    
    /**
     * Fügt einzelnen Druckwert hinzu
     * @param pressure Druck in hPa
     * @param channel Optionaler Kanal (0 = automatisch)
     * @return true wenn erfolgreich hinzugefügt
     */
    bool addPressure(float pressure, uint8_t channel = 0);
    
    /**
     * Fügt Analogwerte hinzu (für Deflection, Sensoren etc.)
     * @param values Array mit Analogwerten
     * @param count Anzahl der Werte
     * @return true wenn erfolgreich hinzugefügt
     */
    bool addAnalogValues(float* values, uint8_t count);
    
    /**
     * Fügt einzelnen Analogwert hinzu
     * @param value Analogwert
     * @param channel Optionaler Kanal (0 = automatisch)
     * @return true wenn erfolgreich hinzugefügt
     */
    bool addAnalogValue(float value, uint8_t channel = 0);
    
    /**
     * Fügt Feuchtigkeitswert hinzu
     * @param humidity Relative Feuchtigkeit in %
     * @param channel Optionaler Kanal (0 = automatisch)
     * @return true wenn erfolgreich hinzugefügt
     */
    bool addHumidity(float humidity, uint8_t channel = 0);
    
    /**
     * Fügt Digitalwert hinzu
     * @param value Digitalwert (0 oder 1)
     * @param channel Optionaler Kanal (0 = automatisch)
     * @return true wenn erfolgreich hinzugefügt
     */
    bool addDigitalInput(uint8_t value, uint8_t channel = 0);
    
    /**
     * Fügt GPS-Position hinzu
     * @param latitude Breitengrad
     * @param longitude Längengrad
     * @param altitude Höhe in Metern
     * @param channel Optionaler Kanal (0 = automatisch)
     * @return true wenn erfolgreich hinzugefügt
     */
    bool addGPS(float latitude, float longitude, float altitude, uint8_t channel = 0);
    
    /**
     * Fügt Beschleunigungswerte hinzu
     * @param x X-Achse in g
     * @param y Y-Achse in g
     * @param z Z-Achse in g
     * @param channel Optionaler Kanal (0 = automatisch)
     * @return true wenn erfolgreich hinzugefügt
     */
    bool addAccelerometer(float x, float y, float z, uint8_t channel = 0);
    
    /**
     * Fügt Helligkeitswert hinzu
     * @param lux Helligkeit in Lux
     * @param channel Optionaler Kanal (0 = automatisch)
     * @return true wenn erfolgreich hinzugefügt
     */
    bool addLuminosity(uint16_t lux, uint8_t channel = 0);
    
    /**
     * Erstellt Payload aus allen konfigurierten Sensordaten
     * Kompatibel mit dem alten Interface
     * @param buffer Output buffer für Payload
     * @param bufferSize Größe des Output buffers
     * @param temp1-temp4 Temperaturwerte (NO_VALUE zum Überspringen)
     * @param defl1-defl3 Deflection/Analogwerte (NO_VALUE zum Überspringen)
     * @param press1-press2 Druckwerte (NO_VALUE zum Überspringen)
     * @param misc1-misc2 Weitere Analogwerte (NO_VALUE zum Überspringen)
     * @return Größe des erstellten Payloads
     */
    size_t buildPayload(uint8_t* buffer, size_t bufferSize,
                       float temp1 = NO_VALUE, float temp2 = NO_VALUE, 
                       float temp3 = NO_VALUE, float temp4 = NO_VALUE,
                       float defl1 = NO_VALUE, float defl2 = NO_VALUE, float defl3 = NO_VALUE,
                       float press1 = NO_VALUE, float press2 = NO_VALUE,
                       float misc1 = NO_VALUE, float misc2 = NO_VALUE);
    
    /**
     * Erstellt CayenneLPP Payload und gibt PayloadResult zurück
     * @param temp1-temp4 Temperaturwerte (NO_VALUE zum Überspringen)
     * @param defl1-defl3 Deflection/Analogwerte (NO_VALUE zum Überspringen)
     * @param press1-press2 Druckwerte (NO_VALUE zum Überspringen)
     * @param misc1-misc2 Weitere Analogwerte (NO_VALUE zum Überspringen)
     * @return PayloadResult mit Buffer und Größe
     */
    PayloadResult buildCayennePayload(float temp1 = NO_VALUE, float temp2 = NO_VALUE, 
                                     float temp3 = NO_VALUE, float temp4 = NO_VALUE,
                                     float defl1 = NO_VALUE, float defl2 = NO_VALUE, float defl3 = NO_VALUE,
                                     float press1 = NO_VALUE, float press2 = NO_VALUE,
                                     float misc1 = NO_VALUE, float misc2 = NO_VALUE);
    
    /**
     * Gibt den internen CayenneLPP-Buffer zurück
     * @return Pointer auf den Buffer
     */
    uint8_t* getBuffer();
    
    /**
     * Gibt die aktuelle Payload-Größe zurück
     * @return Payload-Größe in Bytes
     */
    size_t getSize();
    
    /**
     * Gibt das CayenneLPP-Objekt für erweiterte Funktionen zurück
     * @return Pointer auf das interne CayenneLPP-Objekt
     */
    CayenneLPP* getCayenneLPP();
    
    /**
     * Dekodiert und zeigt CayenneLPP-Payload an (für Debug)
     * @param payload Payload bytes
     * @param size Größe des Payloads
     */
    static void decodePayload(const uint8_t* payload, size_t size);
    
    /**
     * Dekodiert CayenneLPP Payload in SensorData Struktur
     * @param payload Payload bytes
     * @param size Größe des Payloads
     * @param deviceId Optional device ID
     * @return SensorData mit dekodierten Werten
     */
    static SensorData decodeCayenneToSensorData(const uint8_t* payload, size_t size, const String& deviceId = "");
    
    /**
     * Zeigt Payload als Hex-String an (für Debug)
     * @param payload Payload bytes
     * @param size Größe des Payloads
     */
    static void printPayloadHex(const uint8_t* payload, size_t size);
    
private:
    /**
     * Gibt den nächsten freien Kanal zurück
     * @return Kanalnummer
     */
    uint8_t getNextChannel();
};

#endif // PAYLOAD_BUILDER_CAYENNE_H
