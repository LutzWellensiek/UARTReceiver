# UARTReceiver Library - Binärdaten-Modus

## Überblick

Die UARTReceiver Library wurde um einen Binärdaten-Modus erweitert, der speziell für den Empfang von strukturierten Binärdaten entwickelt wurde. Diese Funktionalität ist ideal für Sensordaten-Übertragungen, bei denen Effizienz und Zuverlässigkeit wichtig sind.

## Funktionen

### Binärdaten-Verarbeitung
- **Pufferbasierte Verarbeitung**: Sammelt Binärdaten in einem internen Puffer
- **Payload-Größenerkennung**: Automatische Erkennung vollständiger Pakete
- **Timeout-Handling**: Verwirft unvollständige Pakete nach konfigurierbarem Timeout
- **Hex-Debug-Ausgabe**: Detaillierte Darstellung der empfangenen Rohdaten

### Sensordaten-Dekodierung
Unterstützt folgende Sensor-Typen:
- **'T'**: Temperatur (2 Float-Werte)
- **'D'**: Deflection (1 Float-Wert)
- **'P'**: Pressure (1 Float-Wert)
- **'S'**: PIC Temperature (1 Float-Wert)

### Konfiguration
```cpp
// Binärdaten-Modus aktivieren
uartReceiver.setBinaryMode(true);

// Erwartete Payload-Größe setzen
uartReceiver.setExpectedPayloadSize(24);

// Callback für Binärdaten setzen
uartReceiver.setBinaryCallback(onBinaryDataReceived);
```

## Payload-Format

Die erwartete Payload-Struktur:
```
T<float><float>D<float>P<float>S<float>
```

### Beispiel (24 Bytes):
- 'T' (1 Byte) + 2 Float-Werte (8 Bytes) = 9 Bytes
- 'D' (1 Byte) + 1 Float-Wert (4 Bytes) = 5 Bytes  
- 'P' (1 Byte) + 1 Float-Wert (4 Bytes) = 5 Bytes
- 'S' (1 Byte) + 1 Float-Wert (4 Bytes) = 5 Bytes
- **Gesamt: 24 Bytes**

## Verwendung

### 1. Grundlegende Konfiguration
```cpp
#include "UARTReceiver.h"

UARTReceiver uartReceiver(&Serial2, &Serial, TX_PIN, RX_PIN, BAUDRATE, LED_PIN);

void setup() {
    uartReceiver.begin();
    uartReceiver.setBinaryMode(true);
    uartReceiver.setExpectedPayloadSize(24);
    uartReceiver.setBinaryCallback(onBinaryDataReceived);
}
```

### 2. Callback-Funktion
```cpp
void onBinaryDataReceived(const uint8_t* data, size_t size) {
    Serial.println("Binärdaten empfangen!");
    
    // Weiterverarbeitung der Daten
    // z.B. an Cloud weiterleiten, auf SD-Karte speichern, etc.
}
```

### 3. Hauptschleife
```cpp
void loop() {
    uartReceiver.process();
    delay(1);
}
```

## Konfigurationsmöglichkeiten

### Header-Datei Defines
```cpp
#define MAX_PAYLOAD_SIZE 256        // Maximale Payload-Größe
#define PAYLOAD_TIMEOUT 5000        // Timeout für unvollständige Pakete (ms)
```

### Laufzeit-Konfiguration
```cpp
// Payload-Größe ändern
uartReceiver.setExpectedPayloadSize(32);

// Zwischen Text- und Binärmodus wechseln
uartReceiver.setBinaryMode(false);  // Text-Modus
uartReceiver.setBinaryMode(true);   // Binär-Modus
```

## Debug-Ausgabe

Im Debug-Modus werden folgende Informationen ausgegeben:

```
=== BINÄRE PAYLOAD EMPFANGEN ===
Größe: 24
54 41 42 43 44 45 46 47 48 49 4A 4B 4C 4D 4E 4F 50 51 52 53 54 55 56 57
Temp1: 23.45
Temp2: 24.67
Deflection: 0.123
Pressure: 1013.25
PIC Temp: 35.8
=== ENDE BINÄRE PAYLOAD ===
```

## Fehlerbehandlung

### Pufferüberlauf
```cpp
if (_bufferIndex >= MAX_PAYLOAD_SIZE) {
    Serial.println("ERROR: Buffer overflow!");
    _bufferIndex = 0;
}
```

### Timeout-Behandlung
```cpp
if (bufferIndex > 0 && timeout_reached) {
    Serial.println("WARNING: Unvollständige Payload - Puffer zurückgesetzt");
    _bufferIndex = 0;
}
```

### Unbekannte Sensor-Typen
```cpp
if (unknown_sensor_type) {
    Serial.println("Unbekannter Sensor-Typ erkannt");
    break;  // Stoppe Verarbeitung
}
```

## Erweiterte Funktionen

### Callback-System
Die Library unterstützt verschiedene Callback-Typen:
- `setBinaryCallback()`: Für Binärdaten
- `setMessageCallback()`: Für alle Nachrichten
- `setTimeoutCallback()`: Für Timeout-Events
- `setStatusCallback()`: Für Status-Updates

### Statistiken
```cpp
uint32_t messages, bytes;
unsigned long uptime;
uartReceiver.getStatistics(messages, bytes, uptime);
```

## Kompatibilität

### Unterstützte Plattformen
- ESP32
- ESP8266
- Arduino (mit entsprechender Serial-Unterstützung)

### Abhängigkeiten
- ArduinoJson (für JSON-Verarbeitung im Text-Modus)
- Standard Arduino/ESP Libraries

## Troubleshooting

### Häufige Probleme

1. **Keine Daten empfangen**
   - Überprüfe Verkabelung (TX/RX)
   - Überprüfe Baudrate
   - Teste mit `testBaudrates()`

2. **Unvollständige Pakete**
   - Überprüfe `setExpectedPayloadSize()`
   - Erhöhe `PAYLOAD_TIMEOUT`

3. **Falsche Sensor-Dekodierung**
   - Überprüfe Byte-Reihenfolge (Little/Big Endian)
   - Validiere Payload-Format

## Beispiele

Siehe `examples/BinaryReceiver/main.cpp` für ein vollständiges Beispiel.

## Lizenz

Diese Library steht unter der MIT-Lizenz zur Verfügung.
