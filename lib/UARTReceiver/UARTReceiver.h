/**
 * @file UARTReceiver.h
 * @brief UART-Empfänger Library für AVR-IoT Cellular Mini Board
 * @author Basierend auf UART-Empfänger Hauptprogramm
 * @date 2025-07-15
 * 
 * Diese Library kapselt die UART-Empfangsfunktionalität in eine wiederverwendbare Klasse.
 * Sie unterstützt sowohl JSON- als auch Textdaten-Verarbeitung.
 */

#ifndef UARTRECEIVER_H
#define UARTRECEIVER_H

#include "Arduino.h"
#include <ArduinoJson.h>

// Standardkonfiguration - kann überschrieben werden
#ifndef UART_BUFFER_SIZE
#define UART_BUFFER_SIZE 2048
#endif

#ifndef UART_TIMEOUT_MS
#define UART_TIMEOUT_MS 10000  // 10 Sekunden
#endif

#ifndef UART_STATUS_UPDATE_MS
#define UART_STATUS_UPDATE_MS 30000  // 30 Sekunden
#endif

#ifndef UART_HEARTBEAT_INTERVAL
#define UART_HEARTBEAT_INTERVAL 30000  // 30 Sekunden
#endif

#ifndef MAX_PAYLOAD_SIZE
#define MAX_PAYLOAD_SIZE 256  // Maximale Payload-Größe
#endif

#ifndef PAYLOAD_TIMEOUT
#define PAYLOAD_TIMEOUT 5000  // 5 Sekunden Timeout für unvollständige Payloads
#endif

/**
 * @brief Callback-Funktionstypen für verschiedene Events
 */
typedef void (*UARTMessageCallback)(const String& message);
typedef void (*UARTJSONCallback)(JsonObject data);
typedef void (*UARTTextCallback)(const String& text);
typedef void (*UARTTimeoutCallback)(unsigned long timeoutMs);
typedef void (*UARTStatusCallback)(uint32_t messages, uint32_t bytes, unsigned long uptime);
typedef void (*UARTBinaryCallback)(const uint8_t* data, size_t size, const char* deviceId = nullptr);

/**
 * @brief UART-Empfänger Klasse
 * 
 * Diese Klasse bietet eine einfache Schnittstelle zum Empfangen und Verarbeiten
 * von UART-Daten mit automatischer JSON-Erkennung und Callback-System.
 */
class UARTReceiver {
private:
    // Hardware-Konfiguration
    HardwareSerial* _serial;
    Stream* _debugSerial;
    int _txPin;
    int _rxPin;
    uint32_t _baudrate;
    int _ledPin;
    
    // Pufferung und Timing
    String _inputBuffer;
    unsigned long _lastDataReceived;
    unsigned long _lastStatusUpdate;
    unsigned long _lastTimeoutMessage;
    unsigned long _lastHeartbeat;
    
    // Binärdaten-Puffer
    uint8_t _binaryBuffer[MAX_PAYLOAD_SIZE];
    size_t _bufferIndex;
    size_t _expectedPayloadSize;
    unsigned long _lastBinaryDataReceived;
    
    // Statistiken
    uint32_t _totalMessagesReceived;
    uint32_t _totalBytesReceived;
    bool _dataReceivedSinceLastCheck;
    
    // Status-Flags
    bool _initialized;
    bool _systemReady;
    bool _binaryMode;
    
    // Callback-Funktionen
    UARTMessageCallback _messageCallback;
    UARTJSONCallback _jsonCallback;
    UARTTextCallback _textCallback;
    UARTTimeoutCallback _timeoutCallback;
    UARTStatusCallback _statusCallback;
    UARTBinaryCallback _binaryCallback;
    
    // Konfiguration
    size_t _bufferSize;
    unsigned long _timeoutMs;
    unsigned long _statusUpdateMs;
    unsigned long _heartbeatInterval;
    
    // Private Hilfsfunktionen
    void processMessage(const String& message);
    void processUplinkData(JsonObject data);
    void checkDataTimeout();
    void displayPeriodicStatus();
    void sendHeartbeat();
    
    // Binärdaten-Hilfsfunktionen
    void processBinaryPayload(const uint8_t* payload, size_t size, const char* deviceId = nullptr);
    float readFloat(const uint8_t* data, size_t startIndex);
    void checkPayloadTimeout();
    
public:
    /**
     * @brief Konstruktor der UARTReceiver Klasse
     * @param serial Hardware-Serial-Schnittstelle (z.B. Serial2)
     * @param debugSerial Debug-Ausgabe-Schnittstelle (z.B. Serial)
     * @param txPin TX-Pin für UART
     * @param rxPin RX-Pin für UART
     * @param baudrate Baudrate für UART-Kommunikation
     * @param ledPin LED-Pin für Statusanzeige (optional, -1 für keine LED)
     */
    UARTReceiver(HardwareSerial* serial, Stream* debugSerial, 
                 int txPin, int rxPin, uint32_t baudrate, int ledPin = -1);
    
    /**
     * @brief Initialisiert die UART-Empfänger-Hardware
     * @return true wenn Initialisierung erfolgreich
     */
    bool begin();
    
    /**
     * @brief Hauptverarbeitungsschleife - sollte in loop() aufgerufen werden
     */
    void process();
    
    /**
     * @brief Beendet die UART-Verbindung
     */
    void end();
    
    /**
     * @brief Setzt Callback-Funktion für alle empfangenen Nachrichten
     * @param callback Callback-Funktion
     */
    void setMessageCallback(UARTMessageCallback callback);
    
    /**
     * @brief Setzt Callback-Funktion für JSON-Daten
     * @param callback Callback-Funktion
     */
    void setJSONCallback(UARTJSONCallback callback);
    
    /**
     * @brief Setzt Callback-Funktion für Text-Daten
     * @param callback Callback-Funktion
     */
    void setTextCallback(UARTTextCallback callback);
    
    /**
     * @brief Setzt Callback-Funktion für Timeout-Events
     * @param callback Callback-Funktion
     */
    void setTimeoutCallback(UARTTimeoutCallback callback);
    
    /**
     * @brief Setzt Callback-Funktion für Status-Updates
     * @param callback Callback-Funktion
     */
    void setStatusCallback(UARTStatusCallback callback);
    
    /**
     * @brief Setzt Callback-Funktion für Binärdaten
     * @param callback Callback-Funktion
     */
    void setBinaryCallback(UARTBinaryCallback callback);
    
    /**
     * @brief Konfiguriert die Puffergröße
     * @param size Puffergröße in Bytes
     */
    void setBufferSize(size_t size);
    
    /**
     * @brief Konfiguriert das Timeout für Datenempfang
     * @param timeoutMs Timeout in Millisekunden
     */
    void setTimeout(unsigned long timeoutMs);
    
    /**
     * @brief Konfiguriert das Intervall für Status-Updates
     * @param intervalMs Intervall in Millisekunden
     */
    void setStatusUpdateInterval(unsigned long intervalMs);
    
    /**
     * @brief Konfiguriert das Heartbeat-Intervall
     * @param intervalMs Intervall in Millisekunden
     */
    void setHeartbeatInterval(unsigned long intervalMs);
    
    /**
     * @brief Konfiguriert die erwartete Binärdaten-Payload-Größe
     * @param size Erwartete Payload-Größe in Bytes
     */
    void setExpectedPayloadSize(size_t size);
    
    /**
     * @brief Aktiviert/Deaktiviert den Binärdaten-Modus
     * @param enabled true für Binärdaten-Modus, false für Text-Modus
     */
    void setBinaryMode(bool enabled);
    
    /**
     * @brief Testet verschiedene Baudraten
     */
    void testBaudrates();
    
    /**
     * @brief Gibt aktuelle Statistiken zurück
     * @param messages Anzahl empfangener Nachrichten (out)
     * @param bytes Anzahl empfangener Bytes (out)
     * @param uptime Uptime in Millisekunden (out)
     */
    void getStatistics(uint32_t& messages, uint32_t& bytes, unsigned long& uptime);
    
    /**
     * @brief Prüft ob System bereit ist
     * @return true wenn System bereit
     */
    bool isReady() const;
    
    /**
     * @brief Zeigt Systeminformationen an
     */
    void displaySystemInfo();
    
    /**
     * @brief Sendet Testdaten über UART
     * @param testData Testdaten zum Senden
     */
    void sendTestData(const String& testData);
    
    /**
     * @brief Leert den Eingangspuffer
     */
    void clearBuffer();
};

#endif // UARTRECEIVER_H
