/**
 * @file BasicUsage.ino
 * @brief Beispiel für die Verwendung der UARTReceiver Library
 * @author UARTReceiver Library Demo
 * @date 2025-07-15
 * 
 * Dieses Beispiel zeigt die grundlegende Verwendung der UARTReceiver Library.
 * Die Library übernimmt die komplette UART-Kommunikation und JSON-Verarbeitung.
 */

#include <UARTReceiver.h>
#include <UART_Libs/KitConfig.h>
#include <UART_Libs/SerialMon.h>

// LED Pin für Statusanzeige
const int LED_PIN = 13;

// UARTReceiver Instanz erstellen
UARTReceiver uartReceiver(&Serial2, &SerialMon, SerialPIN_TX, SerialPIN_RX, UART2_BAUDRATE, LED_PIN);

// Callback-Funktionen
void onJSONReceived(JsonObject data) {
    SerialMon.println("=== JSON EMPFANGEN ===");
    
    // Beispiel: Uplink-Daten verarbeiten
    if (data["type"] == "uplink_data") {
        JsonObject uplinkData = data["data"];
        
        if (uplinkData.containsKey("dev_eui")) {
            SerialMon.print("Device EUI: ");
            SerialMon.println(uplinkData["dev_eui"].as<String>());
        }
        
        if (uplinkData.containsKey("data_hex")) {
            SerialMon.print("Payload: ");
            SerialMon.println(uplinkData["data_hex"].as<String>());
        }
    }
    
    SerialMon.println("=== ENDE JSON ===");
}

void onTextReceived(const String& text) {
    SerialMon.println("=== TEXT EMPFANGEN ===");
    SerialMon.print("Inhalt: ");
    SerialMon.println(text);
    SerialMon.println("=== ENDE TEXT ===");
}

void onTimeout(unsigned long timeoutMs) {
    SerialMon.print("Timeout: Keine Daten seit ");
    SerialMon.print(timeoutMs / 1000);
    SerialMon.println(" Sekunden");
}

void onStatus(uint32_t messages, uint32_t bytes, unsigned long uptime) {
    SerialMon.print("Status: ");
    SerialMon.print(messages);
    SerialMon.print(" Nachrichten, ");
    SerialMon.print(bytes);
    SerialMon.print(" Bytes, ");
    SerialMon.print(uptime / 1000);
    SerialMon.println(" s Uptime");
}

void setup() {
    // Debug Serial initialisieren
    SerialMon.begin(SERIAL_MON_BAUDRATE);
    delay(100);
    
    SerialMon.println("=== UARTReceiver Demo ===");
    
    // UART Receiver initialisieren
    if (!uartReceiver.begin()) {
        SerialMon.println("Fehler beim Initialisieren des UART Receivers!");
        while (true) {
            delay(1000);
        }
    }
    
    // Callbacks registrieren
    uartReceiver.setJSONCallback(onJSONReceived);
    uartReceiver.setTextCallback(onTextReceived);
    uartReceiver.setTimeoutCallback(onTimeout);
    uartReceiver.setStatusCallback(onStatus);
    
    // Optional: Konfiguration anpassen
    uartReceiver.setTimeout(15000);  // 15 Sekunden Timeout
    uartReceiver.setStatusUpdateInterval(20000);  // Status alle 20 Sekunden
    
    // System-Informationen anzeigen
    uartReceiver.displaySystemInfo();
    
    SerialMon.println("Demo bereit - sende Daten über UART2");
}

void loop() {
    // Hauptverarbeitungsschleife der Library
    uartReceiver.process();
    
    // Optional: Hier können weitere Aufgaben ausgeführt werden
    
    // Kurze Pause
    delay(1);
}

// Zusätzliche Funktionen für erweiterte Verwendung
void sendTestData() {
    // Testdaten senden
    uartReceiver.sendTestData("{\"type\":\"test\",\"data\":\"Hello World\"}");
}

void testDifferentBaudrates() {
    // Verschiedene Baudraten testen
    uartReceiver.testBaudrates();
}

void printStatistics() {
    // Aktuelle Statistiken abrufen
    uint32_t messages, bytes;
    unsigned long uptime;
    uartReceiver.getStatistics(messages, bytes, uptime);
    
    SerialMon.print("Statistiken: ");
    SerialMon.print(messages);
    SerialMon.print(" Nachrichten, ");
    SerialMon.print(bytes);
    SerialMon.print(" Bytes, ");
    SerialMon.print(uptime / 1000);
    SerialMon.println(" s Uptime");
}
