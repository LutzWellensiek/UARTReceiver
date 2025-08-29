/**
 * @file main.cpp
 * @brief ChirpStack MQTT to UART Bridge Receiver für AVR-IoT Cellular Mini Board
 * @author Smart Wire Industries
 * @version 3.0.0
 * @date 2025-01-24
 * 
 * @details
 * Nutzt die ChirpStackReceiver-Bibliothek um LoRaWAN-Sensordaten zu empfangen
 * und dynamisch abzurufen.
 * 
 * Hardware-Konfiguration:
 * - Board: AVR128DB48 (AVR-IoT Cellular Mini)
 * - UART2: Bridge-Kommunikation (115200 baud)
 * - USB Serial: Debug-Monitor (115200 baud)
 */

#include <Arduino.h>
#include "SMART_WI_Libs/LoraWAN/ChirpStackReceiver/ChirpStackReceiver.h"
#include "SMART_WI_Libs/KitConfig.h"
#include "SMART_WI_Libs/SerialMon.h"

// ========================================
// Globale Objekte
// ========================================

ChirpStackReceiver receiver(&Serial2, &SerialMon, SerialPIN_TX, SerialPIN_RX, UART2_BAUDRATE);

// Für Änderungserkennung
unsigned long lastDataCheck = 0;
unsigned long lastDisplayTime = 0;
const unsigned long DISPLAY_INTERVAL = 5000;  // Zeige Daten alle 5 Sekunden

// ========================================
// Hilfsfunktionen
// ========================================

void displaySensorData(const SensorData& data) {
    SerialMon.println(F("\n=== SENSOR DATEN ==="));
    
    if (data.deviceId.length() > 0) {
        SerialMon.print(F("Device ID: "));
        SerialMon.println(data.deviceId);
    }
    
    SerialMon.print(F("Zeitstempel: "));
    SerialMon.println(data.lastUpdate);
    
    // Temperaturen
    float temps[8];
    size_t tempCount = data.getAllTemperatures(temps, 8);
    if (tempCount > 0) {
        SerialMon.println(F("\nTemperaturen:"));
        for (size_t i = 0; i < tempCount; i++) {
            SerialMon.print(F("  T"));
            SerialMon.print(i + 1);
            SerialMon.print(F(": "));
            SerialMon.print(temps[i], 2);
            SerialMon.println(F(" °C"));
        }
    }
    
    // Deflections
    float defls[8];
    size_t deflCount = data.getAllDeflections(defls, 8);
    if (deflCount > 0) {
        SerialMon.println(F("\nDeflections:"));
        for (size_t i = 0; i < deflCount; i++) {
            SerialMon.print(F("  D"));
            SerialMon.print(i + 1);
            SerialMon.print(F(": "));
            SerialMon.print(defls[i], 2);
            SerialMon.println(F(" mm"));
        }
    }
    
    // Druck
    float pressures[8];
    size_t pressCount = data.getAllPressures(pressures, 8);
    if (pressCount > 0) {
        SerialMon.println(F("\nDruck:"));
        for (size_t i = 0; i < pressCount; i++) {
            SerialMon.print(F("  P"));
            SerialMon.print(i + 1);
            SerialMon.print(F(": "));
            SerialMon.print(pressures[i], 2);
            SerialMon.println(F(" hPa"));
        }
    }
    
    // Sonstige
    float misc[8];
    size_t miscCount = data.getAllMisc(misc, 8);
    if (miscCount > 0) {
        SerialMon.println(F("\nSonstige:"));
        for (size_t i = 0; i < miscCount; i++) {
            SerialMon.print(F("  M"));
            SerialMon.print(i + 1);
            SerialMon.print(F(": "));
            SerialMon.println(misc[i], 2);
        }
    }
    
    SerialMon.println(F("==================\n"));
}

void checkForAlerts(const SensorData& data) {
    // Beispiel: Temperatur-Alarm
    float temp = data.getTemperature(0);
    if (!isnan(temp) && temp > 30.0) {
        SerialMon.println(F("⚠️ WARNUNG: Temperatur über 30°C!"));
    }
    
    // Beispiel: Druck-Alarm
    float pressure = data.getPressure(0);
    if (!isnan(pressure) && pressure < 950.0) {
        SerialMon.println(F("⚠️ WARNUNG: Niedriger Druck!"));
    }
}

// ========================================
// Arduino Hauptfunktionen
// ========================================

void setup() {
    // Serielle Kommunikation initialisieren
    SerialMon.begin(SERIAL_MON_BAUDRATE);
    while (!SerialMon) {
        delay(10);
    }
    
    SerialMon.println(F("\n========================================"));
    SerialMon.println(F("ChirpStack Sensor Data Receiver v3.0"));
    SerialMon.println(F("========================================"));
    SerialMon.println(F("Initialisiere System..."));
    
    // ChirpStackReceiver initialisieren
    if (!receiver.begin()) {
        SerialMon.println(F("❌ FEHLER: Receiver-Initialisierung fehlgeschlagen!"));
        while (1) delay(1000);
    }
    
    SerialMon.println(F("✅ System bereit!"));
    SerialMon.println(F("\nWarte auf Sensordaten..."));
    SerialMon.println(F("(Die empfangenen Daten werden automatisch dekodiert und gespeichert)\n"));
    
    lastDataCheck = millis();
    lastDisplayTime = millis();
}

void loop() {
    // Verarbeite eingehende Nachrichten
    receiver.process();
    
    // Prüfe auf neue Daten
    if (receiver.hasNewData(lastDataCheck)) {
        lastDataCheck = millis();
        
        SerialMon.println(F("\n📡 NEUE DATEN EMPFANGEN!"));
        
        // Hole die neuesten Sensordaten
        SensorData data = receiver.getLastSensorData();
        
        // Zeige die Daten an
        displaySensorData(data);
        
        // Prüfe auf Alarme
        checkForAlerts(data);
        
        // Optional: Exportiere als JSON
        // String jsonData = receiver.getSensorDataAsJson();
        // SerialMon.println(jsonData);
    }
    
    // Periodische Anzeige der aktuellen Daten
    if (millis() - lastDisplayTime > DISPLAY_INTERVAL) {
        lastDisplayTime = millis();
        
        SensorData data = receiver.getLastSensorData();
        if (data.hasData()) {
            SerialMon.println(F("\n📊 AKTUELLE SENSORDATEN:"));
            
            // Kompakte Anzeige
            SerialMon.print(F("Device: "));
            SerialMon.print(data.deviceId);
            
            float temp = data.getTemperature(0);
            if (!isnan(temp)) {
                SerialMon.print(F(" | Temp: "));
                SerialMon.print(temp, 1);
                SerialMon.print(F("°C"));
            }
            
            float pressure = data.getPressure(0);
            if (!isnan(pressure)) {
                SerialMon.print(F(" | Druck: "));
                SerialMon.print(pressure, 0);
                SerialMon.print(F("hPa"));
            }
            
            SerialMon.println();
        }
    }
    
    // Kleine Pause für Stabilität
    delay(10);
}
