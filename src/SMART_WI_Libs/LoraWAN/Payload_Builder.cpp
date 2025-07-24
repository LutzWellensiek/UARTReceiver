/**
 * @file Payload_Builder.cpp
 * @brief Simple LoRaWAN payload builder and decoder implementation
 * @author Smart Wire Industries
 * @version 1.0.0
 * @date 2025-01-17
 */

#include "Payload_Builder.h"
#include "../SerialMon.h"

// Helper function to convert float to bytes
void floatToBytes(float value, uint8_t* bytes) {
    memcpy(bytes, &value, sizeof(float));
}

// Helper function to convert bytes to float
float bytesToFloat(const uint8_t* bytes) {
    float value;
    memcpy(&value, bytes, sizeof(float));
    return value;
}

PayloadResult buildPayload(uint8_t* buffer, size_t bufferSize,
                   float temp1, float temp2, float temp3, float temp4,
                   float defl1, float defl2, float defl3,
                   float press1, float press2,
                   float misc1, float misc2) {
    
    // offset = aktuelle Position im Buffer (startet bei 0)
    // Wird nach jedem hinzugefügten Byte/Element erhöht
    size_t offset = 0;
    
    // ========== TEMPERATUR-WERTE VERARBEITEN ==========
    // Alle Temperaturwerte in ein Array packen für einfachere Verarbeitung
    float temperatures[4] = {temp1, temp2, temp3, temp4};
    
    // Zählen, wie viele gültige Temperaturwerte vorhanden sind
    // (NaN bedeutet "nicht verwendet")
    uint8_t tempCount = 0;
    for (int i = 0; i < 4; i++) {
        if (!isnan(temperatures[i])) tempCount++;  // Wenn nicht NaN, dann zählen
    }
    
    // Nur wenn mindestens ein Temperaturwert vorhanden ist
    if (tempCount > 0) {
        // Prüfen ob genug Platz im Buffer: 1 Byte für 'T' + (Anzahl Werte * 4 Bytes pro Float)
        if (offset + 1 + (tempCount * 4) > bufferSize) return {nullptr, 0};  // Fehler: Buffer zu klein
        
        // TLV: Typ (Tag), Länge und Wert
        for (int i = 0; i < 4; i++) {
            if (!isnan(temperatures[i])) {
                // Stellen Sie sicher, dass genug Platz für Tag, Länge und Wert vorhanden ist
                if (offset + 1 + 1 + 4 > bufferSize) return {nullptr, 0};

                buffer[offset++] = TAG_TEMPERATURE;  // Tag für Temperatur
                buffer[offset++] = 4;     // Length

                floatToBytes(temperatures[i], &buffer[offset]);
                offset += 4;
            }
        }
    }
    // Nach diesem Block: Buffer enthält z.B. [0x01][0x04][temp1_bytes][0x01][0x04][temp2_bytes]...
    
    // ========== DEFLECTION-WERTE VERARBEITEN ==========
    // Gleiche Logik wie bei Temperatur, aber für Deflection-Werte
    float deflections[3] = {defl1, defl2, defl3};
    
    uint8_t deflCount = 0;
    for (int i = 0; i < 3; i++) {
        if (!isnan(deflections[i])) deflCount++;
    }
    
    if (deflCount > 0) {
        // Prüfen ob genug Platz im Buffer (zusätzlich zu dem was schon drin ist)
        if (offset + 1 + (deflCount * 4) > bufferSize) return {nullptr, 0};
        
        // TLV: Typ (Tag), Länge und Wert
        for (int i = 0; i < 3; i++) {
            if (!isnan(deflections[i])) {
                // Stellen Sie sicher, dass genug Platz für Tag, Länge und Wert vorhanden ist
                if (offset + 1 + 1 + 4 > bufferSize) return {nullptr, 0};

                buffer[offset++] = TAG_DEFLECTION;  // Tag für Deflection
                buffer[offset++] = 4;     // Length

                floatToBytes(deflections[i], &buffer[offset]);
                offset += 4;
            }
        }
    }
    // Nach diesem Block: Buffer enthält zusätzlich [0x02][0x04][defl1_bytes]...
    
    // ========== DRUCK-WERTE VERARBEITEN ==========
    float pressures[2] = {press1, press2};
    
    uint8_t pressCount = 0;
    for (int i = 0; i < 2; i++) {
        if (!isnan(pressures[i])) pressCount++;
    }
    
    if (pressCount > 0) {
        if (offset + 1 + (pressCount * 4) > bufferSize) return {nullptr, 0};
        
        // TLV: Typ (Tag), Länge und Wert
        for (int i = 0; i < 2; i++) {
            if (!isnan(pressures[i])) {
                // Stellen Sie sicher, dass genug Platz für Tag, Länge und Wert vorhanden ist
                if (offset + 1 + 1 + 4 > bufferSize) return {nullptr, 0};

                buffer[offset++] = TAG_PRESSURE;  // Tag für Pressure
                buffer[offset++] = 4;     // Length

                floatToBytes(pressures[i], &buffer[offset]);
                offset += 4;
            }
        }
    }
    
    // ========== SONSTIGE WERTE VERARBEITEN ==========
    float miscValues[2] = {misc1, misc2};
    
    uint8_t miscCount = 0;
    for (int i = 0; i < 2; i++) {
        if (!isnan(miscValues[i])) miscCount++;
    }
    
    if (miscCount > 0) {
        if (offset + 1 + (miscCount * 4) > bufferSize) return {nullptr, 0};
        
        // TLV: Typ (Tag), Länge und Wert
        for (int i = 0; i < 2; i++) {
            if (!isnan(miscValues[i])) {
                // Stellen Sie sicher, dass genug Platz für Tag, Länge und Wert vorhanden ist
                if (offset + 1 + 1 + 4 > bufferSize) return {nullptr, 0};

                buffer[offset++] = TAG_MISC;  // Tag für Misc
                buffer[offset++] = 4;     // Length

                floatToBytes(miscValues[i], &buffer[offset]);
                offset += 4;
            }
        }
    }
    
    // Rückgabe: Befüllter Buffer und dessen Größe
    // Beispiel mit TLV: Wenn 2 Temps + 1 Deflection verwendet werden:
    // 2*(1 Byte Tag + 1 Byte Length + 4 Bytes Float) + 1*(1 Byte Tag + 1 Byte Length + 4 Bytes Float) = 18 Bytes
    return {buffer, offset};
}

void decodePayload(const uint8_t* payload, size_t size) {
    // Nutze immer SerialMon für Debug-Ausgaben
    
    size_t offset = 0;
    
    // Header entfernt für kompaktere Ausgabe
    
    while (offset < size) {
        if (offset + 2 > size) {
            // Fehler: Ungültige TLV-Struktur
            SerialMon.println(F("Invalid TLV structure"));
            return;
        }

        uint8_t tag = payload[offset++];
        uint8_t length = payload[offset++];

        if (offset + length > size) {
            // Fehler: Daten sind nicht vollständig
            SerialMon.println(F("Incomplete data for TLV"));
            return;
        }

        switch (tag) {
            case TAG_TEMPERATURE:
                if (length == 4) {
                    float temp = bytesToFloat(&payload[offset]);
                    SerialMon.print(F("Temperature: "));
                    SerialMon.print(temp, 2);
                    SerialMon.println(F("°C"));
                    offset += length;
                } else {
                    SerialMon.println(F("Invalid temperature data length"));
                    offset += length;
                }
                break;
                
            case TAG_DEFLECTION:
                if (length == 4) {
                    float defl = bytesToFloat(&payload[offset]);
                    SerialMon.print(F("Deflection: "));
                    SerialMon.print(defl, 4);
                    SerialMon.println(F("mm"));
                    offset += length;
                } else {
                    SerialMon.println(F("Invalid deflection data length"));
                    offset += length;
                }
                break;
                
            case TAG_PRESSURE:
                if (length == 4) {
                    float press = bytesToFloat(&payload[offset]);
                    SerialMon.print(F("Pressure: "));
                    SerialMon.print(press, 2);
                    SerialMon.println(F("hPa"));
                    offset += length;
                } else {
                    SerialMon.println(F("Invalid pressure data length"));
                    offset += length;
                }
                break;
                
            case TAG_MISC:
                if (length == 4) {
                    float miscVal = bytesToFloat(&payload[offset]);
                    SerialMon.print(F("Misc: "));
                    SerialMon.println(miscVal, 2);
                    offset += length;
                } else {
                    SerialMon.println(F("Invalid misc data length"));
                    offset += length;
                }
                break;
                
            default:
                SerialMon.print(F("Unknown tag: 0x"));
                SerialMon.print(tag, HEX);
                SerialMon.print(F(" with length: "));
                SerialMon.println(length);
                // Skip unknown tags
                offset += length;
                break;
        }
    }
    
    // Footer entfernt für kompaktere Ausgabe
}

void printPayloadHex(const uint8_t* payload, size_t size) {
    SerialMon.print("Payload Hex: ");
    for (size_t i = 0; i < size; i++) {
        if (payload[i] < 0x10) {
            SerialMon.print("0");
        }
        SerialMon.print(payload[i], HEX);
        SerialMon.print(" ");
    }
    SerialMon.println();
}
