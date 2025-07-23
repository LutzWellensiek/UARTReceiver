/**
 * Anleitung: Ein LoRaWAN-Gerät in ChirpStack anlegen (OTAA und ABP)
 * 
 * ---
 * 
 * 1. Vorbereitungen
 * 
 * Bevor du ein Gerät in ChirpStack anlegst, benötigst du:
 * * Zugang zur ChirpStack-Weboberfläche
 * * Den Device EUI des Sensors (16-stellige Hex-Zahl)
 * * Die Authentifizierungsdaten:
 *   * Bei **OTAA**: AppKey
 *   * Bei **ABP**: DevAddr, NwkSKey, AppSKey
 * * Ein Gateway, das mit ChirpStack verbunden ist
 * 
 * ---
 * 
 * 2. Application erstellen
 * 
 * 1. In der ChirpStack-Weboberfläche links auf **Applications** klicken
 * 2. Auf **Create** klicken
 * 3. Felder ausfüllen:
 * 
 *    * **Name**: z. B. `TestApp`
 *    * **Description**: Optional
 * 4. Optional kannst du einen Payload-Decoder unter **Payload Codec** hinzufügen
 * 5. Auf **Create Application** klicken
 * 
 * ---
 * 
 * 3. Device Profile erstellen
 * 
 * 1. Links auf **Device Profiles** klicken
 * 2. Auf **Create** klicken
 * 3. Felder ausfüllen:
 * 
 *    * **Name**: z. B. `Generic-EU868`
 *    * **LoRaWAN MAC Version**: 1.0.3 (oder passend zu deinem Gerät)
 *    * **Regional Parameters**: EU868
 *    * **Max EIRP**: 14 dBm
 *    * **Device Supports Class B / C**: Nur anhaken, wenn dein Gerät es unterstützt
 * 4. Optional: Payload-Decoder als JavaScript-Funktion eintragen
 * 5. Auf **Create Device Profile** klicken
 * 
 * ---
 * 
 * 4. Gerät anlegen
 * 
 * 1. Gehe in der Weboberfläche auf **Applications**
 * 2. Wähle deine erstellte Application (z. B. `TestApp`)
 * 3. Klicke auf den Reiter **Devices**
 * 4. Klicke auf **+ Create**
 * 5. Felder ausfüllen:
 * 
 *    * **Name**: z. B. `sensor01`
 *    * **Device EUI**: Dein Device EUI (16-stellig, Hex)
 *    * **Device Profile**: Wähle das zuvor erstellte Profil aus
 * 6. Auf **Create Device** klicken
 * 
 * ---
 * 
 * 5. Aktivierungsmethode auswählen
 * 
 * 5.1 OTAA (Over-the-Air Activation)
 * 
 * 1. Gehe auf den Reiter **OTAA keys**
 * 2. Trage den **AppKey** ein (32 Hex-Zeichen)
 * 3. Klicke auf **Set Device Keys**
 * 
 * Gerät einschalten:
 * 
 * * Das Gerät sendet einen Join Request
 * * Überprüfe den Reiter **Live LoRaWAN Frames**, ob der Join akzeptiert wurde
 * 
 * Wenn das Gerät Strom verliert, sollte es:
 * 
 * * Einen neuen DevNonce generieren (am besten persistent verwalten)
 * * Bei Problemen: Unter **Activation** den Button **Deactivate Device** klicken, um den alten DevNonce-Verlauf zu löschen
 * 
 * ---
 * 
 * 5.2 ABP (Activation by Personalization)
 * 
 * 1. Gehe auf den Reiter **Activation**
 * 2. Trage ein:
 * 
 *    * **Device Address (DevAddr)**
 *    * **Network Session Key (NwkSKey)**
 *    * **Application Session Key (AppSKey)**
 * 3. Optional: Frame-Counter-Validierung ausschalten
 * 
 *    * Gehe zu **Configuration**
 *    * Setze den Haken bei **Disable frame-counter validation**, wenn dein Gerät den Frame-Counter nicht speichert
 * 
 * Gerät einschalten:
 * 
 * * Das Gerät sendet sofort Daten (kein Join erforderlich)
 * * Überprüfe den Reiter **Live LoRaWAN Frames**
 * 
 * ---
 * 
 * 6. Daten weiterverarbeiten
 * 
 * 1. Gehe auf die **Application**
 * 2. Klicke auf den Reiter **Integration**
 * 3. Richte eine Integration ein (z. B. MQTT, HTTP Webhook, InfluxDB)
 * 
 * ---
 * 
 * 7. Zusammenfassung Checkliste
 * 
 * | Schritt | Beschreibung                                |
 * | ------- | ------------------------------------------- |
 * | 1       | Application erstellen                       |
 * | 2       | Device Profile erstellen                    |
 * | 3       | Device anlegen                              |
 * | 4       | OTAA: AppKey eintragen und Join durchführen |
 * | 5       | ABP: DevAddr, NwkSKey, AppSKey eintragen    |
 * | 6       | Live LoRaWAN Frames überwachen              |
 * | 7       | Integration konfigurieren                   |
 * 
 * ---
 * 
 * Fertig! Dein Gerät ist nun im ChirpStack registriert und bereit zum Einsatz.
 */

/**
 * LoRaWAN-Konfiguration für OTAA (Over-The-Air Activation)
 * 
 * Diese Datei enthält alle Parameter, die du für dein LoRaWAN-Gateway/Network-Server benötigst.
 * WICHTIG: Ersetze die Beispielwerte durch die echten Werte von deinem LoRaWAN-Provider!
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <RadioLib.h>

// =============================================================================
// LORAWAN BAND KONFIGURATION
// =============================================================================
// Für Europa verwende EU868, für andere Regionen entsprechend anpassen
extern const LoRaWANBand_t* band;

// =============================================================================
// OTAA PARAMETER - DIESE MÜSSEN ANGEPASST WERDEN!
// =============================================================================
// WICHTIG: Diese Werte erhältst du von deinem LoRaWAN-Provider (TTN, ChirpStack, etc.)

// JOIN EUI (früher APP EUI genannt) - 8 Bytes
extern const char joinEUI_str[];

// DEVICE EUI - 8 Bytes - Eindeutige Geräte-ID
extern const char devEUI_str[];

// NETWORK KEY (NwkKey) - 16 Bytes
extern const char nwkKey_str[];

// APPLICATION KEY (AppKey) - 16 Bytes
extern const char appKey_str[];

// Konvertierte EUIs für RadioLib
extern const uint64_t joinEUI;
extern const uint64_t devEUI;

// Konvertierte Keys für RadioLib
extern uint8_t nwkKey[16];
extern uint8_t appKey[16];

// =============================================================================
// HARDWARE-KONFIGURATION
// =============================================================================
// Pin-Definitionen für SX1262 (bereits getestet und funktionsfähig)
#define NSS_PIN    PIN_PD2   // CS/NSS Pin
#define RESET_PIN  PIN_PE1   // Reset Pin
#define BUSY_PIN   PIN_PD0   // Busy Pin
#define DIO1_PIN   PIN_PB5   // DIO1 Pin

// SX1262 Modul-Instanz
extern SX1262 radio;

// LoRaWAN Node-Instanz
extern LoRaWANNode node;

// =============================================================================
// ANWENDUNGS-KONFIGURATION
// =============================================================================
// Uplink-Intervall in Sekunden (beachte Fair Access Policy!)
extern const uint32_t uplinkIntervalSeconds;

// Uplink-Port (1-223, Port 0 ist reserviert)
extern const uint8_t uplinkPort;

// =============================================================================
// SENSOR-KONFIGURATION
// =============================================================================
// Sensor-Aktivierung (true = aktiviert, false = deaktiviert)
extern const bool TEMP_SENSOR_AKTIV;
extern const bool DEFLECTION_SENSOR_AKTIV;
extern const bool PRESSURE_SENSOR_AKTIV;
extern const bool SONSTIGES_SENSOR_AKTIV;

// Anzahl der Temperatur-Werte
extern const int anzahl_temp_values;

// =============================================================================
// FUNKTIONSDEKLARATIONEN
// =============================================================================

// Hilfsfunktion zum Konvertieren von Hex-String zu uint64_t
uint64_t hexStringToUint64(const char* hexStr);

// Hilfsfunktion zum Konvertieren von Hex-String zu Byte-Array
void hexStringToByteArray(const char* hexStr, uint8_t* byteArray, size_t arraySize);

// Initialisierung der Keys (wird in setup() aufgerufen)
void initializeKeys();

// Debug-Funktion für Fehlermeldungen
void debug(bool isFail, const __FlashStringHelper* message, int state, bool halt);

// =============================================================================
// PARAMETER FÜR GATEWAY/NETWORK-SERVER KONFIGURATION
// =============================================================================
/*
  WICHTIGE INFORMATIONEN FÜR DEIN GATEWAY/NETWORK-SERVER:

  1. GERÄTEKONFIGURATION:
     - Device EUI: [devEUI aus dieser Datei]
     - Join EUI: [joinEUI aus dieser Datei]
     - App Key: [appKey aus dieser Datei]
     - NwkKey: [nwkKey aus dieser Datei] (nur für LoRaWAN 1.1+)

  2. LORAWAN-PARAMETER:
     - Activation Mode: OTAA (Over-The-Air Activation)
     - LoRaWAN Version: 1.0.4 oder 1.1 (abhängig von deinem Network-Server)
     - Regional Parameters: EU868
     - Device Class: A

  3. NETZWERK-EINSTELLUNGEN:
     - Uplink Port: 1
     - Downlink Port: beliebig (wird vom Network-Server gesetzt)
     - ADR (Adaptive Data Rate): Aktiviert
     - Confirmed/Unconfirmed Uplinks: Unconfirmed (Standard)

  4. FAIR ACCESS POLICY:
     - Uplink-Intervall: 30 Sekunden (kann angepasst werden)
     - Duty Cycle: Automatisch von RadioLib verwaltet
     - Maximale Payload: 51 Bytes für EU868 (DR0-DR5)

  5. TYPISCHE NETWORK-SERVER:
     - The Things Network (TTN): https://www.thethingsnetwork.org/
     - ChirpStack: https://www.chirpstack.io/
     - Helium: https://www.helium.com/
     - Loriot: https://www.loriot.io/

  BEISPIEL FÜR TTN (The Things Network):
  1. Gehe zu https://console.thethingsnetwork.org/
  2. Erstelle eine neue Anwendung
  3. Füge ein neues End-Device hinzu
  4. Wähle "Manually" -> "Over the air activation (OTAA)"
  5. Gib die Werte aus dieser Datei ein
  6. Aktiviere "Resets DevNonces" (wichtig für Development!)
*/

#endif // CONFIG_H
