# ChirpStackReceiver - Sensordaten Abruf API

Diese Anleitung beschreibt, wie Sie die dekodierten Sensordaten aus dem ChirpStackReceiver dynamisch abrufen können.

## Übersicht

Der ChirpStackReceiver speichert automatisch alle empfangenen und dekodierten Sensordaten in einer strukturierten Form. Sie können diese Daten jederzeit abrufen und in Ihrem Code verwenden.

## Datenstrukturen

### SensorValue
Einzelner Sensorwert mit Metadaten:
```cpp
struct SensorValue {
    uint8_t tag;           // Sensor-Typ (0x01-0x04)
    uint8_t index;         // Index innerhalb des Typs (0-3)
    float value;           // Sensorwert
    unsigned long timestamp; // Zeitstempel des Empfangs
}
```

### SensorData
Komplette Sensordaten eines Empfangs:
```cpp
struct SensorData {
    String deviceId;                    // Device ID
    std::vector<SensorValue> values;    // Alle Sensorwerte
    unsigned long lastUpdate;           // Zeitpunkt der letzten Aktualisierung
    size_t rawPayloadSize;              // Größe der Rohdaten
}
```

## Verfügbare Funktionen

### 1. Alle Sensordaten abrufen
```cpp
SensorData data = receiver.getLastSensorData();
```

### 2. Prüfen ob neue Daten vorhanden sind
```cpp
unsigned long lastCheck = millis();
// ... später im Code ...
if (receiver.hasNewData(lastCheck)) {
    // Neue Daten sind vorhanden
    lastCheck = millis();
}
```

### 3. Einzelne Sensorwerte abrufen
```cpp
SensorData data = receiver.getLastSensorData();

// Temperatur mit Index 0 (erste Temperatur)
float temp1 = data.getTemperature(0);
float temp2 = data.getTemperature(1);

// Deflection Werte
float defl1 = data.getDeflection(0);

// Pressure Werte
float press1 = data.getPressure(0);

// Misc/Sonstige Werte
float misc1 = data.getMisc(0);
```

### 4. Alle Werte eines Typs abrufen
```cpp
std::vector<float> allTemps = data.getAllTemperatures();
std::vector<float> allDefls = data.getAllDeflections();
std::vector<float> allPress = data.getAllPressures();
std::vector<float> allMisc = data.getAllMisc();
```

### 5. Als JSON-String exportieren
```cpp
String jsonData = receiver.getSensorDataAsJson();
Serial.println(jsonData);
```

## Beispiel-Code

### Basis-Beispiel
```cpp
#include <ChirpStackReceiver.h>

ChirpStackReceiver receiver(&Serial1, &Serial, 0, 1, 9600);

void setup() {
    Serial.begin(115200);
    receiver.begin();
}

void loop() {
    receiver.process();
    
    // Sensordaten abrufen
    SensorData data = receiver.getLastSensorData();
    
    if (data.hasData()) {
        Serial.print("Device ID: ");
        Serial.println(data.deviceId);
        
        // Erste Temperatur ausgeben
        float temp = data.getTemperature(0);
        if (!isnan(temp)) {
            Serial.print("Temperatur: ");
            Serial.print(temp);
            Serial.println(" °C");
        }
    }
    
    delay(1000);
}
```

### Erweiteres Beispiel mit Änderungserkennung
```cpp
unsigned long lastDataCheck = 0;

void loop() {
    receiver.process();
    
    // Prüfe auf neue Daten
    if (receiver.hasNewData(lastDataCheck)) {
        lastDataCheck = millis();
        
        SensorData data = receiver.getLastSensorData();
        
        Serial.println("\n=== NEUE SENSORDATEN ===");
        Serial.print("Device: ");
        Serial.println(data.deviceId);
        Serial.print("Zeitstempel: ");
        Serial.println(data.lastUpdate);
        
        // Alle Temperaturen ausgeben
        auto temps = data.getAllTemperatures();
        for (size_t i = 0; i < temps.size(); i++) {
            Serial.print("Temp ");  
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(temps[i]);
            Serial.println(" °C");
        }
        
        // Alle Deflections ausgeben
        auto defls = data.getAllDeflections();
        for (size_t i = 0; i < defls.size(); i++) {
            Serial.print("Defl ");
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(defls[i]);
            Serial.println(" mm");
        }
    }
}
```

### JSON-Export Beispiel
```cpp
void exportSensorData() {
    String json = receiver.getSensorDataAsJson();
    
    // JSON über Serial ausgeben
    Serial.println(json);
    
    // Oder an einen Webserver senden
    // httpClient.POST(json);
}
```

### Beispiel mit Schwellwert-Überwachung
```cpp
void checkThresholds() {
    SensorData data = receiver.getLastSensorData();
    
    // Temperatur-Überwachung
    float temp = data.getTemperature(0);
    if (!isnan(temp) && temp > 30.0) {
        Serial.println("WARNUNG: Temperatur zu hoch!");
        // Alarm auslösen
    }
    
    // Druck-Überwachung
    float pressure = data.getPressure(0);
    if (!isnan(pressure) && pressure < 900.0) {
        Serial.println("WARNUNG: Druck zu niedrig!");
    }
}
```

## Sensor-Tags

Die folgenden Tags werden für die verschiedenen Sensortypen verwendet:

- `TAG_TEMPERATURE` (0x01): Temperatur in °C
- `TAG_DEFLECTION` (0x02): Auslenkung/Deflection in mm
- `TAG_PRESSURE` (0x03): Druck in hPa
- `TAG_MISC` (0x04): Sonstige Werte

## JSON-Format

Die `getSensorDataAsJson()` Funktion gibt folgendes Format zurück:

```json
{
  "deviceId": "0102030405060708",
  "timestamp": 123456789,
  "payloadSize": 28,
  "temperatures": [
    {"index": 0, "value": 23.45, "timestamp": 123456789},
    {"index": 1, "value": 24.56, "timestamp": 123456789}
  ],
  "deflections": [
    {"index": 0, "value": 1.23, "timestamp": 123456789}
  ],
  "pressures": [
    {"index": 0, "value": 1013.25, "timestamp": 123456789}
  ],
  "misc": [
    {"index": 0, "value": 42.0, "timestamp": 123456789}
  ]
}
```

## Tipps

1. **Null-Prüfung**: Prüfen Sie immer mit `isnan()` ob ein Wert gültig ist
2. **Zeitstempel**: Nutzen Sie `lastUpdate` um das Alter der Daten zu prüfen
3. **Performance**: Die Getter-Funktionen sind optimiert und können häufig aufgerufen werden
4. **Thread-Safety**: Die Bibliothek ist für Single-Thread Umgebungen (Arduino) ausgelegt

## Fehlerbehandlung

```cpp
SensorData data = receiver.getLastSensorData();

// Prüfe ob überhaupt Daten vorhanden sind
if (!data.hasData()) {
    Serial.println("Noch keine Daten empfangen");
    return;
}

// Prüfe Alter der Daten
unsigned long dataAge = millis() - data.lastUpdate;
if (dataAge > 60000) { // Älter als 60 Sekunden
    Serial.println("Warnung: Daten sind veraltet");
}

// Prüfe einzelne Werte
float temp = data.getTemperature(0);
if (isnan(temp)) {
    Serial.println("Kein Temperaturwert vorhanden");
}
```
