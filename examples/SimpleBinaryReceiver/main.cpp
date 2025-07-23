/**
 * @file main.cpp
 * @brief Einfaches Beispiel für UARTReceiverBinary Library
 * @author UARTReceiver Library
 * @date 2025-07-15
 */

#include <Arduino.h>
#include "UARTReceiverBinary.h"

// Hardware-Konfiguration - ANPASSEN FÜR DEIN PROJEKT
#define UART_TX_PIN 14       // GPIO 14 (TX Pin)
#define UART_RX_PIN 12       // GPIO 12 (RX Pin)
#define UART_BAUDRATE 115200 // Baudrate anpassen
#define LED_PIN 2            // Status-LED (optional)

// UARTReceiverBinary Instanz erstellen
UARTReceiverBinary receiver(&Serial2, &Serial, UART_TX_PIN, UART_RX_PIN, UART_BAUDRATE, LED_PIN);

/**
 * @brief Callback-Funktion für empfangene Binärdaten
 * Diese Funktion wird aufgerufen, wenn ein komplettes Datenpaket empfangen wurde
 * 
 * @param data Zeiger auf die empfangenen Binärdaten
 * @param size Größe der empfangenen Daten in Bytes
 */
void onBinaryDataReceived(const uint8_t* data, size_t size) {
    Serial.println("\n=== NEUE SENSORDATEN EMPFANGEN ===");
    
    // Dekodiere die Sensordaten
    SensorData sensorData = receiver.decodeSensorData(data, size);
    
    // Zeige dekodierte Werte an
    Serial.print("Zeitstempel: ");
    Serial.print(sensorData.timestamp);
    Serial.println(" ms");
    
    if (sensorData.hasTemperature) {
        Serial.print("Temperatur 1: ");
        Serial.print(sensorData.temperature1, 2);
        Serial.println(" °C");
        
        Serial.print("Temperatur 2: ");
        Serial.print(sensorData.temperature2, 2);
        Serial.println(" °C");
    }
    
    if (sensorData.hasDeflection) {
        Serial.print("Deflection: ");
        Serial.println(sensorData.deflection, 4);
    }
    
    if (sensorData.hasPressure) {
        Serial.print("Pressure: ");
        Serial.println(sensorData.pressure, 2);
    }
    
    if (sensorData.hasPicTemp) {
        Serial.print("PIC Temperature: ");
        Serial.print(sensorData.picTemp, 2);
        Serial.println(" °C");
    }
    
    // Hier kannst du die Daten weiterverarbeiten:
    // - An Cloud/Server senden
    // - Auf SD-Karte speichern
    // - Über WiFi weiterleiten
    // - Display anzeigen
    // - Alarme triggern
    
    Serial.println("=== ENDE SENSORDATEN ===\n");
}

/**
 * @brief Callback für Timeout-Events
 * Wird aufgerufen, wenn längere Zeit keine Daten empfangen wurden
 */
void onTimeout(unsigned long timeoutMs) {
    Serial.print("TIMEOUT: Keine Daten seit ");
    Serial.print(timeoutMs / 1000);
    Serial.println(" Sekunden");
}

/**
 * @brief Callback für Status-Updates
 * Wird periodisch aufgerufen mit aktuellen Statistiken
 */
void onStatusUpdate(uint32_t messages, uint32_t bytes, unsigned long uptime) {
    Serial.print("STATUS: ");
    Serial.print(messages);
    Serial.print(" Nachrichten, ");
    Serial.print(bytes);
    Serial.print(" Bytes, Uptime: ");
    Serial.print(uptime / 1000);
    Serial.println(" s");
}

/**
 * @brief Arduino Setup
 */
void setup() {
    // Serial Monitor initialisieren
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("=== UARTReceiverBinary Beispiel ===");
    
    // UART Receiver initialisieren
    if (!receiver.begin()) {
        Serial.println("FEHLER: Receiver konnte nicht initialisiert werden!");
        while(1) delay(1000);
    }
    
    // Konfiguration
    receiver.setExpectedPayloadSize(24);  // 24-Byte-Pakete erwarten
    receiver.setBinaryCallback(onBinaryDataReceived);
    receiver.setTimeoutCallback(onTimeout);
    receiver.setStatusCallback(onStatusUpdate);
    
    // System-Informationen anzeigen
    receiver.displaySystemInfo();
    
    Serial.println("Bereit für Datenempfang...\n");
}

/**
 * @brief Arduino Hauptschleife
 */
void loop() {
    // Hauptverarbeitung - muss regelmäßig aufgerufen werden
    receiver.process();
    
    // Kleine Pause für bessere Performance
    delay(1);
}
