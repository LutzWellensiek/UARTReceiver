/**
 * @file main.cpp
 * @brief Beispiel für die Verwendung des UARTReceiver im Binärdaten-Modus
 * @author UARTReceiver Library
 * @date 2025-07-15
 */

#include <Arduino.h>
#include "UARTReceiver.h"

// Hardware-Konfiguration
#define UART_TX_PIN 14  // GPIO 14 (D5)
#define UART_RX_PIN 12  // GPIO 12 (D6)
#define UART_BAUDRATE 9600
#define LED_PIN 2       // GPIO 2 (interne LED)

// UARTReceiver-Instanz
UARTReceiver uartReceiver(&Serial2, &Serial, UART_TX_PIN, UART_RX_PIN, UART_BAUDRATE, LED_PIN);

/**
 * @brief Callback-Funktion für empfangene Binärdaten
 */
void onBinaryDataReceived(const uint8_t* data, size_t size) {
    Serial.println("=== BINÄRDATEN EMPFANGEN UND VERARBEITET! ===");
    
    // Hier kannst du die empfangenen Binärdaten weiterverarbeiten
    // Beispiel: Daten an andere Systeme weiterleiten
    // sendToCloud(data, size);
    // saveToSD(data, size);
    // displayOnLCD(data, size);
    
    Serial.print("Verarbeitete Payload-Größe: ");
    Serial.print(size);
    Serial.println(" Bytes");
    
    // Rohes Hex-Dump der empfangenen Daten
    Serial.print("Rohdaten: ");
    for (size_t i = 0; i < size; i++) {
        if (data[i] < 16) Serial.print("0");
        Serial.print(data[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
    Serial.println("=== ENDE BINÄRDATEN-VERARBEITUNG ===\n");
}

/**
 * @brief Setup-Funktion
 */
void setup() {
    Serial.begin(115200);  // USB Debug
    delay(1000);
    
    Serial.println("=== UART RECEIVER BINÄRDATEN-MODUS ===");
    Serial.println("Initialisiere UARTReceiver...");
    
    // UARTReceiver initialisieren
    if (!uartReceiver.begin()) {
        Serial.println("FEHLER: UARTReceiver konnte nicht initialisiert werden!");
        while (1) {
            delay(1000);
        }
    }
    
    // Binärdaten-Modus aktivieren
    uartReceiver.setBinaryMode(true);
    
    // Erwartete Payload-Größe setzen (T+D+P+S = 1+4+1+4+1+4+1+4 = 20 Bytes + 4 Bytes für Overhead = 24 Bytes)
    uartReceiver.setExpectedPayloadSize(24);
    
    // Callback für Binärdaten setzen
    uartReceiver.setBinaryCallback(onBinaryDataReceived);
    
    // Systeminformationen anzeigen
    uartReceiver.displaySystemInfo();
    
    Serial.println("System bereit für Binärdaten-Empfang!");
    Serial.println("Erwartete Payload-Größe: 24 Bytes");
    Serial.println("Format: T<float><float>D<float>P<float>S<float>");
    Serial.println("=====================================\n");
}

/**
 * @brief Hauptschleife
 */
void loop() {
    // Hauptverarbeitung
    uartReceiver.process();
    
    // Kleine Pause für bessere Performance
    delay(1);
}
