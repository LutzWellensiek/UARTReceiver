# UART Receiver

Dieses Projekt ist ein UART-Empfänger für das AVR-IoT Cellular Mini Board (ATmega4808). Es empfängt Daten über UART2 von einem externen Gerät, wie einem LoRa-Node, und leitet diese über die USB-Schnittstelle an den PC weiter.

## Hardwarekonfiguration

- **Board**: AVR128DB48 (AVR-IoT Cellular Mini Board)
- **UART2 TX Pin**: PF4 (zu externem Gerät RX)
- **UART2 RX Pin**: PF5 (von externem Gerät TX)
- **Baudrate**:
  - UART2: 115200 Baud
  - USB Serial: 115200 Baud (Serial Monitor)

## Funktionen

- Kontinuierlicher Empfang von Daten über UART2
- Ausgabe empfangener Daten über USB Serial
- Timeout-Erkennung bei fehlenden Daten (5 Sekunden)
- Periodische Status-Updates alle 10 Sekunden
- Nachrichten-Statistiken (Anzahl Nachrichten, Bytes gesamt, Uptime)
- Pufferüberlauf-Schutz (256 Bytes Puffergröße)
- Hardware-Funktionstest beim Start

## Technische Details

### Timing-Konfiguration
- **Data Timeout**: 5 Sekunden
- **Status Update**: 10 Sekunden
- **Puffergröße**: 256 Bytes

### Pin-Zuordnung
- **UART2 TX**: PIN_PF4 (zu externem Gerät RX)
- **UART2 RX**: PIN_PF5 (von externem Gerät TX)

## Anforderungen

- **Plattform**: atmelmegaavr
- **Framework**: Arduino
- **Board**: AVR128DB48
- **DxCore**: Version 1.5.11
- **Upload-Protokoll**: pkobn_updi

## Projektstruktur

```
UARTReceiver/
├── src/
│   ├── main.cpp              # Hauptprogramm
│   └── UART_Libs/
│       ├── KitConfig.h       # Konfigurationsdatei
│       └── SerialMon.h       # Serial Monitor Definitionen
├── platformio.ini            # PlatformIO Konfiguration
└── README.md                 # Diese Datei
```

## Installation und Verwendung

1. **Projekt öffnen**: Öffne das Projekt in PlatformIO
2. **Hardware anschließen**: Verbinde das AVR-IoT Cellular Mini Board
3. **Code kompilieren**: Verwende PlatformIO zum Kompilieren
4. **Upload**: Lade den Code über UPDI auf das Board
5. **Serial Monitor**: Öffne den Serial Monitor mit 115200 Baud
6. **Externe Verbindung**: Verbinde das externe Gerät mit den UART2-Pins

## Verwendung

### Beim Start
Das System zeigt folgende Informationen an:
- Hardware-Initialisierung
- Systeminformationen
- Konfiguration
- Pin-Zuordnung
- Status

### Während des Betriebs
- Empfangene Daten werden direkt im Serial Monitor angezeigt
- Nachrichten werden mit Nummer und Byte-Anzahl markiert
- Timeout-Meldungen erscheinen bei fehlenden Daten
- Periodische Statistiken zeigen Gesamtübersicht

### Beispiel-Ausgabe
```
=== AVR128DB48 UART-Empfänger ===
Board: AVR-IoT Cellular Mini (ATmega4808)
Funktion: UART-zu-USB Bridge

=== Konfiguration ===
UART2 Baudrate: 115200 Baud
USB Baudrate: 115200 Baud
Daten-Timeout: 5 Sekunden
Puffergröße: 256 Bytes

Bereit für Datenempfang...
```

## Fehlerbehebung

### Keine Daten empfangen
- Überprüfe die Verkabelung (TX ↔ RX)
- Kontrolliere die Baudrate des externen Geräts
- Stelle sicher, dass beide Geräte die gleiche Masse haben

### Pufferüberlauf
- Reduziere die Datenrate des externen Geräts
- Überprüfe auf kontinuierliche Datenströme ohne Pausen

### Hardware-Probleme
- Verwende den integrierten Loopback-Test
- Überprüfe die Pin-Konfiguration

## Lizenz

Dieses Projekt basiert auf dem LoRaWAN-Endnode-Projekt und wird entsprechend lizenziert.

## Autor

Basierend auf LoRaWAN-Endnode-Projekt
Erstellt: 2025-07-02
