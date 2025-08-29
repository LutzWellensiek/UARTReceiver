/**
 * @file Payload_Builder_Cayenne.cpp
 * @brief CayenneLPP-based LoRaWAN payload builder implementation
 * @author Smart Wire Industries
 * @version 2.0.0
 * @date 2025-01-28
 */

#include "Payload_Builder_Cayenne.h"
#include "../SerialMon.h"
#include "ChirpStackReceiver/ChirpStackReceiver.h"

// Konstruktor
Payload_Builder_Cayenne::Payload_Builder_Cayenne(uint16_t bufferSize) {
    lpp = new CayenneLPP(bufferSize);
    nextChannel = 1;  // Kanäle starten bei 1
}

// Destruktor
Payload_Builder_Cayenne::~Payload_Builder_Cayenne() {
    if (lpp != nullptr) {
        delete lpp;
    }
}

// Payload zurücksetzen
void Payload_Builder_Cayenne::reset() {
    lpp->reset();
    nextChannel = 1;  // Kanalnummerierung zurücksetzen
}

// Nächsten freien Kanal bekommen
uint8_t Payload_Builder_Cayenne::getNextChannel() {
    return nextChannel++;
}

// Temperaturwerte hinzufügen (Array)
bool Payload_Builder_Cayenne::addTemperatures(float* temperatures, uint8_t count) {
    bool success = true;
    for (uint8_t i = 0; i < count; i++) {
        if (!isnan(temperatures[i])) {
            if (!addTemperature(temperatures[i])) {
                success = false;
            }
        }
    }
    return success;
}

// Einzelnen Temperaturwert hinzufügen
bool Payload_Builder_Cayenne::addTemperature(float temperature, uint8_t channel) {
    if (isnan(temperature)) return true;  // Skip NaN values
    
    uint8_t ch = (channel == 0) ? getNextChannel() : channel;
    return lpp->addTemperature(ch, temperature) == 0;
}

// Druckwerte hinzufügen (Array)
bool Payload_Builder_Cayenne::addPressures(float* pressures, uint8_t count) {
    bool success = true;
    for (uint8_t i = 0; i < count; i++) {
        if (!isnan(pressures[i])) {
            if (!addPressure(pressures[i])) {
                success = false;
            }
        }
    }
    return success;
}

// Einzelnen Druckwert hinzufügen
bool Payload_Builder_Cayenne::addPressure(float pressure, uint8_t channel) {
    if (isnan(pressure)) return true;  // Skip NaN values
    
    uint8_t ch = (channel == 0) ? getNextChannel() : channel;
    return lpp->addBarometricPressure(ch, pressure) == 0;
}

// Analogwerte hinzufügen (Array)
bool Payload_Builder_Cayenne::addAnalogValues(float* values, uint8_t count) {
    bool success = true;
    for (uint8_t i = 0; i < count; i++) {
        if (!isnan(values[i])) {
            if (!addAnalogValue(values[i])) {
                success = false;
            }
        }
    }
    return success;
}

// Einzelnen Analogwert hinzufügen
bool Payload_Builder_Cayenne::addAnalogValue(float value, uint8_t channel) {
    if (isnan(value)) return true;  // Skip NaN values
    
    uint8_t ch = (channel == 0) ? getNextChannel() : channel;
    return lpp->addAnalogInput(ch, value) == 0;
}

// Feuchtigkeitswert hinzufügen
bool Payload_Builder_Cayenne::addHumidity(float humidity, uint8_t channel) {
    if (isnan(humidity)) return true;  // Skip NaN values
    
    uint8_t ch = (channel == 0) ? getNextChannel() : channel;
    return lpp->addRelativeHumidity(ch, humidity) == 0;
}

// Digitalwert hinzufügen
bool Payload_Builder_Cayenne::addDigitalInput(uint8_t value, uint8_t channel) {
    uint8_t ch = (channel == 0) ? getNextChannel() : channel;
    return lpp->addDigitalInput(ch, value) == 0;
}

// GPS-Position hinzufügen
bool Payload_Builder_Cayenne::addGPS(float latitude, float longitude, float altitude, uint8_t channel) {
    uint8_t ch = (channel == 0) ? getNextChannel() : channel;
    return lpp->addGPS(ch, latitude, longitude, altitude) == 0;
}

// Beschleunigungswerte hinzufügen
bool Payload_Builder_Cayenne::addAccelerometer(float x, float y, float z, uint8_t channel) {
    uint8_t ch = (channel == 0) ? getNextChannel() : channel;
    return lpp->addAccelerometer(ch, x, y, z) == 0;
}

// Helligkeitswert hinzufügen
bool Payload_Builder_Cayenne::addLuminosity(uint16_t lux, uint8_t channel) {
    uint8_t ch = (channel == 0) ? getNextChannel() : channel;
    return lpp->addLuminosity(ch, lux) == 0;
}

// Kompatibilitätsfunktion für altes Interface
size_t Payload_Builder_Cayenne::buildPayload(uint8_t* buffer, size_t bufferSize,
                                            float temp1, float temp2, float temp3, float temp4,
                                            float defl1, float defl2, float defl3,
                                            float press1, float press2,
                                            float misc1, float misc2) {
    
    // Payload zurücksetzen
    reset();
    
    // Temperaturwerte hinzufügen
    float temperatures[4] = {temp1, temp2, temp3, temp4};
    for (int i = 0; i < 4; i++) {
        if (!isnan(temperatures[i])) {
            addTemperature(temperatures[i]);
        }
    }
    
    // Deflection-Werte als Analogwerte hinzufügen
    float deflections[3] = {defl1, defl2, defl3};
    for (int i = 0; i < 3; i++) {
        if (!isnan(deflections[i])) {
            addAnalogValue(deflections[i]);
        }
    }
    
    // Druckwerte hinzufügen
    float pressures[2] = {press1, press2};
    for (int i = 0; i < 2; i++) {
        if (!isnan(pressures[i])) {
            addPressure(pressures[i]);
        }
    }
    
    // Misc-Werte als weitere Analogwerte hinzufügen
    float miscValues[2] = {misc1, misc2};
    for (int i = 0; i < 2; i++) {
        if (!isnan(miscValues[i])) {
            addAnalogValue(miscValues[i]);
        }
    }
    
    // Buffer kopieren
    size_t payloadSize = lpp->getSize();
    if (payloadSize > bufferSize) {
        SerialMon.println(F("❌ Buffer zu klein für CayenneLPP Payload!"));
        return 0;
    }
    
    memcpy(buffer, lpp->getBuffer(), payloadSize);
    return payloadSize;
}

// Buffer zurückgeben
uint8_t* Payload_Builder_Cayenne::getBuffer() {
    return lpp->getBuffer();
}

// Payload-Größe zurückgeben
size_t Payload_Builder_Cayenne::getSize() {
    return lpp->getSize();
}

// CayenneLPP-Objekt für erweiterte Funktionen
CayenneLPP* Payload_Builder_Cayenne::getCayenneLPP() {
    return lpp;
}

// Erstellt CayenneLPP Payload und gibt PayloadResult zurück
PayloadResult Payload_Builder_Cayenne::buildCayennePayload(float temp1, float temp2, 
                                                           float temp3, float temp4,
                                                           float defl1, float defl2, float defl3,
                                                           float press1, float press2,
                                                           float misc1, float misc2) {
    // Reset payload für neue Daten
    reset();
    
    uint8_t channel = 1;
    
    // Temperaturen hinzufügen
    if (!isnan(temp1)) {
        addTemperature(temp1, channel++);
    }
    if (!isnan(temp2)) {
        addTemperature(temp2, channel++);
    }
    if (!isnan(temp3)) {
        addTemperature(temp3, channel++);
    }
    if (!isnan(temp4)) {
        addTemperature(temp4, channel++);
    }
    
    // Deflection/Analog-Werte hinzufügen
    if (!isnan(defl1)) {
        addAnalogValue(defl1, channel++);
    }
    if (!isnan(defl2)) {
        addAnalogValue(defl2, channel++);
    }
    if (!isnan(defl3)) {
        addAnalogValue(defl3, channel++);
    }
    
    // Druckwerte hinzufügen
    if (!isnan(press1)) {
        addPressure(press1, channel++);
    }
    if (!isnan(press2)) {
        addPressure(press2, channel++);
    }
    
    // Misc-Werte als Analog hinzufügen
    if (!isnan(misc1)) {
        addAnalogValue(misc1, channel++);
    }
    if (!isnan(misc2)) {
        addAnalogValue(misc2, channel++);
    }
    
    // PayloadResult erstellen und zurückgeben
    PayloadResult result;
    result.buffer = getBuffer();
    result.size = getSize();
    
    return result;
}

// Statische Dekodierungsfunktion (für Debug)
void Payload_Builder_Cayenne::decodePayload(const uint8_t* payload, size_t size) {
    SerialMon.println(F("=== CayenneLPP Payload Decode ==="));
    
    size_t offset = 0;
    
    while (offset < size) {
        if (offset + 2 > size) {
            SerialMon.println(F("❌ Unvollständige Daten"));
            break;
        }
        
        uint8_t channel = payload[offset++];
        uint8_t type = payload[offset++];
        
        SerialMon.print(F("Kanal "));
        SerialMon.print(channel);
        SerialMon.print(F(": "));
        
        switch (type) {
            case LPP_TEMPERATURE: {
                if (offset + 2 > size) break;
                int16_t temp = (payload[offset] << 8) | payload[offset + 1];
                SerialMon.print(F("Temperatur = "));
                SerialMon.print(temp / 10.0);
                SerialMon.println(F("°C"));
                offset += 2;
                break;
            }
            
            case LPP_BAROMETRIC_PRESSURE: {
                if (offset + 2 > size) break;
                uint16_t pressure = (payload[offset] << 8) | payload[offset + 1];
                SerialMon.print(F("Druck = "));
                SerialMon.print(pressure / 10.0);
                SerialMon.println(F(" hPa"));
                offset += 2;
                break;
            }
            
            case LPP_ANALOG_INPUT: {
                if (offset + 2 > size) break;
                int16_t analog = (payload[offset] << 8) | payload[offset + 1];
                SerialMon.print(F("Analogwert = "));
                SerialMon.println(analog / 100.0);
                offset += 2;
                break;
            }
            
            case LPP_RELATIVE_HUMIDITY: {
                if (offset + 1 > size) break;
                uint8_t humidity = payload[offset];
                SerialMon.print(F("Feuchtigkeit = "));
                SerialMon.print(humidity / 2.0);
                SerialMon.println(F("%"));
                offset += 1;
                break;
            }
            
            case LPP_DIGITAL_INPUT: {
                if (offset + 1 > size) break;
                uint8_t digital = payload[offset];
                SerialMon.print(F("Digital = "));
                SerialMon.println(digital);
                offset += 1;
                break;
            }
            
            case LPP_GPS: {
                if (offset + 9 > size) break;
                int32_t lat = (payload[offset] << 16) | (payload[offset + 1] << 8) | payload[offset + 2];
                if (lat & 0x800000) lat |= 0xFF000000;  // Sign extend
                int32_t lon = (payload[offset + 3] << 16) | (payload[offset + 4] << 8) | payload[offset + 5];
                if (lon & 0x800000) lon |= 0xFF000000;  // Sign extend
                int32_t alt = (payload[offset + 6] << 16) | (payload[offset + 7] << 8) | payload[offset + 8];
                if (alt & 0x800000) alt |= 0xFF000000;  // Sign extend
                
                SerialMon.print(F("GPS = "));
                SerialMon.print(lat / 10000.0, 6);
                SerialMon.print(F("°, "));
                SerialMon.print(lon / 10000.0, 6);
                SerialMon.print(F("°, "));
                SerialMon.print(alt / 100.0);
                SerialMon.println(F("m"));
                offset += 9;
                break;
            }
            
            case LPP_ACCELEROMETER: {
                if (offset + 6 > size) break;
                int16_t x = (payload[offset] << 8) | payload[offset + 1];
                int16_t y = (payload[offset + 2] << 8) | payload[offset + 3];
                int16_t z = (payload[offset + 4] << 8) | payload[offset + 5];
                SerialMon.print(F("Beschleunigung = X:"));
                SerialMon.print(x / 1000.0);
                SerialMon.print(F("g, Y:"));
                SerialMon.print(y / 1000.0);
                SerialMon.print(F("g, Z:"));
                SerialMon.print(z / 1000.0);
                SerialMon.println(F("g"));
                offset += 6;
                break;
            }
            
            case LPP_LUMINOSITY: {
                if (offset + 2 > size) break;
                uint16_t lux = (payload[offset] << 8) | payload[offset + 1];
                SerialMon.print(F("Helligkeit = "));
                SerialMon.print(lux);
                SerialMon.println(F(" Lux"));
                offset += 2;
                break;
            }
            
            default:
                SerialMon.print(F("Unbekannter Typ: 0x"));
                SerialMon.println(type, HEX);
                // Kann nicht weiter decodieren, da Länge unbekannt
                return;
        }
    }
    
    SerialMon.println(F("=== Ende Decode ==="));
}

// Payload als Hex ausgeben
void Payload_Builder_Cayenne::printPayloadHex(const uint8_t* payload, size_t size) {
    SerialMon.print(F("CayenneLPP Hex ("));
    SerialMon.print(size);
    SerialMon.print(F(" bytes): "));
    
    for (size_t i = 0; i < size; i++) {
        if (payload[i] < 0x10) {
            SerialMon.print("0");
        }
        SerialMon.print(payload[i], HEX);
        SerialMon.print(" ");
    }
    SerialMon.println();
}

// Dekodiert CayenneLPP Payload in SensorData Struktur
SensorData Payload_Builder_Cayenne::decodeCayenneToSensorData(const uint8_t* payload, size_t size, const String& deviceId) {
    SensorData result;
    result.deviceId = deviceId;
    result.rawPayloadSize = size;
    result.lastUpdate = millis();
    
    if (!payload || size == 0) {
        return result;
    }
    
    size_t offset = 0;
    uint8_t tempCount = 0, deflCount = 0, pressCount = 0, miscCount = 0;
    
    while (offset < size) {
        if (offset + 2 > size) {
            break; // Nicht genug Daten für Channel + Type
        }
        
        uint8_t channel = payload[offset++];
        uint8_t type = payload[offset++];
        
        switch (type) {
            case LPP_TEMPERATURE: {
                if (offset + 2 > size) break;
                int16_t temp = (payload[offset] << 8) | payload[offset + 1];
                float value = temp / 10.0;
                result.addValue(SensorValue(TAG_TEMPERATURE, tempCount++, value));
                offset += 2;
                break;
            }
            
            case LPP_BAROMETRIC_PRESSURE: {
                if (offset + 2 > size) break;
                uint16_t pressure = (payload[offset] << 8) | payload[offset + 1];
                float value = pressure / 10.0;
                result.addValue(SensorValue(TAG_PRESSURE, pressCount++, value));
                offset += 2;
                break;
            }
            
            case LPP_ANALOG_INPUT: {
                if (offset + 2 > size) break;
                int16_t analog = (payload[offset] << 8) | payload[offset + 1];
                float value = analog / 100.0;
                // Analog values werden als Deflection interpretiert
                // (kann angepasst werden je nach Verwendung)
                result.addValue(SensorValue(TAG_DEFLECTION, deflCount++, value));
                offset += 2;
                break;
            }
            
            case LPP_RELATIVE_HUMIDITY: {
                if (offset + 1 > size) break;
                uint8_t humidity = payload[offset];
                float value = humidity / 2.0;
                result.addValue(SensorValue(TAG_MISC, miscCount++, value));
                offset += 1;
                break;
            }
            
            case LPP_DIGITAL_INPUT: {
                if (offset + 1 > size) break;
                uint8_t digital = payload[offset];
                float value = (float)digital;
                result.addValue(SensorValue(TAG_MISC, miscCount++, value));
                offset += 1;
                break;
            }
            
            case LPP_GPS: {
                if (offset + 9 > size) break;
                // GPS wird momentan übersprungen
                offset += 9;
                break;
            }
            
            case LPP_ACCELEROMETER: {
                if (offset + 6 > size) break;
                // Accelerometer wird momentan übersprungen
                offset += 6;
                break;
            }
            
            case LPP_LUMINOSITY: {
                if (offset + 2 > size) break;
                uint16_t lux = (payload[offset] << 8) | payload[offset + 1];
                float value = (float)lux;
                result.addValue(SensorValue(TAG_MISC, miscCount++, value));
                offset += 2;
                break;
            }
            
            default:
                // Unbekannter Typ - abbrechen
                return result;
        }
    }
    
    return result;
}
