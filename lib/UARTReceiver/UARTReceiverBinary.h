/**
 * @file UARTReceiverBinary.h
 * @brief UART-Empfänger Library für Binärdaten-Verarbeitung
 * @author UARTReceiver Library
 * @version 2.0
 * @date 2025-07-15
 * 
 * ============================================================================
 * UART RECEIVER BINARY LIBRARY - ANLEITUNG
 * ============================================================================
 * 
 * Diese Library ermöglicht den einfachen Empfang und die Dekodierung von
 * strukturierten Binärdaten über UART. Sie wurde speziell für Sensordaten
 * entwickelt und unterstützt verschiedene Sensor-Typen.
 * 
 * UNTERSTÜTZTE SENSOR-TYPEN:
 * - 'T': Temperatur (2 Float-Werte, 9 Bytes total)
 * - 'D': Deflection (1 Float-Wert, 5 Bytes total)
 * - 'P': Pressure (1 Float-Wert, 5 Bytes total)
 * - 'S': PIC Temperature (1 Float-Wert, 5 Bytes total)
 * 
 * PAYLOAD-FORMAT:
 * T<float><float>D<float>P<float>S<float> = 24 Bytes
 * 
 * VERWENDUNG:
 * 
 * 1. LIBRARY EINBINDEN:
 *    #include "UARTReceiverBinary.h"
 * 
 * 2. INSTANZ ERSTELLEN:
 *    UARTReceiverBinary receiver(&Serial2, &Serial, TX_PIN, RX_PIN, BAUDRATE);
 * 
 * 3. INITIALISIERUNG:
 *    receiver.begin();
 *    receiver.setExpectedPayloadSize(24);
 *    receiver.setBinaryCallback(onDataReceived);
 * 
 * 4. CALLBACK DEFINIEREN:
 *    void onDataReceived(const uint8_t* data, size_t size) {
 *        // Daten verarbeiten
 *    }
 * 
 * 5. HAUPTSCHLEIFE:
 *    void loop() {
 *        receiver.process();
 *    }
 * 
 * ERWEITERTE FUNKTIONEN:
 * - Automatische Sensordaten-Dekodierung
 * - Timeout-Handling für unvollständige Pakete
 * - Debug-Ausgaben mit Hex-Dumps
 * - Statistiken und Status-Callbacks
 * - Pufferüberlauf-Schutz
 * 
 * BEISPIEL-PROJEKT:
 * Siehe examples/SimpleBinaryReceiver/main.cpp
 * 
 * ============================================================================
 */

#ifndef UARTRECEIVER_BINARY_H
#define UARTRECEIVER_BINARY_H

#include "Arduino.h"

// Konfiguration - kann überschrieben werden
#ifndef MAX_PAYLOAD_SIZE
#define MAX_PAYLOAD_SIZE 256
#endif

#ifndef PAYLOAD_TIMEOUT
#define PAYLOAD_TIMEOUT 5000  // 5 Sekunden
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

/**
 * @brief Callback-Funktionstypen
 */
typedef void (*BinaryDataCallback)(const uint8_t* data, size_t size);
typedef void (*TimeoutCallback)(unsigned long timeoutMs);
typedef void (*StatusCallback)(uint32_t messages, uint32_t bytes, unsigned long uptime);

/**
 * @brief Struktur für dekodierte Sensordaten
 */
struct SensorData {
    bool hasTemperature = false;
    float temperature1 = 0.0f;
    float temperature2 = 0.0f;
    
    bool hasDeflection = false;
    float deflection = 0.0f;
    
    bool hasPressure = false;
    float pressure = 0.0f;
    
    bool hasPicTemp = false;
    float picTemp = 0.0f;
    
    unsigned long timestamp = 0;
};

/**
 * @brief UART-Empfänger für Binärdaten
 */
class UARTReceiverBinary {
private:
    // Hardware-Konfiguration
    HardwareSerial* _serial;
    Stream* _debugSerial;
    int _txPin;
    int _rxPin;
    uint32_t _baudrate;
    int _ledPin;
    
    // Binärdaten-Puffer
    uint8_t _binaryBuffer[MAX_PAYLOAD_SIZE];
    size_t _bufferIndex;
    size_t _expectedPayloadSize;
    
    // Timing
    unsigned long _lastDataReceived;
    unsigned long _lastStatusUpdate;
    unsigned long _lastHeartbeat;
    
    // Statistiken
    uint32_t _totalMessagesReceived;
    uint32_t _totalBytesReceived;
    bool _dataReceivedSinceLastCheck;
    
    // Status
    bool _initialized;
    bool _systemReady;
    
    // Callbacks
    BinaryDataCallback _binaryCallback;
    TimeoutCallback _timeoutCallback;
    StatusCallback _statusCallback;
    
    // Private Methoden
    void processBinaryPayload(const uint8_t* payload, size_t size);
    float readFloat(const uint8_t* data, size_t startIndex);
    void checkPayloadTimeout();
    void checkDataTimeout();
    void displayPeriodicStatus();
    void sendHeartbeat();
    
public:
    /**
     * @brief Konstruktor
     * @param serial Hardware-Serial-Schnittstelle
     * @param debugSerial Debug-Ausgabe (optional, kann nullptr sein)
     * @param txPin TX-Pin für UART
     * @param rxPin RX-Pin für UART
     * @param baudrate Baudrate für UART
     * @param ledPin LED-Pin für Status (optional, -1 für keine LED)
     */
    UARTReceiverBinary(HardwareSerial* serial, Stream* debugSerial, 
                      int txPin, int rxPin, uint32_t baudrate, int ledPin = -1);
    
    /**
     * @brief Initialisiert die Hardware
     * @return true wenn erfolgreich
     */
    bool begin();
    
    /**
     * @brief Hauptverarbeitungsschleife
     * Sollte regelmäßig in loop() aufgerufen werden
     */
    void process();
    
    /**
     * @brief Beendet die Verbindung
     */
    void end();
    
    /**
     * @brief Setzt die erwartete Payload-Größe
     * @param size Erwartete Größe in Bytes
     */
    void setExpectedPayloadSize(size_t size);
    
    /**
     * @brief Setzt Callback für Binärdaten
     * @param callback Callback-Funktion
     */
    void setBinaryCallback(BinaryDataCallback callback);
    
    /**
     * @brief Setzt Callback für Timeout-Events
     * @param callback Callback-Funktion
     */
    void setTimeoutCallback(TimeoutCallback callback);
    
    /**
     * @brief Setzt Callback für Status-Updates
     * @param callback Callback-Funktion
     */
    void setStatusCallback(StatusCallback callback);
    
    /**
     * @brief Dekodiert Sensordaten aus Binärdaten
     * @param data Binärdaten
     * @param size Größe der Daten
     * @return Dekodierte Sensordaten
     */
    SensorData decodeSensorData(const uint8_t* data, size_t size);
    
    /**
     * @brief Gibt aktuelle Statistiken zurück
     * @param messages Anzahl empfangener Nachrichten (out)
     * @param bytes Anzahl empfangener Bytes (out)
     * @param uptime Uptime in Millisekunden (out)
     */
    void getStatistics(uint32_t& messages, uint32_t& bytes, unsigned long& uptime);
    
    /**
     * @brief Prüft ob System bereit ist
     * @return true wenn bereit
     */
    bool isReady() const;
    
    /**
     * @brief Leert den Puffer
     */
    void clearBuffer();
    
    /**
     * @brief Zeigt Systeminformationen an
     */
    void displaySystemInfo();
    
    /**
     * @brief Sendet Testdaten
     * @param data Testdaten zum Senden
     * @param size Größe der Testdaten
     */
    void sendTestData(const uint8_t* data, size_t size);
};

#endif // UARTRECEIVER_BINARY_H
