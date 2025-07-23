# UARTReceiver Library

Eine leistungsstarke und einfach zu verwendende Library für das Empfangen und Verarbeiten von UART-Daten auf AVR-IoT Cellular Mini Boards.

## Features

- 🔄 Automatische JSON-Erkennung und -Verarbeitung
- 📝 Unterstützung für Text- und Binärdaten
- 🔔 Callback-System für verschiedene Ereignisse
- ⏱️ Timeout-Erkennung für fehlende Daten
- 📊 Integrierte Statistiken und Monitoring
- 🔧 Konfigurierbare Parameter
- 🚀 Einfache Integration in bestehende Projekte

## Installation

1. Kopieren Sie die Library-Dateien in Ihr `lib/UARTReceiver/` Verzeichnis
2. Inkludieren Sie die Header-Datei in Ihr Projekt: `#include <UARTReceiver.h>`
3. Stellen Sie sicher, dass die ArduinoJson Library verfügbar ist

## Grundlegende Verwendung

```cpp
#include <UARTReceiver.h>
#include <UART_Libs/KitConfig.h>
#include <UART_Libs/SerialMon.h>

// LED Pin für Statusanzeige
const int LED_PIN = 13;

// UARTReceiver Instanz erstellen
UARTReceiver uartReceiver(&Serial2, &SerialMon, SerialPIN_TX, SerialPIN_RX, UART2_BAUDRATE, LED_PIN);

void setup() {
    SerialMon.begin(SERIAL_MON_BAUDRATE);
    
    // UART Receiver initialisieren
    if (!uartReceiver.begin()) {
        SerialMon.println("Fehler beim Initialisieren!");
        while (true) delay(1000);
    }
    
    // Callbacks registrieren
    uartReceiver.setJSONCallback([](JsonObject data) {
        SerialMon.println("JSON empfangen!");
        // JSON-Daten verarbeiten
    });
    
    uartReceiver.setTextCallback([](const String& text) {
        SerialMon.println("Text: " + text);
    });
}

void loop() {
    uartReceiver.process();
    delay(1);
}
```

## Callback-Funktionen

Die Library bietet verschiedene Callback-Funktionen für unterschiedliche Ereignisse:

### JSON-Callback
```cpp
uartReceiver.setJSONCallback([](JsonObject data) {
    if (data["type"] == "uplink_data") {
        // Uplink-Daten verarbeiten
        JsonObject uplinkData = data["data"];
        // ...
    }
});
```

### Text-Callback
```cpp
uartReceiver.setTextCallback([](const String& text) {
    SerialMon.println("Empfangener Text: " + text);
});
```

### Timeout-Callback
```cpp
uartReceiver.setTimeoutCallback([](unsigned long timeoutMs) {
    SerialMon.print("Timeout nach ");
    SerialMon.print(timeoutMs / 1000);
    SerialMon.println(" Sekunden");
});
```

### Status-Callback
```cpp
uartReceiver.setStatusCallback([](uint32_t messages, uint32_t bytes, unsigned long uptime) {
    SerialMon.print("Status: ");
    SerialMon.print(messages);
    SerialMon.print(" Nachrichten, ");
    SerialMon.print(bytes);
    SerialMon.println(" Bytes");
});
```

## Konfiguration

Die Library bietet verschiedene Konfigurationsmöglichkeiten:

```cpp
// Timeout für Datenempfang (in Millisekunden)
uartReceiver.setTimeout(15000);

// Intervall für Status-Updates (in Millisekunden)
uartReceiver.setStatusUpdateInterval(30000);

// Heartbeat-Intervall (in Millisekunden)
uartReceiver.setHeartbeatInterval(60000);

// Puffergröße (in Bytes)
uartReceiver.setBufferSize(4096);
```

## Erweiterte Funktionen

### Baudraten-Test
```cpp
// Testet verschiedene Baudraten automatisch
uartReceiver.testBaudrates();
```

### Statistiken abrufen
```cpp
uint32_t messages, bytes;
unsigned long uptime;
uartReceiver.getStatistics(messages, bytes, uptime);
```

### Testdaten senden
```cpp
uartReceiver.sendTestData("{\"type\":\"test\",\"data\":\"Hello World\"}");
```

### System-Informationen anzeigen
```cpp
uartReceiver.displaySystemInfo();
```

## Konstruktor-Parameter

```cpp
UARTReceiver(HardwareSerial* serial, Stream* debugSerial, 
             int txPin, int rxPin, uint32_t baudrate, int ledPin = -1)
```

- `serial`: Hardware-Serial-Schnittstelle (z.B. &Serial2)
- `debugSerial`: Debug-Ausgabe-Schnittstelle (z.B. &SerialMon)
- `txPin`: TX-Pin für UART-Kommunikation
- `rxPin`: RX-Pin für UART-Kommunikation
- `baudrate`: Baudrate für UART-Kommunikation
- `ledPin`: LED-Pin für Statusanzeige (optional, -1 für keine LED)

## Standardkonfiguration

Die Library verwendet folgende Standardwerte:

- Puffergröße: 2048 Bytes
- Timeout: 10 Sekunden
- Status-Update-Intervall: 30 Sekunden
- Heartbeat-Intervall: 30 Sekunden

Diese können durch Defines vor dem Include überschrieben werden:

```cpp
#define UART_BUFFER_SIZE 4096
#define UART_TIMEOUT_MS 20000
#include <UARTReceiver.h>
```

## Beispiele

Siehe `examples/BasicUsage/BasicUsage.ino` für ein vollständiges Beispiel der Library-Verwendung.

## JSON-Format

Die Library erkennt automatisch JSON-Nachrichten und verarbeitet diese. Beispiel für unterstützte JSON-Strukturen:

```json
{
  "type": "uplink_data",
  "msg_id": 12345,
  "data": {
    "dev_eui": "1234567890ABCDEF",
    "data_hex": "48656C6C6F",
    "rssi": -85,
    "snr": 7.5,
    "frequency": 868100000
  }
}
```

## Kompatibilität

- AVR-IoT Cellular Mini Board (ATmega4808)
- PlatformIO
- Arduino IDE
- ArduinoJson Library (Version 6.x)

## Lizenz

Diese Library basiert auf dem ursprünglichen UART-Empfänger-Projekt und ist für den Einsatz in eigenen Projekten freigegeben.

## Changelog

### Version 1.0.0 (2025-07-15)
- Erste Version der Library
- Grundlegende UART-Empfangsfunktionalität
- JSON-Verarbeitung
- Callback-System
- Konfigurationsmöglichkeiten
- Beispiele und Dokumentation
