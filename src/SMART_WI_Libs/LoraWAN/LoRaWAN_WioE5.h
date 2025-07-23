/**
 * ═════════════════════════════════════════════════════════════════════════════
 *                      LoRaWAN WioE5 Library - README & EXAMPLES
 * ═════════════════════════════════════════════════════════════════════════════
 * 
 * ULTRA-EINFACHE LoRaWAN-LIBRARY für Grove Wio-E5 Module!
 * 
 * Entwickelt für AVR128DB48 Mikrocontroller mit folgenden Features:
 * ✅ ABP (Activation by Personalization) - EMPFOHLEN!
 * ✅ OTAA (Over-the-Air Activation)
 * ✅ Kompakte JSON-Nachrichten für LoRaWAN-Limits
 * ✅ Automatische Initialisierung
 * ✅ EU868 Frequenz-Unterstützung
 * ✅ Firmware-Update Funktionalität
 * ✅ Debug-Ausgaben über Serial Monitor
 * 
 * ═════════════════════════════════════════════════════════════════════════════
 *                                QUICK START EXAMPLES
 * ═════════════════════════════════════════════════════════════════════════════
 * 
 * ┌─────────────────────────────────────────────────────────────────────────┐
 * │ BEISPIEL 1: SUPER EINFACH - NUR 2 FUNKTIONEN!                         │
 * └─────────────────────────────────────────────────────────────────────────┘
 * 
 * #include "SMART_WI_Libs/LoRaWAN_WioE5.h"
 * #include "SMART_WI_Libs/SerialMon.h"
 * 
 * // Globale Instanz
 * LoRaWAN_WioE5 lora(&Serial2, &SerialMon);
 * 
 * void setup() {
 *     SerialMon.begin(115200);
 *     while (!SerialMon) delay(100);
 *     
 *     // ══════════════════════════════════════════════════════════════════
 *     // NUR EINE FUNKTION FÜR KOMPLETTE INITIALISIERUNG!
 *     // ══════════════════════════════════════════════════════════════════
 *     
 *     if (lora.initializeEverything()) {
 *         SerialMon.println("🎉 LoRaWAN bereit!");
 *     } else {
 *         SerialMon.println("❌ Initialisierung fehlgeschlagen!");
 *         while (1) delay(1000);
 *     }
 * }
 * 
 * void loop() {
 *     static unsigned long lastSend = 0;
 *     
 *     if (millis() - lastSend >= 30000) {  // Alle 30 Sekunden
 *         
 *         // Kompakte JSON für LoRaWAN erstellen (< 200 bytes!)
 *         String json = "{\"d\":[23.5,1013.2,0.045],\"t\":" + String(millis()/1000) + "}";
 *         
 *         // ══════════════════════════════════════════════════════════════
 *         // JSON SENDEN - NUR EINE FUNKTION!
 *         // ══════════════════════════════════════════════════════════════
 *         
 *         if (lora.sendJsonMessage(json.c_str())) {
 *             SerialMon.println("✅ JSON gesendet!");
 *         }
 *         
 *         lastSend = millis();
 *     }
 *     
 *     delay(100);
 * }
 * 
 * ┌─────────────────────────────────────────────────────────────────────────┐
 * │ BEISPIEL 2: NOCH EINFACHER - GLOBALE FUNKTIONEN                       │
 * └─────────────────────────────────────────────────────────────────────────┘
 * 
 * #include "SMART_WI_Libs/LoRaWAN_WioE5.h"
 * #include "SMART_WI_Libs/SerialMon.h"
 * 
 * void setup() {
 *     SerialMon.begin(115200);
 *     while (!SerialMon) delay(100);
 *     
 *     // NUR EINE ZEILE FÜR KOMPLETTE INITIALISIERUNG!
 *     if (LoRaWAN_initEverything()) {
 *         SerialMon.println("🎉 LoRaWAN bereit!");
 *     } else {
 *         SerialMon.println("❌ Initialisierung fehlgeschlagen!");
 *         while (1) delay(1000);
 *     }
 * }
 * 
 * void loop() {
 *     static unsigned long lastSend = 0;
 *     
 *     if (millis() - lastSend >= 30000) {  // Alle 30 Sekunden
 *         
 *         // JSON erstellen und senden - NUR EINE ZEILE!
 *         char json[100];
 *         sprintf(json, "{\"hello\":\"world\",\"time\":%lu}", millis()/1000);
 *         
 *         if (LoRaWAN_sendJson(json)) {
 *             SerialMon.println("✅ JSON gesendet!");
 *         }
 *         
 *         lastSend = millis();
 *     }
 *     
 *     delay(100);
 * }
 * 
 * ┌─────────────────────────────────────────────────────────────────────────┐
 * │ BEISPIEL 3: SENSORDATEN - KOMPAKTE JSON FÜR LORAWAN                   │
 * └─────────────────────────────────────────────────────────────────────────┘
 * 
 * #include "SMART_WI_Libs/LoRaWAN_WioE5.h"
 * #include "SMART_WI_Libs/SerialMon.h"
 * 
 * LoRaWAN_WioE5 lora(&Serial2, &SerialMon);
 * 
 * // Dummy-Sensordaten
 * float temperature = 23.5;
 * float pressure = 1013.25;
 * float deflection = 0.045;
 * 
 * void setup() {
 *     SerialMon.begin(115200);
 *     
 *     if (lora.initializeEverything()) {
 *         SerialMon.println("🎉 LoRaWAN bereit für Sensordaten!");
 *     } else {
 *         SerialMon.println("❌ Initialisierung fehlgeschlagen!");
 *         while (1) delay(1000);
 *     }
 * }
 * 
 * void loop() {
 *     static unsigned long lastSend = 0;
 *     
 *     if (millis() - lastSend >= 30000) {
 *         
 *         // Sensordaten simulieren
 *         temperature += random(-10, 11) / 10.0;
 *         pressure += random(-50, 51) / 100.0;
 *         deflection += random(-5, 6) / 1000.0;
 *         
 *         // KOMPAKTE JSON erstellen (LoRaWAN-optimiert < 200 bytes)
 *         String compactJson = "{\"d\":[";
 *         compactJson += String(temperature, 1);   // 1 Dezimalstelle
 *         compactJson += ",";
 *         compactJson += String(pressure, 0);      // Keine Dezimalstellen
 *         compactJson += ",";
 *         compactJson += String(deflection, 3);    // 3 Dezimalstellen
 *         compactJson += "],\"id\":\"SENSOR_01\",\"t\":";
 *         compactJson += String(millis() / 1000);
 *         compactJson += "}";
 *         
 *         // Größe prüfen (sollte < 200 bytes sein)
 *         SerialMon.println("JSON Größe: " + String(compactJson.length()) + " bytes");
 *         SerialMon.println("Sende: " + compactJson);
 *         
 *         // Senden
 *         if (lora.sendJsonMessage(compactJson.c_str())) {
 *             SerialMon.println("✅ Sensordaten erfolgreich gesendet!");
 *         } else {
 *             SerialMon.println("❌ Sendung fehlgeschlagen!");
 *         }
 *         
 *         lastSend = millis();
 *     }
 *     
 *     delay(100);
 * }
 * 
 * ┌─────────────────────────────────────────────────────────────────────────┐
 * │ BEISPIEL 4: KONFIGURIERBARE ABP-EINSTELLUNGEN                        │
 * └─────────────────────────────────────────────────────────────────────────┘
 * 
 * #include "SMART_WI_Libs/LoRaWAN_WioE5.h"
 * #include "SMART_WI_Libs/SerialMon.h"
 * 
 * LoRaWAN_WioE5 lora(&Serial2, &SerialMon);
 * 
 * void setup() {
 *     SerialMon.begin(115200);
 *     
 *     // UART initialisieren
 *     if (!lora.begin(9600)) {
 *         SerialMon.println("❌ UART-Initialisierung fehlgeschlagen!");
 *         while (1) delay(1000);
 *     }
 *     
 *     // ══════════════════════════════════════════════════════════════════
 *     // INDIVIDUELLE ABP-KONFIGURATION (ANPASSEN AN IHRE WERTE!)
 *     // ══════════════════════════════════════════════════════════════════
 *     
 *     // TODO: Ersetzen Sie diese Werte durch Ihre ChirpStack-Konfiguration!
 *     const char* deviceAddr = "01234567";                     // Ihr Device Address
 *     const char* nwkSKey = "FEDCBA9876543210FEDCBA9876543210";  // Ihr Network Session Key
 *     const char* appSKey = "0123456789ABCDEF0123456789ABCDEF";  // Ihr Application Session Key
 *     
 *     if (!lora.configureABP(deviceAddr, nwkSKey, appSKey)) {
 *         SerialMon.println("❌ ABP-Konfiguration fehlgeschlagen!");
 *         SerialMon.println("Prüfen Sie Ihre Schlüssel!");
 *         while (1) delay(1000);
 *     }
 *     
 *     // Erweiterte Einstellungen
 *     if (!lora.configureAdvanced(LORAWAN_REGION_EU868, LORAWAN_CLASS_A, LORAWAN_DR5, true)) {
 *         SerialMon.println("❌ Erweiterte Konfiguration fehlgeschlagen!");
 *         while (1) delay(1000);
 *     }
 *     
 *     // Netzwerk beitreten
 *     if (!lora.joinNetwork()) {
 *         SerialMon.println("❌ Netzwerk-Join fehlgeschlagen!");
 *         while (1) delay(1000);
 *     }
 *     
 *     SerialMon.println("🎉 LoRaWAN mit individueller ABP-Konfiguration bereit!");
 *     lora.printModuleInfo();
 * }
 * 
 * void loop() {
 *     static unsigned long lastSend = 0;
 *     
 *     if (millis() - lastSend >= 30000) {
 *         String json = "{\"temp\":23.5,\"id\":\"" + String(deviceAddr) + "\"}";
 *         
 *         if (lora.sendJsonMessage(json.c_str())) {
 *             SerialMon.println("✅ Nachricht gesendet!");
 *         }
 *         
 *         lastSend = millis();
 *     }
 *     delay(100);
 * }
 * 
 * ┌─────────────────────────────────────────────────────────────────────────┐
 * │ BEISPIEL 5: ERWEITERTE KONFIGURATION (OPTIONAL)                       │
 * └─────────────────────────────────────────────────────────────────────────┘
 * 
 * #include "SMART_WI_Libs/LoRaWAN_WioE5.h"
 * #include "SMART_WI_Libs/SerialMon.h"
 * 
 * LoRaWAN_WioE5 lora(&Serial2, &SerialMon);
 * 
 * void setup() {
 *     SerialMon.begin(115200);
 *     
 *     // Schritt-für-Schritt Konfiguration
 *     if (!lora.begin(9600)) {
 *         SerialMon.println("❌ UART-Initialisierung fehlgeschlagen!");
 *         while (1) delay(1000);
 *     }
 *     
 *     // Eigene ABP-Konfiguration (ANPASSEN AN IHRE CHIRPSTACK-EINSTELLUNGEN!)
 *     if (!lora.configureABP(
 *         "12345678",                                      // Ihr Device Address
 *         "A1B2C3D4E5F60718293A4B5C6D7E8F90",              // Ihr Network Session Key
 *         "A1B2C3D4E5F60718293A4B5C6D7E8F91"               // Ihr Application Session Key
 *     )) {
 *         SerialMon.println("❌ ABP-Konfiguration fehlgeschlagen!");
 *         while (1) delay(1000);
 *     }
 *     
 *     // Erweiterte Einstellungen
 *     if (!lora.configureAdvanced(LORAWAN_REGION_EU868, LORAWAN_CLASS_A, LORAWAN_DR5, true)) {
 *         SerialMon.println("❌ Erweiterte Konfiguration fehlgeschlagen!");
 *         while (1) delay(1000);
 *     }
 *     
 *     // Netzwerk beitreten
 *     if (!lora.joinNetwork()) {
 *         SerialMon.println("❌ Netzwerk-Join fehlgeschlagen!");
 *         while (1) delay(1000);
 *     }
 *     
 *     SerialMon.println("🎉 LoRaWAN vollständig konfiguriert!");
 *     
 *     // Modul-Informationen ausgeben
 *     lora.printModuleInfo();
 * }
 * 
 * void loop() {
 *     // Dein Code hier...
 * }
 * 
 * ═════════════════════════════════════════════════════════════════════════════
 *                                 HARDWARE SETUP
 * ═════════════════════════════════════════════════════════════════════════════
 * 
 * ┌─────────────────────────────────────────────────────────────────────────┐
 * │ AVR128DB48 ↔ Grove Wio-E5 Verbindung:                                 │
 * └─────────────────────────────────────────────────────────────────────────┘
 * 
 *   AVR128DB48        Grove Wio-E5
 *   ──────────        ────────────
 *   PIN_PF4 (TX) ───→ RX (Pin 2)
 *   PIN_PF5 (RX) ←─── TX (Pin 3)
 *   GND          ──── GND (Pin 4)
 *   3.3V         ──── VCC (Pin 1)
 * 
 * ⚠️  WICHTIG: Grove Wio-E5 arbeitet mit 3.3V!
 * ⚠️  AVR128DB48 UART2 wird für LoRaWAN verwendet
 * ⚠️  Serial Monitor läuft über UART0 (USB)
 * 
 * ═════════════════════════════════════════════════════════════════════════════
 *                               CHIRPSTACK SETUP
 * ═════════════════════════════════════════════════════════════════════════════
 * 
 * ┌─────────────────────────────────────────────────────────────────────────┐
 * │ ABP-Konfiguration per Funktion (EMPFOHLEN):                           │
 * └─────────────────────────────────────────────────────────────────────────┘
 * 
 * // ABP-Konfiguration über Funktion (bitte an Ihre ChirpStack-Einstellungen anpassen)
 * lora.configureABP(
 *     "12345678",                                      // Device Address (4 Bytes Hex)
 *     "A1B2C3D4E5F60718293A4B5C6D7E8F90",              // Network Session Key (16 Bytes Hex)
 *     "A1B2C3D4E5F60718293A4B5C6D7E8F91"               // Application Session Key (16 Bytes Hex)
 * );
 * 
 * // Alternative: Standard-Werte aus Header verwenden
 * lora.configureWithDefaults(LORAWAN_MODE_ABP);
 * 
 * Einstellungen in ChirpStack:
 * - Region: EU868
 * - LoRaWAN Version: 1.0.3
 * - Regional Parameters: RP002-1.0.1
 * - Klasse: A
 * - ABP-Modus aktivieren
 * - Device Address: Ihr individueller Wert
 * - Network Session Key: Ihr individueller Wert  
 * - Application Session Key: Ihr individueller Wert
 * 
 * ═════════════════════════════════════════════════════════════════════════════
 *                            JSON FORMAT GUIDELINES
 * ═════════════════════════════════════════════════════════════════════════════
 * 
 * ┌─────────────────────────────────────────────────────────────────────────┐
 * │ LoRaWAN Payload Limits:                                                │
 * └─────────────────────────────────────────────────────────────────────────┘
 * 
 * EU868 Payload-Limits (abhängig von Data Rate):
 * - DR0 (SF12): Max. 51 bytes
 * - DR1 (SF11): Max. 51 bytes
 * - DR2 (SF10): Max. 51 bytes
 * - DR3 (SF9):  Max. 115 bytes
 * - DR4 (SF8):  Max. 242 bytes
 * - DR5 (SF7):  Max. 242 bytes ← EMPFOHLEN!
 * 
 * ┌─────────────────────────────────────────────────────────────────────────┐
 * │ Kompakte JSON-Beispiele (< 200 bytes):                                │
 * └─────────────────────────────────────────────────────────────────────────┘
 * 
 * OPTIMAL - Array-Format (65 bytes):
 * {"d":[23.5,1013,0.045,24.1],"id":"NODE01","t":1234567}
 * 
 * GUT - Kurze Schlüssel (89 bytes):
 * {"t1":23.5,"t2":24.1,"p":1013,"d":0.045,"id":"NODE01","ts":1234567}
 * 
 * SCHLECHT - Lange Schlüssel (156 bytes):
 * {"temperature1":23.5,"temperature2":24.1,"pressure":1013,"deflection":0.045,"deviceId":"NODE01","timestamp":1234567}
 * 
 * ┌─────────────────────────────────────────────────────────────────────────┐
 * │ Optimierungstipps:                                                     │
 * └─────────────────────────────────────────────────────────────────────────┘
 * 
 * ✅ Kurze Schlüssel verwenden ("d" statt "data")
 * ✅ Arrays für Messwerte
 * ✅ Weniger Dezimalstellen (1-2 für Temperatur, 0 für Druck)
 * ✅ Kurze Device-IDs
 * ✅ Timestamp in Sekunden, nicht Millisekunden
 * ❌ Lange, beschreibende Schlüsselnamen vermeiden
 * ❌ Unnötige Leerzeichen
 * ❌ Zu viele Dezimalstellen
 * 
 * ═════════════════════════════════════════════════════════════════════════════
 *                                TROUBLESHOOTING
 * ═════════════════════════════════════════════════════════════════════════════
 * 
 * ┌─────────────────────────────────────────────────────────────────────────┐
 * │ Häufige Probleme und Lösungen:                                         │
 * └─────────────────────────────────────────────────────────────────────────┘
 * 
 * Problem: "AT-Kommando Timeout"
 * Lösung:  - UART-Verkabelung prüfen
 *          - Baudrate prüfen (Standard: 9600)
 *          - 3.3V Spannungsversorgung prüfen
 * 
 * Problem: "Join fehlgeschlagen"
 * Lösung:  - ChirpStack-Konfiguration prüfen
 *          - ABP-Schlüssel prüfen
 *          - Gateway-Verbindung prüfen
 *          - Antennenverbindung prüfen
 * 
 * Problem: "Payload zu groß"
 * Lösung:  - JSON kompakter machen
 *          - Data Rate erhöhen (DR5 empfohlen)
 *          - Weniger Daten pro Nachricht
 * 
 * Problem: "Modul antwortet nicht"
 * Lösung:  - resetModule() aufrufen
 *          - Power Cycle durchführen
 *          - Firmware-Update prüfen
 * 
 * ┌─────────────────────────────────────────────────────────────────────────┐
 * │ Debug-Funktionen:                                                      │
 * └─────────────────────────────────────────────────────────────────────────┘
 * 
 * lora.printModuleInfo();        // Alle Modul-Informationen
 * lora.printChannelConfig();     // Kanal-Konfiguration
 * lora.testUARTConnection();     // UART-Verbindung testen
 * lora.setDebugEnabled(true);    // Debug-Ausgaben aktivieren
 * 
 * ═════════════════════════════════════════════════════════════════════════════
 *                                  CHANGELOG
 * ═════════════════════════════════════════════════════════════════════════════
 * 
 * v1.0.0 (2025-01-04):
 * ✅ Erste stabile Version
 * ✅ ABP und OTAA Unterstützung
 * ✅ Ultra-einfache API mit nur 2 Funktionen
 * ✅ Kompakte JSON-Optimierung für LoRaWAN
 * ✅ EU868 Frequenz-Unterstützung
 * ✅ Firmware-Update Funktionalität
 * ✅ Umfassendes Debug-System
 * ✅ Vollständige Dokumentation mit Beispielen
 * 
 * @author: Smart Wire Industries
 * @version: 1.0.0
 * @date: 2025-01-04
 * @license: MIT
 * 
 * ═════════════════════════════════════════════════════════════════════════════
 */

#ifndef LORAWAN_WIOE5_H
#define LORAWAN_WIOE5_H

#include "Arduino.h"
#include "HardwareSerial.h"

// ================================================================
// KONSTANTEN UND KONFIGURATION
// ================================================================

// Timing-Parameter
#define LORAWAN_DEFAULT_TIMEOUT_MS 5000       // Standard AT-Kommando Timeout
#define LORAWAN_JOIN_TIMEOUT_MS 45000         // Timeout für LoRaWAN Join
#define LORAWAN_SEND_TIMEOUT_MS 15000         // Timeout für Nachrichtenversand
#define LORAWAN_RESPONSE_BUFFER_SIZE 1024     // Größe des Empfangspuffers

// EU868 Standard-Frequenzen
#define LORAWAN_FREQ_CH0 867.1f  // MHz
#define LORAWAN_FREQ_CH1 867.3f  // MHz
#define LORAWAN_FREQ_CH2 867.5f  // MHz

// ================================================================
// HARDWARE KONFIGURATION
// ================================================================

// UART2 Pin-Definitionen für Wio-E5 Kommunikation (AVR128DB48)
#ifndef WIO_E5_TX_PIN
#define WIO_E5_TX_PIN PIN_PF4        // AVR TX -> Wio-E5 RX
#endif

#ifndef WIO_E5_RX_PIN
#define WIO_E5_RX_PIN PIN_PF5        // AVR RX <- Wio-E5 TX
#endif

#ifndef WIO_E5_BAUDRATE
#define WIO_E5_BAUDRATE 9600         // Standard AT-Firmware Baudrate
#endif

// ================================================================
// STANDARD LORAWAN KONFIGURATION
// ================================================================

// ABP Parameter - Standard-Werte (bitte in ChirpStack entsprechend konfigurieren)
#ifndef LORAWAN_DEVICE_EUI
#define LORAWAN_DEVICE_EUI "7856341200000001"   // Device EUI (8 Bytes Hex)
#endif

#ifndef LORAWAN_DEVICE_ADDR
#define LORAWAN_DEVICE_ADDR "12345678"          // DevAddr (4 Bytes Hex)
#endif

#ifndef LORAWAN_NETWORK_SESSION_KEY
#define LORAWAN_NETWORK_SESSION_KEY "A1B2C3D4E5F60718293A4B5C6D7E8F90"  // NwkSKey
#endif

#ifndef LORAWAN_APP_SESSION_KEY
#define LORAWAN_APP_SESSION_KEY "A1B2C3D4E5F60718293A4B5C6D7E8F91"      // AppSKey
#endif

// OTAA Parameter - Standard-Werte
#ifndef LORAWAN_APPLICATION_EUI
#define LORAWAN_APPLICATION_EUI "FEDCBA0987654321"    // Application EUI (8 Bytes Hex)
#endif

#ifndef LORAWAN_APPLICATION_KEY
#define LORAWAN_APPLICATION_KEY "A1B2C3D4E5F60718293A4B5C6D7E8F90A1B2C3D4"  // Application Key (16 Bytes Hex)
#endif

// Timing-Parameter
#ifndef LORAWAN_SEND_INTERVAL_MS
#define LORAWAN_SEND_INTERVAL_MS 30000       // 30 Sekunden zwischen Nachrichten
#endif

#ifndef LORAWAN_UPDATE_CHECK_TIME_MS
#define LORAWAN_UPDATE_CHECK_TIME_MS 5000    // 5 Sekunden für Firmware-Update Check
#endif

// EU868 Frequenz-Konfiguration (angepasst an Gateway)
#ifndef LORAWAN_FREQUENCY_CH0
#define LORAWAN_FREQUENCY_CH0 867.1f         // MHz
#endif

#ifndef LORAWAN_FREQUENCY_CH1
#define LORAWAN_FREQUENCY_CH1 867.3f         // MHz
#endif

#ifndef LORAWAN_FREQUENCY_CH2
#define LORAWAN_FREQUENCY_CH2 867.5f         // MHz
#endif

// Standard-Konfiguration für LoRaWAN
#ifndef LORAWAN_DEFAULT_REGION
#define LORAWAN_DEFAULT_REGION LORAWAN_REGION_EU868
#endif

#ifndef LORAWAN_DEFAULT_CLASS
#define LORAWAN_DEFAULT_CLASS LORAWAN_CLASS_A
#endif

#ifndef LORAWAN_DEFAULT_DATA_RATE
#define LORAWAN_DEFAULT_DATA_RATE LORAWAN_DR5
#endif

#ifndef LORAWAN_DEFAULT_ADR
#define LORAWAN_DEFAULT_ADR true
#endif

// ================================================================
// ENUMERATIONEN
// ================================================================

enum LoRaWAN_Mode {
    LORAWAN_MODE_OTAA,  // Over-the-Air Activation
    LORAWAN_MODE_ABP    // Activation by Personalization
};

enum LoRaWAN_Class {
    LORAWAN_CLASS_A,    // Bi-directional end-devices
    LORAWAN_CLASS_B,    // Bi-directional end-devices with scheduled receive slots
    LORAWAN_CLASS_C     // Bi-directional end-devices with maximal receive slots
};

enum LoRaWAN_Region {
    LORAWAN_REGION_EU868,   // Europe 868 MHz
    LORAWAN_REGION_US915,   // US 915 MHz
    LORAWAN_REGION_AS923    // Asia 923 MHz
};

enum LoRaWAN_DataRate {
    LORAWAN_DR0 = 0,    // SF12BW125
    LORAWAN_DR1 = 1,    // SF11BW125
    LORAWAN_DR2 = 2,    // SF10BW125
    LORAWAN_DR3 = 3,    // SF9BW125
    LORAWAN_DR4 = 4,    // SF8BW125
    LORAWAN_DR5 = 5,    // SF7BW125
    LORAWAN_DR6 = 6,    // SF7BW250
    LORAWAN_DR7 = 7     // FSK
};

// ================================================================
// STRUKTUREN
// ================================================================

struct LoRaWAN_Config {
    // OTAA Parameter
    char deviceEUI[17];         // 16 Hex-Zeichen + Null-Terminator
    char applicationEUI[17];    // 16 Hex-Zeichen + Null-Terminator
    char applicationKey[33];    // 32 Hex-Zeichen + Null-Terminator
    
    // ABP Parameter
    char deviceAddr[9];         // 8 Hex-Zeichen + Null-Terminator
    char networkSessionKey[33]; // 32 Hex-Zeichen + Null-Terminator
    char appSessionKey[33];     // 32 Hex-Zeichen + Null-Terminator
    
    // Allgemeine Konfiguration
    LoRaWAN_Mode mode;
    LoRaWAN_Class deviceClass;
    LoRaWAN_Region region;
    LoRaWAN_DataRate dataRate;
    bool adaptiveDataRate;
    float customFrequencies[3]; // Benutzerdefinierte Frequenzen
};

struct LoRaWAN_Status {
    bool moduleReady;
    bool networkJoined;
    char firmwareVersion[32];
    char realDeviceEUI[17];
    uint16_t messageCounter;
    unsigned long lastSendTime;
    bool hasDownlink;
    uint8_t downlinkData[256];
    size_t downlinkSize;
    uint8_t downlinkPort;
};

// ================================================================
// HAUPTKLASSE
// ================================================================

class LoRaWAN_WioE5 {
private:
    HardwareSerial* _serial;
    Stream* _debugSerial;
    
    char _responseBuffer[LORAWAN_RESPONSE_BUFFER_SIZE];
    LoRaWAN_Config _config;
    LoRaWAN_Status _status;
    
    // Private Hilfsmethoden
    bool sendATCommand(const char* command, const char* expectedResponse = nullptr, 
                      unsigned long timeout = LORAWAN_DEFAULT_TIMEOUT_MS);
    bool sendATCommandSilent(const char* command, const char* expectedResponse = nullptr, 
                            unsigned long timeout = LORAWAN_DEFAULT_TIMEOUT_MS);
    bool waitForResponse(const char* expectedResponse, unsigned long timeout);
    void clearInputBuffer();
    void debugPrint(const char* message);
    void debugPrintln(const char* message);
    bool parseDownlinkMessage(const char* response);
    
    // Konfigurationsmethoden
    bool setMode(LoRaWAN_Mode mode);
    bool setRegion(LoRaWAN_Region region);
    bool setClass(LoRaWAN_Class deviceClass);
    bool setDataRate(LoRaWAN_DataRate dataRate);
    bool setAdaptiveDataRate(bool enabled);
    bool setCustomFrequencies();
    
    // ABP-spezifische Methoden
    bool setDeviceAddr(const char* deviceAddr);
    bool setNetworkSessionKey(const char* networkSessionKey);
    bool setAppSessionKey(const char* appSessionKey);
    
    // OTAA-spezifische Methoden
    bool setDeviceEUI(const char* deviceEUI);
    bool setApplicationEUI(const char* applicationEUI);
    bool setApplicationKey(const char* applicationKey);

public:
    // ================================================================
    // KONSTRUKTOR UND DESTRUKTOR
    // ================================================================
    
    /**
     * Konstruktor für LoRaWAN WioE5 Library
     * @param serial Pointer zur Hardware-Serial-Schnittstelle
     * @param debugSerial Pointer zur Debug-Serial-Schnittstelle (optional)
     */
    LoRaWAN_WioE5(HardwareSerial* serial, Stream* debugSerial = nullptr);
    
    /**
     * Destruktor
     */
    ~LoRaWAN_WioE5();
    
    // ================================================================
    // INITIALISIERUNG UND KONFIGURATION
    // ================================================================
    
    /**
     * Initialisiert die LoRaWAN-Kommunikation
     * @param baudRate Baudrate für die UART-Kommunikation (Standard: 9600)
     * @return true wenn Initialisierung erfolgreich
     */
    bool begin(unsigned long baudRate = 9600);
    
    /**
     * Setzt die Konfiguration für ABP-Modus
     * @param deviceAddr Device Address (8 Hex-Zeichen)
     * @param networkSessionKey Network Session Key (32 Hex-Zeichen)
     * @param appSessionKey Application Session Key (32 Hex-Zeichen)
     * @return true wenn Konfiguration erfolgreich
     */
    bool configureABP(const char* deviceAddr, const char* networkSessionKey, 
                     const char* appSessionKey);
    
    /**
     * Setzt die Konfiguration für OTAA-Modus
     * @param deviceEUI Device EUI (16 Hex-Zeichen)
     * @param applicationEUI Application EUI (16 Hex-Zeichen)
     * @param applicationKey Application Key (32 Hex-Zeichen)
     * @return true wenn Konfiguration erfolgreich
     */
    bool configureOTAA(const char* deviceEUI, const char* applicationEUI, 
                      const char* applicationKey);
    
    /**
     * Setzt erweiterte Konfiguration
     * @param region LoRaWAN-Region
     * @param deviceClass LoRaWAN-Klasse
     * @param dataRate Data Rate
     * @param adaptiveDataRate Adaptive Data Rate aktivieren
     * @return true wenn Konfiguration erfolgreich
     */
    bool configureAdvanced(LoRaWAN_Region region = LORAWAN_REGION_EU868,
                          LoRaWAN_Class deviceClass = LORAWAN_CLASS_A,
                          LoRaWAN_DataRate dataRate = LORAWAN_DR5,
                          bool adaptiveDataRate = true);
    
    /**
     * Setzt benutzerdefinierte Frequenzen
     * @param ch0 Frequenz für Kanal 0 (MHz)
     * @param ch1 Frequenz für Kanal 1 (MHz)
     * @param ch2 Frequenz für Kanal 2 (MHz)
     * @return true wenn Konfiguration erfolgreich
     */
    bool setCustomFrequencies(float ch0, float ch1, float ch2);
    
    /**
     * Konfiguriert das System mit Standard-Werten aus den Header-Konstanten
     * @param mode LoRaWAN-Modus (ABP oder OTAA)
     * @return true wenn Konfiguration erfolgreich
     */
    bool configureWithDefaults(LoRaWAN_Mode mode = LORAWAN_MODE_ABP);
    
    /**
     * Setzt den Spreading Factor
     * @param sf Spreading Factor (7-12)
     * @return true wenn erfolgreich
     */
    bool setSpreadingFactor(int sf);
    
    // ================================================================
    // NETZWERK-VERBINDUNG
    // ================================================================
    
    /**
     * Verbindet mit dem LoRaWAN-Netzwerk
     * @return true wenn Verbindung erfolgreich
     */
    bool joinNetwork();
    
    /**
     * Prüft ob das Modul mit dem Netzwerk verbunden ist
     * @return true wenn verbunden
     */
    bool isNetworkJoined();
    
    // ================================================================
    // NACHRICHTENVERSAND
    // ================================================================
    
    /**
     * Sendet eine Textnachricht
     * @param message Zu sendende Nachricht
     * @param port LoRaWAN-Port (1-223, Standard: 1)
     * @param confirmed Bestätigte Nachricht (Standard: false)
     * @return true wenn Sendung erfolgreich
     */
    bool sendMessage(const char* message, uint8_t port = 1, bool confirmed = false);
    
    /**
     * Sendet Binärdaten (Hex-String)
     * @param hexData Hex-String der Daten
     * @param port LoRaWAN-Port (1-223, Standard: 1)
     * @param confirmed Bestätigte Nachricht (Standard: false)
     * @return true wenn Sendung erfolgreich
     */
    bool sendHexData(const char* hexData, uint8_t port = 1, bool confirmed = false);
    
    /**
     * Sendet Binärdaten (Byte-Array)
     * @param data Byte-Array der Daten
     * @param length Länge der Daten
     * @param port LoRaWAN-Port (1-223, Standard: 1)
     * @param confirmed Bestätigte Nachricht (Standard: false)
     * @return true wenn Sendung erfolgreich
     */
    bool sendBinaryData(const uint8_t* data, size_t length, uint8_t port = 1, bool confirmed = false);
    
    // ================================================================
    // STATUS UND INFORMATION
    // ================================================================
    
    /**
     * Gibt den aktuellen Status zurück
     * @return Status-Struktur
     */
    LoRaWAN_Status getStatus();
    
    /**
     * Gibt die aktuelle Konfiguration zurück
     * @return Konfigurations-Struktur
     */
    LoRaWAN_Config getConfig();
    
    /**
     * Liest die echte Device EUI des Moduls aus
     * @param buffer Buffer für die EUI (mindestens 17 Zeichen)
     * @return true wenn erfolgreich
     */
    bool getRealDeviceEUI(char* buffer);
    
    /**
     * Liest die Firmware-Version des Moduls aus
     * @param buffer Buffer für die Version (mindestens 32 Zeichen)
     * @return true wenn erfolgreich
     */
    bool getFirmwareVersion(char* buffer);
    
    /**
     * Prüft ob das Modul bereit ist
     * @return true wenn bereit
     */
    bool isModuleReady();
    
    /**
     * Gibt die Anzahl der gesendeten Nachrichten zurück
     * @return Nachrichtenzähler
     */
    uint16_t getMessageCounter();
    
    /**
     * Prüft ob eine Downlink-Nachricht empfangen wurde
     * @return true wenn Downlink verfügbar
     */
    bool hasDownlinkMessage();
    
    /**
     * Gibt die Größe der Downlink-Nachricht zurück
     * @return Größe in Bytes
     */
    size_t getDownlinkSize();
    
    /**
     * Gibt die Downlink-Daten zurück
     * @return Pointer auf Downlink-Daten
     */
    const uint8_t* getDownlinkData();
    
    /**
     * Gibt den Downlink-Port zurück
     * @return Port-Nummer
     */
    uint8_t getDownlinkPort();
    
    /**
     * Löscht die Downlink-Daten
     */
    void clearDownlink();
    
    // ================================================================
    // ERWEITERTE FUNKTIONEN
    // ================================================================
    
    /**
     * Führt einen Modul-Reset durch
     * @return true wenn erfolgreich
     */
    bool resetModule();
    
    /**
     * Aktiviert den DFU-Modus für Firmware-Updates
     * @return true wenn erfolgreich
     */
    bool enterDFUMode();
    
    /**
     * Startet den Firmware-Update-Passthrough-Modus
     * Diese Funktion blockiert bis "EXIT" eingegeben wird
     */
    void firmwareUpdateMode();
    
    /**
     * Sendet ein beliebiges AT-Kommando
     * @param command AT-Kommando
     * @param expectedResponse Erwartete Antwort (optional)
     * @param timeout Timeout in Millisekunden
     * @return true wenn erfolgreich
     */
    bool sendCustomATCommand(const char* command, const char* expectedResponse = nullptr, 
                           unsigned long timeout = LORAWAN_DEFAULT_TIMEOUT_MS);
    
    /**
     * Gibt den letzten Antwortpuffer zurück
     * @return Pointer zum Antwortpuffer
     */
    const char* getLastResponse();
    
    // ================================================================
    // DEBUGGING
    // ================================================================
    
    /**
     * Aktiviert/Deaktiviert Debug-Ausgaben
     * @param enabled Debug-Ausgaben aktivieren
     */
    void setDebugEnabled(bool enabled);
    
    /**
     * Testet die UART-Verbindung
     * @return true wenn Verbindung funktioniert
     */
    bool testUARTConnection();
    
    /**
     * Gibt alle wichtigen Informationen aus
     */
    void printModuleInfo();
    
    /**
     * Gibt die aktuelle Kanal-Konfiguration aus
     */
    void printChannelConfig();
    
    // ================================================================
    // EINFACHE API - NUR ZWEI FUNKTIONEN FÜR MAIN!
    // ================================================================
    
    /**
     * SUPER EINFACHE INITIALISIERUNG - Macht alles automatisch!
     * Initialisiert UART, konfiguriert LoRaWAN mit Standard-Werten und verbindet
     * @return true wenn alles erfolgreich initialisiert
     */
    bool initializeEverything();
    
    /**
     * SUPER EINFACHES SENDEN - Sendet JSON-Nachricht
     * @param jsonMessage JSON-String der gesendet werden soll
     * @return true wenn Nachricht erfolgreich gesendet
     */
    bool sendJsonMessage(const char* jsonMessage);
};

// ================================================================
// GLOBALE EINFACHE API-FUNKTIONEN (OPTIONAL)
// ================================================================

/**
 * Globale Instanz für super einfache Verwendung (optional)
 * Kann mit LoRaWAN_initEverything() und LoRaWAN_sendJson() verwendet werden
 */
extern LoRaWAN_WioE5* g_loraWAN_instance;

/**
 * SUPER EINFACHE GLOBALE INITIALISIERUNG
 * Erstellt automatisch eine globale Instanz und initialisiert alles
 * @return true wenn erfolgreich
 */
bool LoRaWAN_initEverything();

/**
 * SUPER EINFACHES GLOBALES SENDEN
 * Sendet JSON-Nachricht über die globale Instanz
 * @param jsonMessage JSON-String
 * @return true wenn erfolgreich
 */
bool LoRaWAN_sendJson(const char* jsonMessage);

#endif // LORAWAN_WIOE5_H
