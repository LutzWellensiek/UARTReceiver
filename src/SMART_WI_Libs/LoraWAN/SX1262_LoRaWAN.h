/**
 * ═════════════════════════════════════════════════════════════════════════════
 *                      SX1262 LoRaWAN Library - README & EXAMPLES
 * ═════════════════════════════════════════════════════════════════════════════
 *
 * ULTRA-EINFACHE LoRaWAN-LIBRARY für SX1262 Module!
 *
 * Entwickelt für AVR128DB48 Mikrocontroller mit folgenden Features:
 * ✅ ABP (Activation by Personalization) und OTAA (Over-the-Air Activation)
 * ✅ Unterstützung für EU868 Frequenz
 * ✅ Modulare Payload-Erstellung mit Payload_Builder
 * ✅ Separate Konfigurationsdatei für jeden Endknoten
 * ✅ Automatische Initialisierung mit config.h
 * ✅ Debug-Ausgaben und Monitoring
 *
 * ═════════════════════════════════════════════════════════════════════════════
 *                                QUICK START EXAMPLES
 * ═════════════════════════════════════════════════════════════════════════════
 *
 * ┌─────────────────────────────────────────────────────────────────────────┐
 * │ BEISPIEL 1: SAUBERE ARCHITEKTUR - PAYLOAD BUILDER + SENDER             │
 * └─────────────────────────────────────────────────────────────────────────┘
 *
 * #include "SMART_WI_Libs/SX1262_LoRaWAN.h"
 * #include "SMART_WI_Libs/Payload_Builder.h"
 * #include "SMART_WI_Libs/lorawanconfig.h"
 *
 * SX1262_LoRaWAN lora;
 * Payload_Builder payloadBuilder;
 *
 * void setup() {
 *     Serial.begin(115200);
 *     
 *     // ══════════════════════════════════════════════════════════════════
 *     // SUPER EINFACHE INITIALISIERUNG - Macht alles automatisch!
 *     // ══════════════════════════════════════════════════════════════════
 *     
 *     if (lora.initializeSetup()) {
 *         Serial.println("🎉 LoRaWAN bereit!");
 *     } else {
 *         Serial.println("❌ Initialisierung fehlgeschlagen!");
 *         while (1) delay(1000);
 *     }
 * }
 *
 * void loop() {
 *     static unsigned long lastSend = 0;
 *
 *     if (millis() - lastSend >= 30000) {  // Alle 30 Sekunden
 *
 *         // ══════════════════════════════════════════════════════════════
 *         // 1. PAYLOAD ERSTELLEN
 *         // ══════════════════════════════════════════════════════════════
 *         
 *         payloadBuilder.reset();
 *         payloadBuilder.enableTemperatureSensors(2);
 *         payloadBuilder.enableDeflectionSensors(1);
 *         payloadBuilder.enablePressureSensors(1);
 *         
 *         payloadBuilder.addTemperature(23.5);
 *         payloadBuilder.addTemperature(24.1);
 *         payloadBuilder.addDeflection(0.045);
 *         payloadBuilder.addPressure(1013.25);
 *         
 *         uint8_t payload[256];
 *         size_t payloadSize = payloadBuilder.build(payload, sizeof(payload));
 *         
 *         // ══════════════════════════════════════════════════════════════
 *         // 2. PAYLOAD SENDEN
 *         // ══════════════════════════════════════════════════════════════
 *         
 *         if (lora.sendPayload(payload, payloadSize)) {
 *             Serial.println("✅ Payload erfolgreich gesendet!");
 *         }
 *
 *         lastSend = millis();
 *     }
 *
 *     delay(100);
 * }
 *
 * ┌─────────────────────────────────────────────────────────────────────────┐
 * │ BEISPIEL 2: ERWEITERTE KONFIGURATION                                  │
 * └─────────────────────────────────────────────────────────────────────────┘
 *
 * #include "SMART_WI_Libs/SX1262_LoRaWAN.h"
 * #include "SMART_WI_Libs/lorawanconfig.h"
 *
 * SX1262_LoRaWAN lora(&Serial);
 *
 * void setup() {
 *     Serial.begin(115200);
 *     
 *     // Schritt-für-Schritt Konfiguration
 *     if (!lora.begin()) {
 *         Serial.println("❌ Radio-Initialisierung fehlgeschlagen!");
 *         while (1) delay(1000);
 *     }
 *     
 *     // LoRaWAN mit Werten aus config.h konfigurieren
 *     if (!lora.configureFromConfig()) {
 *         Serial.println("❌ LoRaWAN-Konfiguration fehlgeschlagen!");
 *         while (1) delay(1000);
 *     }
 *     
 *     // Payload Builder konfigurieren
 *     lora.configurePayloadBuilder();
 *     
 *     // Netzwerk beitreten
 *     if (!lora.joinNetwork()) {
 *         Serial.println("❌ Netzwerk-Join fehlgeschlagen!");
 *         while (1) delay(1000);
 *     }
 *     
 *     Serial.println("🎉 LoRaWAN vollständig konfiguriert!");
 *     lora.printModuleInfo();
 * }
 *
 * void loop() {
 *     // Dein Code hier...
 * }
 *
 * ═════════════════════════════════════════════════════════════════════════════
 *                                HARDWARE SETUP
 * ═════════════════════════════════════════════════════════════════════════════
 *
 * ┌─────────────────────────────────────────────────────────────────────────┐
 * │ AVR128DB48 ↔ SX1262 Verbindung:                                       │
 * └─────────────────────────────────────────────────────────────────────────┘
 *
 *   AVR128DB48        SX1262
 *   ──────────        ──────
 *   PIN_PD2   ────→   NSS (CS)
 *   PIN_PE1   ────→   RESET
 *   PIN_PD0   ←────   BUSY
 *   PIN_PB5   ←────   DIO1
 *   SPI Pins  ←────→  SPI (MOSI, MISO, SCK)
 *   GND       ────    GND
 *   3.3V      ────    VCC
 *
 * ⚠️  WICHTIG: SX1262 arbeitet mit 3.3V!
 * ⚠️  Stelle sicher, dass alle Pin-Definitionen in config.h korrekt sind
 *
 * ═════════════════════════════════════════════════════════════════════════════
 *                               CHIRPSTACK SETUP
 * ═════════════════════════════════════════════════════════════════════════════
 *
 * **Anleitung: Ein LoRaWAN-Gerät in ChirpStack anlegen (OTAA und ABP)**
 *
 * ┌─────────────────────────────────────────────────────────────────────────┐
 * │ 1. Vorbereitungen                                                      │
 * └─────────────────────────────────────────────────────────────────────────┘
 *
 * Bevor du ein Gerät in ChirpStack anlegst, benötigst du:
 * • Zugang zur ChirpStack-Weboberfläche
 * • Den Device EUI des Sensors (16-stellige Hex-Zahl)
 * • Die Authentifizierungsdaten:
 *   - Bei OTAA: AppKey
 *   - Bei ABP: DevAddr, NwkSKey, AppSKey
 * • Ein Gateway, das mit ChirpStack verbunden ist
 *
 * ┌─────────────────────────────────────────────────────────────────────────┐
 * │ 2. Application erstellen                                               │
 * └─────────────────────────────────────────────────────────────────────────┘
 *
 * 1. In der ChirpStack-Weboberfläche links auf "Applications" klicken
 * 2. Auf "Create" klicken
 * 3. Felder ausfüllen:
 *    • Name: z. B. "SX1262_TestApp"
 *    • Description: Optional
 * 4. Optional: Payload-Decoder unter "Payload Codec" hinzufügen
 * 5. Auf "Create Application" klicken
 *
 * ┌─────────────────────────────────────────────────────────────────────────┐
 * │ 3. Device Profile erstellen                                            │
 * └─────────────────────────────────────────────────────────────────────────┘
 *
 * 1. Links auf "Device Profiles" klicken
 * 2. Auf "Create" klicken
 * 3. Felder ausfüllen:
 *    • Name: z. B. "SX1262-EU868"
 *    • LoRaWAN MAC Version: 1.0.3 (oder passend zu deinem Gerät)
 *    • Regional Parameters: EU868
 *    • Max EIRP: 14 dBm
 *    • Device Supports Class B / C: Nur anhaken, wenn dein Gerät es unterstützt
 * 4. Optional: Payload-Decoder als JavaScript-Funktion eintragen
 * 5. Auf "Create Device Profile" klicken
 *
 * ┌─────────────────────────────────────────────────────────────────────────┐
 * │ 4. Gerät anlegen                                                       │
 * └─────────────────────────────────────────────────────────────────────────┘
 *
 * 1. Gehe in der Weboberfläche auf "Applications"
 * 2. Wähle deine erstellte Application (z. B. "SX1262_TestApp")
 * 3. Klicke auf den Reiter "Devices"
 * 4. Klicke auf "+ Create"
 * 5. Felder ausfüllen:
 *    • Name: z. B. "sx1262_node_001"
 *    • Device EUI: Dein Device EUI (16-stellig, Hex) aus config.h
 *    • Device Profile: Wähle das zuvor erstellte Profil aus
 * 6. Auf "Create Device" klicken
 *
 * ┌─────────────────────────────────────────────────────────────────────────┐
 * │ 5. Aktivierungsmethode auswählen                                       │
 * └─────────────────────────────────────────────────────────────────────────┘
 *
 * ══════════════════════════════════════════════════════════════════════════
 * 5.1 OTAA (Over-the-Air Activation) - EMPFOHLEN!
 * ══════════════════════════════════════════════════════════════════════════
 *
 * 1. Gehe auf den Reiter "OTAA keys"
 * 2. Trage den AppKey ein (32 Hex-Zeichen aus config.h)
 * 3. Klicke auf "Set Device Keys"
 *
 * Gerät einschalten:
 * • Das Gerät sendet einen Join Request
 * • Überprüfe den Reiter "Live LoRaWAN Frames", ob der Join akzeptiert wurde
 *
 * Wenn das Gerät Strom verliert:
 * • Es sollte einen neuen DevNonce generieren (RadioLib macht das automatisch)
 * • Bei Problemen: Unter "Activation" den Button "Deactivate Device" klicken
 *
 * ══════════════════════════════════════════════════════════════════════════
 * 5.2 ABP (Activation by Personalization)
 * ══════════════════════════════════════════════════════════════════════════
 *
 * 1. Gehe auf den Reiter "Activation"
 * 2. Trage ein:
 *    • Device Address (DevAddr)
 *    • Network Session Key (NwkSKey)
 *    • Application Session Key (AppSKey)
 * 3. Optional: Frame-Counter-Validierung ausschalten
 *    • Gehe zu "Configuration"
 *    • Setze den Haken bei "Disable frame-counter validation"
 *
 * Gerät einschalten:
 * • Das Gerät sendet sofort Daten (kein Join erforderlich)
 * • Überprüfe den Reiter "Live LoRaWAN Frames"
 *
 * ┌─────────────────────────────────────────────────────────────────────────┐
 * │ 6. Payload-Decoder für SX1262 Library (JavaScript)                    │
 * └─────────────────────────────────────────────────────────────────────────┘
 *
 * Füge diesen JavaScript-Decoder in ChirpStack ein:
 *
 * function Decode(fPort, bytes) {
 *     var result = {};
 *     var offset = 0;
 *     
 *     // Temperatur-Daten (Type 'T' = 0x54)
 *     if (bytes.length > offset && bytes[offset] === 0x54) {
 *         offset++;
 *         result.temperatures = [];
 *         while (offset + 4 <= bytes.length && 
 *                bytes[offset] !== 0x44 && bytes[offset] !== 0x50 && bytes[offset] !== 0x53) {
 *             var temp = new Float32Array(new Uint8Array(bytes.slice(offset, offset + 4)).buffer)[0];
 *             result.temperatures.push(parseFloat(temp.toFixed(2)));
 *             offset += 4;
 *         }
 *     }
 *     
 *     // Deflection-Daten (Type 'D' = 0x44)
 *     if (bytes.length > offset && bytes[offset] === 0x44) {
 *         offset++;
 *         result.deflections = [];
 *         while (offset + 4 <= bytes.length && 
 *                bytes[offset] !== 0x54 && bytes[offset] !== 0x50 && bytes[offset] !== 0x53) {
 *             var defl = new Float32Array(new Uint8Array(bytes.slice(offset, offset + 4)).buffer)[0];
 *             result.deflections.push(parseFloat(defl.toFixed(4)));
 *             offset += 4;
 *         }
 *     }
 *     
 *     // Pressure-Daten (Type 'P' = 0x50)
 *     if (bytes.length > offset && bytes[offset] === 0x50) {
 *         offset++;
 *         result.pressures = [];
 *         while (offset + 4 <= bytes.length && 
 *                bytes[offset] !== 0x54 && bytes[offset] !== 0x44 && bytes[offset] !== 0x53) {
 *             var press = new Float32Array(new Uint8Array(bytes.slice(offset, offset + 4)).buffer)[0];
 *             result.pressures.push(parseFloat(press.toFixed(1)));
 *             offset += 4;
 *         }
 *     }
 *     
 *     // Misc-Daten (Type 'S' = 0x53)
 *     if (bytes.length > offset && bytes[offset] === 0x53) {
 *         offset++;
 *         result.misc = [];
 *         while (offset + 4 <= bytes.length && 
 *                bytes[offset] !== 0x54 && bytes[offset] !== 0x44 && bytes[offset] !== 0x50) {
 *             var misc = new Float32Array(new Uint8Array(bytes.slice(offset, offset + 4)).buffer)[0];
 *             result.misc.push(parseFloat(misc.toFixed(2)));
 *             offset += 4;
 *         }
 *     }
 *     
 *     return result;
 * }
 *
 * ┌─────────────────────────────────────────────────────────────────────────┐
 * │ 7. Zusammenfassung Checkliste                                          │
 * └─────────────────────────────────────────────────────────────────────────┘
 *
 * | Schritt | Beschreibung                                |
 * |---------|---------------------------------------------|
 * | 1       | Application erstellen                       |
 * | 2       | Device Profile erstellen                    |
 * | 3       | Device anlegen                              |
 * | 4       | OTAA: AppKey eintragen und Join durchführen |
 * | 5       | ABP: DevAddr, NwkSKey, AppSKey eintragen    |
 * | 6       | Payload-Decoder JavaScript eintragen        |
 * | 7       | Live LoRaWAN Frames überwachen              |
 * | 8       | Integration konfigurieren                   |
 *
 * ═════════════════════════════════════════════════════════════════════════════
 *                                TROUBLESHOOTING
 * ═════════════════════════════════════════════════════════════════════════════
 *
 * ┌─────────────────────────────────────────────────────────────────────────┐
 * │ Häufige Probleme und Lösungen:                                         │
 * └─────────────────────────────────────────────────────────────────────────┘
 *
 * Problem: "Radio initialization failed"
 * Lösung:  - SPI-Verkabelung prüfen
 *          - Pin-Definitionen in config.h prüfen
 *          - 3.3V Spannungsversorgung prüfen
 *
 * Problem: "Join fehlgeschlagen"
 * Lösung:  - ChirpStack-Konfiguration prüfen
 *          - Keys in config.h prüfen
 *          - Gateway-Verbindung prüfen
 *          - Antennenverbindung prüfen
 *
 * Problem: "Payload zu groß"
 * Lösung:  - Payload Builder kompakter konfigurieren
 *          - Ultra-Compact-Modus aktivieren
 *          - Weniger Sensoren pro Übertragung
 *
 * Problem: "Keine Daten in ChirpStack"
 * Lösung:  - Live LoRaWAN Frames prüfen
 *          - Payload-Decoder testen
 *          - Frame-Counter-Validierung deaktivieren
 *
 * @author: Smart Wire Industries
 * @version: 1.0.0
 * @date: 2025-01-17
 * @license: MIT
 */

#ifndef SX1262_LORAWAN_H
#define SX1262_LORAWAN_H

#include "Arduino.h"
#include "Payload_Builder.h"
#include "lorawanconfig.h"
#include <RadioLib.h>

// ================================================================
// HAUPTKLASSE
// ================================================================

class SX1262_LoRaWAN {
private:
    // Private Hilfsmethoden
    void configureLoRaWAN();
    size_t getMaxPayloadSize();
    
public:
    // ================================================================
    // KONSTRUKTOR UND DESTRUKTOR
    // ================================================================
    
    /**
     * Konstruktor für SX1262 LoRaWAN
     */
    SX1262_LoRaWAN();
    
    /**
     * Destruktor
     */
    ~SX1262_LoRaWAN();

    // ================================================================
    // INITIALISIERUNG UND KONFIGURATION
    // ================================================================
    
    /**
     * Initialisiert das SX1262 Radio
     * @return true wenn erfolgreich
     */
    bool initializeRadio();
    
    /**
     * Initialisiert LoRaWAN Node mit OTAA
     * @return true wenn erfolgreich
     */
    bool initializeLoRaWAN();
    
    /**
     * Führt OTAA Join durch
     * @return true wenn erfolgreich
     */
    bool joinNetwork();
    
    /**
     * Komplette Initialisierung: Radio + LoRaWAN + Join
     * @return true wenn alles erfolgreich
     */
    bool initializeEverything();
    
    // ================================================================
    // PAYLOAD FUNKTIONEN
    // ================================================================
    
    /**
     * Erstellt modulares Payload basierend auf Sensor-Konfiguration
     * @param temp1 Temperatur 1
     * @param temp2 Temperatur 2
     * @param deflection Deflection Wert
     * @param pressure Druck Wert
     * @param picTemp PIC Temperatur
     * @param payload Buffer für das Payload
     * @param maxSize Maximale Payload-Größe
     * @return Tatsächliche Payload-Größe
     */
    int buildModularPayload(float temp1, float temp2, float deflection, float pressure, float picTemp, uint8_t* payload, int maxSize);
    
    /**
     * Dekodiert und zeigt Payload-Inhalt an
     * @param payload Payload Buffer
     * @param payloadSize Payload Größe
     */
    void displayPayloadContent(uint8_t* payload, int payloadSize);
    
    // ================================================================
    // LORAWAN KOMMUNIKATION
    // ================================================================
    
    /**
     * Initialisiert das Setup mit allen notwendigen Schritten
     * @return true, wenn erfolgreich initialisiert
     */
    bool initializeSetup();
    
    /**
     * Sendet ein fertiges Payload über LoRaWAN
     * @param payload Pointer auf das Payload-Array
     * @param payloadSize Größe des Payloads in Bytes
     * @return true, wenn erfolgreich gesendet
     */
    bool sendPayload(const uint8_t* payload, size_t payloadSize);
    
    
    // ================================================================
    // DEBUG UND HILFSFUNKTIONEN
    // ================================================================
    
    /**
     * Zeigt Debug-Informationen der verwendeten Keys an
     */
    void printDebugInfo();
    
    /**
     * Zeigt Pin-Konfiguration an
     */
    void printPinConfiguration();
};

#endif // SX1262_LORAWAN_H

