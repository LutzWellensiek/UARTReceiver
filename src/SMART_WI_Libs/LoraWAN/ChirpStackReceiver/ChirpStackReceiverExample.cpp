/**
 * @file ChirpStackReceiverExample.cpp
 * @brief Beispiel-Implementierung für ChirpStackReceiver
 * @details Zeigt die korrekte Verwendung mit variabler Nachrichtenlänge
 */

#include <Arduino.h>
#include "ChirpStackReceiver.h"

// Beispiel für die Verwendung in main.cpp
void setupChirpStackReceiver() {
    // Verwende bereits existierende UARTReceiver Instanz
    extern UARTReceiver uartReceiver;
    
    // Setze Callbacks für verschiedene Datentypen
    uartReceiver.setBinaryCallback([](const uint8_t* data, size_t size) {
        // Die UARTReceiver Library handhabt bereits die Device-Name Extraktion
        // 'data' enthält nur die reine Payload
        
        SerialMon.println(F("\n=== CHIRPSTACK NACHRICHT ==="));
        SerialMon.print(F("Empfangene Bytes: "));
        SerialMon.println(size);
        
        // Wichtig: Prüfe ob genug Daten für TLV vorhanden sind
        if (size < 2) {
            SerialMon.println(F("[ERROR] Zu wenig Daten für TLV"));
            return;
        }
        
        // Verwende die Payload_Builder Dekodierung
        decodePayload(data, size);
        
        // Optional: Hex-Ausgabe
        SerialMon.print(F("\nRohdaten (Hex): "));
        for (size_t i = 0; i < size; i++) {
            if (data[i] < 16) SerialMon.print(F("0"));
            SerialMon.print(data[i], HEX);
            SerialMon.print(F(" "));
        }
        SerialMon.println(F("\n===========================\n"));
    });
    
    // Aktiviere Binärmodus
    uartReceiver.setBinaryMode(true);
    
    // Optional: Setze größeren Timeout für längere Nachrichten
    // uartReceiver.setTimeout(15000); // 15 Sekunden
}

// Hilfsfunktion zur Analyse von TLV-Strukturen
void analyzeTLVStructure(const uint8_t* data, size_t size) {
    SerialMon.println(F("\n[TLV-ANALYSE]"));
    
    size_t offset = 0;
    int tlvCount = 0;
    
    while (offset < size) {
        if (offset + 2 > size) {
            SerialMon.print(F("[WARNUNG] Unvollständige TLV-Struktur bei Offset "));
            SerialMon.println(offset);
            break;
        }
        
        uint8_t tag = data[offset];
        uint8_t length = data[offset + 1];
        
        SerialMon.print(F("TLV #"));
        SerialMon.print(++tlvCount);
        SerialMon.print(F(": Tag=0x"));
        if (tag < 16) SerialMon.print(F("0"));
        SerialMon.print(tag, HEX);
        
        switch(tag) {
            case 0x01: SerialMon.print(F(" (Temperature)")); break;
            case 0x02: SerialMon.print(F(" (Deflection)")); break;
            case 0x03: SerialMon.print(F(" (Pressure)")); break;
            case 0x04: SerialMon.print(F(" (Misc)")); break;
            default: SerialMon.print(F(" (Unknown)")); break;
        }
        
        SerialMon.print(F(", Length="));
        SerialMon.print(length);
        
        if (offset + 2 + length > size) {
            SerialMon.println(F(" [ERROR: Daten unvollständig!]"));
            break;
        } else {
            SerialMon.println(F(" [OK]"));
        }
        
        offset += 2 + length;
    }
    
    SerialMon.print(F("Verarbeitete Bytes: "));
    SerialMon.print(offset);
    SerialMon.print(F(" von "));
    SerialMon.println(size);
    
    if (offset != size) {
        SerialMon.print(F("[WARNUNG] "));
        SerialMon.print(size - offset);
        SerialMon.println(F(" Bytes übrig!"));
    }
}
