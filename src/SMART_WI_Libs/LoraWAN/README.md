# SMART_WI_Libs - LoRaWAN Library Collection

**Ultra-einfache LoRaWAN-Bibliothek für SX1262 Module auf AVR128DB48**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/Platform-AVR128DB48-blue.svg)](https://github.com/SpenceKonde/DxCore)
[![RadioLib](https://img.shields.io/badge/RadioLib-v7.2.1-green.svg)](https://github.com/jgromes/RadioLib)

---

## 📋 Inhaltsverzeichnis

- [Überblick](#-überblick)
- [Features](#-features)
- [Hardware-Anforderungen](#-hardware-anforderungen)
- [Installation](#-installation)
- [Schnellstart](#-schnellstart)
- [Dateien-Übersicht](#-dateien-übersicht)
- [Konfiguration](#-konfiguration)
- [Verwendung](#-verwendung)
- [Payload-Struktur](#-payload-struktur)
- [ChirpStack Setup](#-chirpstack-setup)
- [Troubleshooting](#-troubleshooting)

---

## 🎯 Überblick

Die SMART_WI_Libs ist eine hochoptimierte LoRaWAN-Bibliothek, die speziell für industrielle Sensoranwendungen entwickelt wurde. Sie bietet eine einfache, aber mächtige API für die Übertragung von Sensordaten über LoRaWAN-Netzwerke.

**Hauptziel:** Reduzierung der Komplexität bei der LoRaWAN-Implementierung auf nur **zwei Hauptfunktionen**:
- `initializeSetup()` - Komplette Initialisierung
- `sendData()` - Datenübertragung

---

## ✨ Features

### 🚀 **Einfache Nutzung**
- **Nur 2 Hauptfunktionen** für den Einstieg
- **Automatische Initialisierung** mit Retry-Logik
- **Plug & Play** Konfiguration

### 📡 **LoRaWAN-Unterstützung**
- **OTAA (Over-the-Air Activation)** mit automatischen Retry-Versuchen
- **EU868 Frequenzband** optimiert
- **Adaptive Data Rate (ADR)** unterstützt
- **Duty Cycle** Management

### 📊 **Flexible Payload-Struktur**
- **Modularer Aufbau** für verschiedene Sensortypen
- **Automatische Payload-Optimierung**
- **EU868-konforme Größenlimits** (51 Bytes)
- **Skalierbare Sensor-Anzahl**

### 🔧 **Hardware-Kompatibilität**
- **SX1262 LoRaWAN Modul**
- **AVR128DB48 Mikrocontroller**
- **RadioLib v7.2.1** Integration

---

## 🛠 Hardware-Anforderungen

### **Mikrocontroller**
- **AVR128DB48** (mit DxCore Framework)
- **24MHz Takt**
- **16KB RAM, 128KB Flash**

### **LoRaWAN-Modul**
- **SX1262** Transceiver
- **EU868 Frequenzband**
- **3.3V Betriebsspannung**

### **Verbindungsschema**
```
AVR128DB48    ↔    SX1262
──────────          ──────
PIN_PD2       →     NSS (CS)
PIN_PE1       →     RESET
PIN_PD0       ←     BUSY
PIN_PB5       ←     DIO1
SPI Pins      ↔     SPI (MOSI, MISO, SCK)
GND           ─     GND
3.3V          ─     VCC
```

---

## 📦 Installation

### **PlatformIO Setup**
1. Kopiere die `SMART_WI_Libs` Dateien in dein `src/` Verzeichnis
2. Füge zu deiner `platformio.ini` hinzu:
```ini
[env:AVR128DB48]
platform = atmelmegaavr
board = AVR128DB48
framework = arduino
lib_deps = 
    jgromes/RadioLib@^7.2.1
```

### **Abhängigkeiten**
- **RadioLib v7.2.1+**
- **DxCore Framework**
- **Arduino.h**

---

## 🚀 Schnellstart

### **Minimales Beispiel (nur 2 Funktionen!)**

```cpp
#include <Arduino.h>
#include "SMART_WI_Libs/SX1262_LoRaWAN.h"

// LoRaWAN Instanz
SX1262_LoRaWAN lora;

void setup() {
    Serial.begin(115200);
    
    // 🎯 FUNKTION 1: Komplett-Initialisierung
    if (lora.initializeSetup()) {
        Serial.println("🎉 LoRaWAN bereit!");
    } else {
        Serial.println("❌ Initialisierung fehlgeschlagen!");
        while (true) delay(1000);
    }
}

void loop() {
    // Sensordaten definieren
    float temps[] = {23.5, 24.1, 22.8, 25.0};  // 2-4 Temperaturwerte
    float defs[] = {0.045, 0.052, 0.038};       // 1-3 Verformungswerte
    float press[] = {1013.25, 1012.8};          // 1-2 Druckwerte
    float others[] = {25.0, 3.3};               // 2 sonstige Werte
    
    // 🎯 FUNKTION 2: Daten senden
    if (lora.sendData(temps, 4, defs, 3, press, 2, others, 2)) {
        Serial.println("✅ Daten erfolgreich gesendet!");
    }
    
    delay(30000);  // 30 Sekunden warten
}
```

---

## 📁 Dateien-Übersicht

### **Hauptdateien**

| Datei | Beschreibung |
|-------|-------------|
| `SX1262_LoRaWAN.h` | 🎯 **Hauptklasse** - Einfache LoRaWAN-API |
| `SX1262_LoRaWAN.cpp` | 🔧 **Implementierung** - Alle LoRaWAN-Funktionen |
| `lorawanconfig.h` | ⚙️ **Konfiguration** - Keys, Pins, Parameter |
| `lorawanconfig.cpp` | 🔗 **Config-Implementierung** - Variablen & Funktionen |
| `Payload_Builder.h` | 📦 **Payload-API** - Modularer Payload-Aufbau |
| `Payload_Builder.cpp` | 🔨 **Payload-Implementierung** - Binäre Payload-Erzeugung |

### **Kernfunktionen**

#### **SX1262_LoRaWAN Klasse**
- `initializeSetup()` - Komplette Initialisierung (Radio + LoRaWAN + Join)
- `sendData()` - Flexible Datenübertragung mit Payload-Validierung
- `printDebugInfo()` - Debug-Ausgaben für Troubleshooting

#### **Payload_Builder Klasse**
- Modularer Payload-Aufbau für verschiedene Sensortypen
- Automatische Größenoptimierung
- Debug-Unterstützung für Payload-Analyse

---

## ⚙️ Konfiguration

### **LoRaWAN-Credentials (`lorawanconfig.h`)**

```cpp
// OTAA Parameter - VON IHREM LORAWAN-PROVIDER ANPASSEN!
const char joinEUI_str[] = "57fa6af7e943453d";  // Join EUI
const char devEUI_str[] = "c95ca89cc36cee56";   // Device EUI
const char nwkKey_str[] = "ee428117008c670dc1559c86ead819f0";  // Network Key
const char appKey_str[] = "ee428117008c670dc1559c86ead819f0";  // Application Key
```

### **Hardware-Pins**
```cpp
#define NSS_PIN    PIN_PD2   // CS/NSS Pin
#define RESET_PIN  PIN_PE1   // Reset Pin
#define BUSY_PIN   PIN_PD0   // Busy Pin
#define DIO1_PIN   PIN_PB5   // DIO1 Pin
```

### **Sensor-Konfiguration**
```cpp
// Sensor-Aktivierung
const bool TEMP_SENSOR_AKTIV = true;
const bool DEFLECTION_SENSOR_AKTIV = true;
const bool PRESSURE_SENSOR_AKTIV = true;
const bool SONSTIGES_SENSOR_AKTIV = true;
```

---

## 🔧 Verwendung

### **Hauptfunktionen**

#### **1. Initialisierung**
```cpp
SX1262_LoRaWAN lora;

// Komplette Initialisierung mit Retry-Logik
bool success = lora.initializeSetup();
```

**Was passiert intern:**
- ✅ Keys initialisieren
- ✅ SX1262 Radio initialisieren
- ✅ LoRaWAN Node konfigurieren
- ✅ Netzwerk-Join (bis zu 5 Versuche mit 30s Pause)
- ✅ Debug-Informationen ausgeben

#### **2. Datenübertragung**
```cpp
// Sensordaten definieren
float temps[] = {23.5, 24.1};        // 2 Temperaturwerte
float defs[] = {0.045};               // 1 Verformungswert
float press[] = {1013.25, 1012.8};   // 2 Druckwerte
float others[] = {25.0, 3.3};        // 2 sonstige Werte

// Daten senden
bool success = lora.sendData(temps, 2, defs, 1, press, 2, others, 2);
```

**Payload-Validierung:**
- ✅ Temperatur: 2-4 Werte erlaubt
- ✅ Verformung: 1-3 Werte erlaubt
- ✅ Druck: 1-2 Werte erlaubt
- ✅ Sonstiges: genau 2 Werte
- ✅ EU868-Limit: Maximal 51 Bytes

---

## 📦 Payload-Struktur

### **Binäres Format**

Die Payload folgt einem strukturierten Binärformat:

```
[Typ][Wert1][Wert2]...[Typ][Wert1][Wert2]...
```

#### **Sensor-Typen**
| Typ | Byte | Beschreibung | Werte | Größe |
|-----|------|-------------|--------|-------|
| **T** | 0x54 | Temperatur | 2-4 Float | 9-17 Bytes |
| **D** | 0x44 | Verformung | 1-3 Float | 5-13 Bytes |
| **P** | 0x50 | Druck | 1-2 Float | 5-9 Bytes |
| **S** | 0x53 | Sonstiges | 2 Float | 9 Bytes |

#### **Beispiel-Payload**
```
T 41 BC 00 00 41 C0 CC CD    // 'T' + 23.5°C + 24.1°C
D 3D 37 89 FC                // 'D' + 0.045mm
P 44 7D 84 21 44 7D 66 66    // 'P' + 1013.25hPa + 1012.8hPa
S 41 C8 00 00 40 53 33 33    // 'S' + 25.0°C + 3.3V

Beispiel für einen möglichen Payload-String (für Debug-Zwecke):
Angenommen, es werden 3 Temperaturen, 2 Deflections, 1 Druck und 2 sonstige Werte gesendet:
Payload als String: T[cd][cc][4c][42][00][00][20][41][66][66][86][41]D[9a][99][19][3f][00][00][00][40]P[9a][99][19][42]S[00][00][20][41][9a][99][19][41]
Dabei steht z.B. [cd][cc][4c][42] für den ersten Temperaturwert als Float im Hex-Format.
Die Buchstaben T, D, P, S sind die jeweiligen Sektionskennungen.
```

---

## 🌐 ChirpStack Setup

### **JavaScript Payload-Decoder**

Für ChirpStack/TTN verwende diesen Decoder:

```javascript
function Decode(fPort, bytes) {
    var result = {};
    var offset = 0;
    
    // Temperatur-Daten (Type 'T' = 0x54)
    if (bytes.length > offset && bytes[offset] === 0x54) {
        offset++;
        result.temperatures = [];
        while (offset + 4 <= bytes.length && 
               bytes[offset] !== 0x44 && bytes[offset] !== 0x50 && bytes[offset] !== 0x53) {
            var temp = new Float32Array(new Uint8Array(bytes.slice(offset, offset + 4)).buffer)[0];
            result.temperatures.push(parseFloat(temp.toFixed(2)));
            offset += 4;
        }
    }
    
    // Verformung-Daten (Type 'D' = 0x44)
    if (bytes.length > offset && bytes[offset] === 0x44) {
        offset++;
        result.deflections = [];
        while (offset + 4 <= bytes.length && 
               bytes[offset] !== 0x54 && bytes[offset] !== 0x50 && bytes[offset] !== 0x53) {
            var defl = new Float32Array(new Uint8Array(bytes.slice(offset, offset + 4)).buffer)[0];
            result.deflections.push(parseFloat(defl.toFixed(4)));
            offset += 4;
        }
    }
    
    // Druck-Daten (Type 'P' = 0x50)
    if (bytes.length > offset && bytes[offset] === 0x50) {
        offset++;
        result.pressures = [];
        while (offset + 4 <= bytes.length && 
               bytes[offset] !== 0x54 && bytes[offset] !== 0x44 && bytes[offset] !== 0x53) {
            var press = new Float32Array(new Uint8Array(bytes.slice(offset, offset + 4)).buffer)[0];
            result.pressures.push(parseFloat(press.toFixed(1)));
            offset += 4;
        }
    }
    
    // Sonstige Daten (Type 'S' = 0x53)
    if (bytes.length > offset && bytes[offset] === 0x53) {
        offset++;
        result.misc = [];
        while (offset + 4 <= bytes.length && 
               bytes[offset] !== 0x54 && bytes[offset] !== 0x44 && bytes[offset] !== 0x50) {
            var misc = new Float32Array(new Uint8Array(bytes.slice(offset, offset + 4)).buffer)[0];
            result.misc.push(parseFloat(misc.toFixed(2)));
            offset += 4;
        }
    }
    
    return result;
}
```

### **ChirpStack Gerät anlegen**

1. **Application erstellen**
2. **Device Profile erstellen** (EU868, LoRaWAN 1.0.3)
3. **Device anlegen** mit Device EUI
4. **OTAA Keys eintragen** (AppKey)
5. **Payload-Decoder hinzufügen**

---

## 🔧 Troubleshooting

### **Häufige Probleme**

#### **❌ "Radio initialization failed"**
- SPI-Verkabelung prüfen
- Pin-Definitionen in `lorawanconfig.h` prüfen
- 3.3V Spannungsversorgung sicherstellen

#### **❌ "Join fehlgeschlagen"**
- ChirpStack/TTN-Konfiguration prüfen
- Keys in `lorawanconfig.h` validieren
- Gateway-Verbindung testen
- Antennenverbindung prüfen

#### **❌ "Payload zu groß"**
- Weniger Sensoren pro Übertragung
- Payload-Größe mit `displayPayloadContent()` prüfen
- EU868-Limit: 51 Bytes beachten

#### **❌ "Keine Daten in ChirpStack"**
- Live LoRaWAN Frames überwachen
- Payload-Decoder testen
- Frame-Counter-Validierung deaktivieren

### **Debug-Ausgaben aktivieren**

```cpp
// Debug-Informationen anzeigen
lora.printDebugInfo();        // Keys und EUIs
lora.printPinConfiguration(); // Pin-Zuordnung
```

---

## 📊 Technische Spezifikationen

### **Speicherverbrauch**
- **RAM**: ~4.8KB (29.3% von 16KB)
- **Flash**: ~70KB (53.9% von 128KB)

### **LoRaWAN-Parameter**
- **Frequenzband**: EU868
- **Activation**: OTAA
- **Class**: A
- **ADR**: Aktiviert
- **Duty Cycle**: Automatisch verwaltet

### **Limits**
- **Payload-Größe**: 51 Bytes (EU868)
- **Uplink-Intervall**: 30 Sekunden (konfigurierbar)
- **Join-Versuche**: 5 mit 30s Pause

---

## 📄 Lizenz

MIT License - Siehe [LICENSE](LICENSE) für Details.

---

## 👥 Entwickler

**Smart Wire Industries**
- Version: 1.0.0
- Datum: 2025-01-17
- Kontakt: [GitHub Issues](https://github.com/smartwire/SMART_WI_Libs/issues)

---

## 🔗 Weiterführende Links

- [RadioLib Documentation](https://github.com/jgromes/RadioLib)
- [ChirpStack Documentation](https://www.chirpstack.io/docs/)
- [AVR128DB48 Datasheet](https://www.microchip.com/wwwproducts/en/AVR128DB48)
- [LoRaWAN Specification](https://lora-alliance.org/resource_hub/lorawan-specification-v1-0-3/)

---

**🎯 Ziel erreicht: LoRaWAN in nur 2 Funktionen!**
```cpp
lora.initializeSetup()  // Alles initialisieren
lora.sendData(...)      // Daten senden
```
