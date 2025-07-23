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
        
        // Typ-Kennzeichnung 'T' (0x54) in den Buffer schreiben
        buffer[offset++] = 'T';  // offset wird nach dem Schreiben um 1 erhöht
        
        // Alle gültigen Temperaturwerte in den Buffer schreiben
        for (int i = 0; i < 4; i++) {
            if (!isnan(temperatures[i])) {  // Nur gültige Werte
                // Float in 4 Bytes umwandeln und in Buffer schreiben
                floatToBytes(temperatures[i], &buffer[offset]);
                offset += 4;  // offset um 4 erhöhen (1 Float = 4 Bytes)
            }
        }
    }
    // Nach diesem Block: Buffer enthält z.B. ['T'][temp1_byte1][temp1_byte2][temp1_byte3][temp1_byte4][temp2_byte1]...
    
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
        
        // Typ-Kennzeichnung 'D' (0x44) in den Buffer schreiben
        buffer[offset++] = 'D';
        
        // Alle gültigen Deflection-Werte hinzufügen
        for (int i = 0; i < 3; i++) {
            if (!isnan(deflections[i])) {
                floatToBytes(deflections[i], &buffer[offset]);
                offset += 4;
            }
        }
    }
    // Nach diesem Block: Buffer enthält zusätzlich ['D'][defl1_bytes][defl2_bytes]...
    
    // ========== DRUCK-WERTE VERARBEITEN ==========
    float pressures[2] = {press1, press2};
    
    uint8_t pressCount = 0;
    for (int i = 0; i < 2; i++) {
        if (!isnan(pressures[i])) pressCount++;
    }
    
    if (pressCount > 0) {
        if (offset + 1 + (pressCount * 4) > bufferSize) return {nullptr, 0};
        
        // Typ-Kennzeichnung 'P' (0x50) in den Buffer schreiben
        buffer[offset++] = 'P';
        
        for (int i = 0; i < 2; i++) {
            if (!isnan(pressures[i])) {
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
        
        // Typ-Kennzeichnung 'S' (0x53) in den Buffer schreiben
        buffer[offset++] = 'S';
        
        for (int i = 0; i < 2; i++) {
            if (!isnan(miscValues[i])) {
                floatToBytes(miscValues[i], &buffer[offset]);
                offset += 4;
            }
        }
    }
    
    // Rückgabe: Befüllter Buffer und dessen Größe
    // Beispiel: Wenn 2 Temps + 1 Deflection verwendet werden:
    // 1 Byte ('T') + 8 Bytes (2 Floats) + 1 Byte ('D') + 4 Bytes (1 Float) = 14 Bytes
    return {buffer, offset};
}

void decodePayload(const uint8_t* payload, size_t size) {
    // Nutze immer SerialMon für Debug-Ausgaben
    
    size_t offset = 0;
    
    SerialMon.println(F("=== Payload Decode ==="));
    
    while (offset < size) {
        char sensorType = (char)payload[offset++];
        
        switch (sensorType) {
            case 'T':
                SerialMon.print(F("Temperature: "));
                while (offset + 4 <= size && offset < size && 
                       (payload[offset] != 'T' && payload[offset] != 'D' && 
                        payload[offset] != 'P' && payload[offset] != 'S')) {
                    float temp = bytesToFloat(&payload[offset]);
                    SerialMon.print(temp, 2);
                    SerialMon.print(F("°C "));
                    offset += 4;
                }
                SerialMon.println();
                break;
                
            case 'D':
                SerialMon.print(F("Deflection: "));
                while (offset + 4 <= size && offset < size && 
                       (payload[offset] != 'T' && payload[offset] != 'D' && 
                        payload[offset] != 'P' && payload[offset] != 'S')) {
                    float defl = bytesToFloat(&payload[offset]);
                    SerialMon.print(defl, 4);
                    SerialMon.print(F("mm "));
                    offset += 4;
                }
                SerialMon.println();
                break;
                
            case 'P':
                SerialMon.print(F("Pressure: "));
                while (offset + 4 <= size && offset < size && 
                       (payload[offset] != 'T' && payload[offset] != 'D' && 
                        payload[offset] != 'P' && payload[offset] != 'S')) {
                    float press = bytesToFloat(&payload[offset]);
                    SerialMon.print(press, 2);
                    SerialMon.print(F("hPa "));
                    offset += 4;
                }
                SerialMon.println();
                break;
                
            case 'S':
                SerialMon.print(F("Misc: "));
                while (offset + 4 <= size && offset < size && 
                       (payload[offset] != 'T' && payload[offset] != 'D' && 
                        payload[offset] != 'P' && payload[offset] != 'S')) {
                    float miscVal = bytesToFloat(&payload[offset]);
                    SerialMon.print(miscVal, 2);
                    SerialMon.print(F(" "));
                    offset += 4;
                }
                SerialMon.println();
                break;
                
            default:
                SerialMon.print(F("Unknown sensor type: "));
                SerialMon.println(sensorType);
                return;
        }
    }
    
    SerialMon.println(F("=== End Decode ==="));
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
