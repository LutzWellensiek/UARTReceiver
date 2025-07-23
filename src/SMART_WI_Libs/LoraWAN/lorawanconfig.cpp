/**
 * LoRaWAN-Konfiguration Implementierung
 * 
 * Diese Datei enthält die Implementierung der Konfigurationsfunktionen
 * und Variablen für die LoRaWAN-Verbindung.
 */

#include "lorawanconfig.h"

// =============================================================================
// LORAWAN BAND KONFIGURATION
// =============================================================================
// Für Europa verwende EU868, für andere Regionen entsprechend anpassen
const LoRaWANBand_t* band = &EU868;

// =============================================================================
// OTAA PARAMETER - DIESE MÜSSEN ANGEPASST WERDEN!
// =============================================================================
// WICHTIG: Diese Werte erhältst du von deinem LoRaWAN-Provider (TTN, ChirpStack, etc.)

// JOIN EUI (früher APP EUI genannt) - 8 Bytes
// OPTION 1: Als Hex-String (wie von ChirpStack generiert)
const char joinEUI_str[] = "57fa6af7e943453d";  // Ersetze durch deine Join EUI

// DEVICE EUI - 8 Bytes - Eindeutige Geräte-ID
// OPTION 1: Als Hex-String (wie von ChirpStack generiert)
const char devEUI_str[] = "c95ca89cc36cee56";  // Deine Device EUI

// NETWORK KEY (NwkKey) - 16 Bytes
// OPTION 1: Als Hex-String (wie von ChirpStack generiert)
const char nwkKey_str[] = "ee428117008c670dc1559c86ead819f0";  // Ersetze durch deinen NwkKey

// APPLICATION KEY (AppKey) - 16 Bytes
// OPTION 1: Als Hex-String (wie von ChirpStack generiert)
const char appKey_str[] = "ee428117008c670dc1559c86ead819f0";  // Ersetze durch deinen AppKey

// Konvertierte EUIs für RadioLib
const uint64_t joinEUI = hexStringToUint64(joinEUI_str);
const uint64_t devEUI = hexStringToUint64(devEUI_str);

// Konvertierte Keys für RadioLib
uint8_t nwkKey[16];
uint8_t appKey[16];

// =============================================================================
// HARDWARE-KONFIGURATION
// =============================================================================
// SX1262 Modul-Instanz
SX1262 radio = new Module(NSS_PIN, DIO1_PIN, RESET_PIN, BUSY_PIN);

// LoRaWAN Node-Instanz
LoRaWANNode node(&radio, band);

// =============================================================================
// ANWENDUNGS-KONFIGURATION
// =============================================================================
// Uplink-Intervall in Sekunden (beachte Fair Access Policy!)
const uint32_t uplinkIntervalSeconds = 30;

// Uplink-Port (1-223, Port 0 ist reserviert)
const uint8_t uplinkPort = 1;

// =============================================================================
// SENSOR-KONFIGURATION
// =============================================================================
// Sensor-Aktivierung (true = aktiviert, false = deaktiviert)
const bool TEMP_SENSOR_AKTIV = true;
const bool DEFLECTION_SENSOR_AKTIV = true;
const bool PRESSURE_SENSOR_AKTIV = true;
const bool SONSTIGES_SENSOR_AKTIV = true;

// Anzahl der Temperatur-Werte
const int anzahl_temp_values = 2;

// =============================================================================
// FUNKTIONEN
// =============================================================================

// Hilfsfunktion zum Konvertieren von Hex-String zu uint64_t
uint64_t hexStringToUint64(const char* hexStr) {
    uint64_t result = 0;
    for (int i = 0; i < 16 && hexStr[i] != '\0'; i++) {
        result <<= 4;
        char c = hexStr[i];
        if (c >= '0' && c <= '9') {
            result += c - '0';
        } else if (c >= 'A' && c <= 'F') {
            result += c - 'A' + 10;
        } else if (c >= 'a' && c <= 'f') {
            result += c - 'a' + 10;
        }
    }
    return result;
}

// Hilfsfunktion zum Konvertieren von Hex-String zu Byte-Array
void hexStringToByteArray(const char* hexStr, uint8_t* byteArray, size_t arraySize) {
    for (size_t i = 0; i < arraySize && i * 2 < strlen(hexStr); i++) {
        char hexByte[3] = {hexStr[i * 2], hexStr[i * 2 + 1], '\0'};
        byteArray[i] = (uint8_t)strtol(hexByte, NULL, 16);
    }
}

// Initialisierung der Keys (wird in setup() aufgerufen)
void initializeKeys() {
    hexStringToByteArray(nwkKey_str, nwkKey, 16);
    hexStringToByteArray(appKey_str, appKey, 16);
}

// Debug-Funktion für Fehlermeldungen
void debug(bool isFail, const __FlashStringHelper* message, int state, bool halt) {
  if (isFail) {
    Serial.print("Error: ");
    Serial.print(message);
    Serial.print(" (");
    Serial.print(state);
    Serial.println(")");
    if (halt) {
      while(true) delay(1000);
    }
  }
}
