/**
 * SX1262_LoRaWAN.cpp
 * Implementierung der SX1262_LoRaWAN Klasse
 */

#include "SX1262_LoRaWAN.h"
#include "lorawanconfig.h"

// SerialMon für AVR
#if defined (__AVR__)
    #define SerialMon Serial3
#else
    #define SerialMon Serial
#endif

SX1262_LoRaWAN::SX1262_LoRaWAN() {
    // Konstruktor
}

SX1262_LoRaWAN::~SX1262_LoRaWAN() {}

// ================================================================
// INITIALISIERUNG UND KONFIGURATION
// ================================================================

bool SX1262_LoRaWAN::initializeRadio() {
    SerialMon.print("Initialisiere SX1262...");
    int state = radio.begin();
    if (state == RADIOLIB_ERR_NONE) {
        SerialMon.println(" Erfolgreich!");
        return true;
    } else {
        SerialMon.print(" Fehler: ");
        SerialMon.println(state);
        return false;
    }
}

bool SX1262_LoRaWAN::initializeLoRaWAN() {
    SerialMon.println("Initialisiere LoRaWAN Node...");
    int state = node.beginOTAA(joinEUI, devEUI, nwkKey, appKey);
    if (state == RADIOLIB_ERR_NONE) {
        SerialMon.println("Node initialisiert!");
        return true;
    } else {
        SerialMon.print("Node-Initialisierung fehlgeschlagen: ");
        SerialMon.println(state);
        return false;
    }
}

bool SX1262_LoRaWAN::joinNetwork() {
    int maxRetries = 5;  // Maximale Anzahl von Versuchen
    int retryDelay = 30000;  // 30 Sekunden Wartezeit
    
    for (int attempt = 1; attempt <= maxRetries; attempt++) {
        SerialMon.print("Sende Join-Request... (Versuch ");
        SerialMon.print(attempt);
        SerialMon.print("/");
        SerialMon.print(maxRetries);
        SerialMon.println(")");
        
        int state = node.activateOTAA();
        if (state == RADIOLIB_LORAWAN_NEW_SESSION) {
            SerialMon.println("✅ Join erfolgreich!");
            return true;
        } else {
            SerialMon.print("❌ Join fehlgeschlagen: ");
            SerialMon.println(state);
            
            // Wenn es nicht der letzte Versuch ist, warten
            if (attempt < maxRetries) {
                SerialMon.print("⏰ Warte ");
                SerialMon.print(retryDelay / 1000);
                SerialMon.println(" Sekunden bis zum nächsten Versuch...");
                delay(retryDelay);
            }
        }
    }
    
    SerialMon.println("❌ Alle Join-Versuche fehlgeschlagen!");
    return false;
}

bool SX1262_LoRaWAN::initializeEverything() {
    // Keys initialisieren
    initializeKeys();
    
    // Radio initialisieren
    if (!initializeRadio()) {
        return false;
    }
    
    // LoRaWAN initialisieren
    if (!initializeLoRaWAN()) {
        return false;
    }
    
    // Netzwerk beitreten
    if (!joinNetwork()) {
        return false;
    }
    
    SerialMon.println("Setup abgeschlossen. Bereit für Datenübertragung...");
    return true;
}

// ================================================================
// PAYLOAD FUNKTIONEN
// ================================================================

int SX1262_LoRaWAN::buildModularPayload(float temp1, float temp2, float deflection, float pressure, float picTemp, uint8_t* payload, int maxSize) {
    // Bestimme, welche Werte verwendet werden sollen
    float t1 = NO_VALUE, t2 = NO_VALUE, t3 = NO_VALUE, t4 = NO_VALUE;
    float d1 = NO_VALUE, d2 = NO_VALUE, d3 = NO_VALUE;
    float p1 = NO_VALUE, p2 = NO_VALUE;
    float m1 = NO_VALUE, m2 = NO_VALUE;
    
    // Temperatursensoren
    if (TEMP_SENSOR_AKTIV) {
        t1 = temp1;
        if (anzahl_temp_values > 1) {
            t2 = temp2;
        }
    }
    
    // Deflection-Sensor
    if (DEFLECTION_SENSOR_AKTIV) {
        d1 = deflection;
    }
    
    // Druck-Sensor
    if (PRESSURE_SENSOR_AKTIV) {
        p1 = pressure;
    }
    
    // Sonstiges (PIC-Temperatur)
    if (SONSTIGES_SENSOR_AKTIV) {
        m1 = picTemp;
    }
    
    // Payload mit neuer buildPayload-Funktion erstellen
    PayloadResult result = buildPayload(payload, maxSize, t1, t2, t3, t4, d1, d2, d3, p1, p2, m1, m2);
    
    if (result.buffer == nullptr) {
        return 0;  // Fehler
    }
    
    return result.size;
}

void SX1262_LoRaWAN::displayPayloadContent(uint8_t* payload, int payloadSize) {
    // Verwende die neuen Debug-Funktionen
    printPayloadHex(payload, payloadSize, &SerialMon);
    decodePayload(payload, payloadSize, &SerialMon);
}

// ================================================================
// LORAWAN KOMMUNIKATION
// ================================================================

bool SX1262_LoRaWAN::initializeSetup() {
    // Zufalls-Generator für bessere Entropie initialisieren
    randomSeed(analogRead(0) + millis() + micros());
    
    SerialMon.println("=== SX1262 LoRaWAN Endknoten ===");
    
    // Debug-Informationen anzeigen
    printDebugInfo();
    printPinConfiguration();
    
    // Keys initialisieren
    initializeKeys();
    
    // Radio initialisieren
    if (!initializeRadio()) {
        return false;
    }
    
    // LoRaWAN initialisieren
    if (!initializeLoRaWAN()) {
        return false;
    }
    
    // Netzwerk beitreten
    if (!joinNetwork()) {
        return false;
    }
    
    SerialMon.println("✅ LoRaWAN bereit - Beginne mit Datenübertragung...");
    return true;
}

bool SX1262_LoRaWAN::sendPayload(const uint8_t* payload, size_t payloadSize) {
    // Eingabe-Validierung
    if (payload == nullptr || payloadSize == 0) {
        SerialMon.println("❌ Ungültiges Payload!");
        return false;
    }
    
    // Payload-Größe basierend auf aktuellem Spreading Factor prüfen
    size_t maxPayloadSize = getMaxPayloadSize();
    if (payloadSize > maxPayloadSize) {
        SerialMon.print("❌ Payload zu groß (");
        SerialMon.print(payloadSize);
        SerialMon.print(" bytes) für aktuellen SF. Maximum: ");
        SerialMon.print(maxPayloadSize);
        SerialMon.println(" bytes");
        return false;
    }
    
    // Debug-Ausgabe
    SerialMon.print("📦 Sende Payload (");
    SerialMon.print(payloadSize);
    SerialMon.println(" Bytes):");
    
    // Payload-Inhalt anzeigen
    SerialMon.print("Payload Hex: ");
    for (size_t i = 0; i < payloadSize; i++) {
        if (payload[i] < 0x10) SerialMon.print("0");
        SerialMon.print(payload[i], HEX);
        SerialMon.print(" ");
    }
    SerialMon.println();
    
    // Daten senden
    SerialMon.println("📡 Sende Payload...");
    int state = node.sendReceive(payload, payloadSize);
    
    if (state < RADIOLIB_ERR_NONE) {
        SerialMon.print("❌ Uplink-Fehler: ");
        SerialMon.println(state);
        return false;
    } else if (state == 0) {
        SerialMon.println("✅ Uplink erfolgreich, kein Downlink empfangen");
    } else {
        SerialMon.print("✅ Uplink erfolgreich, Downlink empfangen in Fenster Rx");
        SerialMon.println(state);
    }
    
    return true;
}



// ================================================================
// DEBUG UND HILFSFUNKTIONEN
// ================================================================

void SX1262_LoRaWAN::printDebugInfo() {
    SerialMon.print("Join EUI: ");
    SerialMon.printf(joinEUI_str);
    SerialMon.print("Dev EUI: ");
    SerialMon.printf(devEUI_str);
    SerialMon.print("App Key: ");
    SerialMon.printf(appKey_str);
    SerialMon.println();
    SerialMon.print("Nwk Key: ");
    SerialMon.printf(nwkKey_str);
    SerialMon.println();
}

void SX1262_LoRaWAN::printPinConfiguration() {
    SerialMon.print("Pin-Konfiguration: CS=");
    SerialMon.print(NSS_PIN);
    SerialMon.print(", RESET=");
    SerialMon.print(RESET_PIN);
    SerialMon.print(", BUSY=");
    SerialMon.print(BUSY_PIN);
    SerialMon.print(", DIO1=");
    SerialMon.println(DIO1_PIN);
}

// ================================================================
// PRIVATE HILFSMETHODEN
// ================================================================

void SX1262_LoRaWAN::configureLoRaWAN() {
    // Placeholder function to set up LoRaWAN settings
    SerialMon.println("Configuring LoRaWAN...");
}

size_t SX1262_LoRaWAN::getMaxPayloadSize() {
    // Zurzeit hardcoded auf EU868 DR5 (SF7)
    // TODO: Dynamisch basierend auf aktuellem Spreading Factor
    return 242;  // EU868 DR5 Maximum
    
    // Alternative: Basierend auf aktuellem DR
    // int dataRate = node.getDataRate();
    // switch (dataRate) {
    //     case 0: case 1: case 2: return 51;   // SF12, SF11, SF10
    //     case 3: return 115;                  // SF9
    //     case 4: case 5: return 242;          // SF8, SF7
    //     default: return 51;                  // Sicherheitswert
    // }
}

