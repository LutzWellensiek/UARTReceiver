/**
 * @file ChirpStackReceiver.cpp
 * @brief Implementierung der ChirpStack MQTT to UART Bridge Receiver Library
 */

#include "ChirpStackReceiver.h"
#include "../../SerialMon.h"
#include "../Payload_Builder_Cayenne.h"
// Definition der statischen Instanz
ChirpStackReceiver* ChirpStackReceiver::instance = nullptr;

// ========================================
// SensorData Implementierung
// ========================================

float SensorData::getTemperature(uint8_t index) const {
    for (size_t i = 0; i < valueCount; i++) {
        if (values[i].tag == TAG_TEMPERATURE && values[i].index == index) {
            return values[i].value;
        }
    }
    return NAN;
}

float SensorData::getDeflection(uint8_t index) const {
    for (size_t i = 0; i < valueCount; i++) {
        if (values[i].tag == TAG_DEFLECTION && values[i].index == index) {
            return values[i].value;
        }
    }
    return NAN;
}

float SensorData::getPressure(uint8_t index) const {
    for (size_t i = 0; i < valueCount; i++) {
        if (values[i].tag == TAG_PRESSURE && values[i].index == index) {
            return values[i].value;
        }
    }
    return NAN;
}

float SensorData::getMisc(uint8_t index) const {
    for (size_t i = 0; i < valueCount; i++) {
        if (values[i].tag == TAG_MISC && values[i].index == index) {
            return values[i].value;
        }
    }
    return NAN;
}

size_t SensorData::getAllTemperatures(float* temps, size_t maxCount) const {
    size_t count = 0;
    for (size_t i = 0; i < valueCount && count < maxCount; i++) {
        if (values[i].tag == TAG_TEMPERATURE) {
            temps[count++] = values[i].value;
        }
    }
    return count;
}

size_t SensorData::getAllDeflections(float* defls, size_t maxCount) const {
    size_t count = 0;
    for (size_t i = 0; i < valueCount && count < maxCount; i++) {
        if (values[i].tag == TAG_DEFLECTION) {
            defls[count++] = values[i].value;
        }
    }
    return count;
}

size_t SensorData::getAllPressures(float* pressures, size_t maxCount) const {
    size_t count = 0;
    for (size_t i = 0; i < valueCount && count < maxCount; i++) {
        if (values[i].tag == TAG_PRESSURE) {
            pressures[count++] = values[i].value;
        }
    }
    return count;
}

size_t SensorData::getAllMisc(float* misc, size_t maxCount) const {
    size_t count = 0;
    for (size_t i = 0; i < valueCount && count < maxCount; i++) {
        if (values[i].tag == TAG_MISC) {
            misc[count++] = values[i].value;
        }
    }
    return count;
}

void SensorData::clear() {
    deviceId = "";
    valueCount = 0;
    lastUpdate = 0;
    rawPayloadSize = 0;
}

/**
 * @brief Konstruktor
 */
ChirpStackReceiver::ChirpStackReceiver(HardwareSerial* serial, Stream* debugSerial,
                                       int txPin, int rxPin, uint32_t baudrate, int ledPin)
    : uartReceiver(serial, debugSerial, txPin, rxPin, baudrate, ledPin),
      debugSerial(debugSerial),
      debugMode(false),
      statsIntervalMs(ChirpStackConfig::DEFAULT_STATS_INTERVAL_MS) {
    instance = this;
}

bool ChirpStackReceiver::begin(bool dbgMode) {
    debugMode = dbgMode;
    stats.begin(debugSerial);
    initializeSerial();
    initializeUART();
    if (debugSerial) {
        displayWelcome();
    }
    return true;
}

void ChirpStackReceiver::initializeSerial() {
    if (debugSerial) {
        // debugSerial should already be initialized before passing to constructor
        delay(ChirpStackConfig::SERIAL_INIT_DELAY_MS);
    }
}

void ChirpStackReceiver::initializeUART() {
    uartReceiver.setBinaryCallback(staticOnBinaryData);
    uartReceiver.setJSONCallback(staticOnJsonData);
    uartReceiver.setTimeoutCallback(staticOnTimeout);
    uartReceiver.setStatusCallback(staticOnStatus);
    uartReceiver.setBinaryMode(true);
}

void ChirpStackReceiver::displayWelcome() {
    if (debugSerial) {
        debugSerial->println(F("ChirpStack MQTT-to-UART Bridge Receiver"));
    }
}

void ChirpStackReceiver::process() {
    uartReceiver.process();
    if (stats.shouldDisplay()) {
        stats.display();
    }
}

void ChirpStackReceiver::setStatsInterval(uint32_t intervalMs) {
    statsIntervalMs = intervalMs;
}

void ChirpStackReceiver::setDebugMode(bool enabled) {
    debugMode = enabled;
}

void ChirpStackReceiver::displayStats() {
    stats.display();
}

// Callbacks Implementierung
void ChirpStackReceiver::staticOnBinaryData(const uint8_t* data, size_t size) {
    if (instance) {
        instance->onBinaryData(data, size);
    }
}

void ChirpStackReceiver::staticOnJsonData(JsonObject data) {
    if (instance) {
        instance->onJsonData(data);
    }
}

void ChirpStackReceiver::staticOnTimeout(unsigned long timeout) {
    if (instance) {
        instance->onTimeout(timeout);
    }
}

void ChirpStackReceiver::staticOnStatus(uint32_t messages, uint32_t bytes, unsigned long uptime) {
    if (instance) {
        instance->onStatus(messages, bytes, uptime);
    }
}

void ChirpStackReceiver::onBinaryData(const uint8_t* data, size_t size) {
    // Debug: Zeige empfangene Rohdaten
    if (debugSerial) {
        debugSerial->print(F("[DEBUG] Empfangen: "));
        debugSerial->print(size);
        debugSerial->print(F(" Bytes: "));
        for (size_t i = 0; i < size && i < 32; i++) {
            if (data[i] < 0x10) debugSerial->print("0");
            debugSerial->print(data[i], HEX);
            debugSerial->print(" ");
        }
        if (size > 32) debugSerial->print("...");
        debugSerial->println();
    }
    
    // Prüfe ob Device-ID vorhanden ist
    String deviceId = "";
    const uint8_t* payloadData = data;
    size_t payloadSize = size;
    
    // Extrahiere Device-ID wenn vorhanden (16 Hex-Zeichen + ": ")
    if (size > 18) {
        bool hasDeviceId = true;
        for (int i = 0; i < 16; i++) {
            if (!isxdigit(data[i])) {
                hasDeviceId = false;
                break;
            }
        }
        if (hasDeviceId && data[16] == ':' && data[17] == ' ') {
            char devId[17];
            memcpy(devId, data, 16);
            devId[16] = '\0';
            deviceId = String(devId);
            payloadData = data + 18;
            payloadSize = size - 18;
        }
    }
    
    // Dekodiere und speichere die Sensordaten
    lastSensorData = ChirpStackMessageProcessor::decodeSensorDataToStruct(payloadData, payloadSize, deviceId);
    
    // Zeige die Daten an (wie bisher)
    ChirpStackMessageProcessor::decodeSensorData(data, size);
    stats.recordProcessed();
}

void ChirpStackReceiver::onJsonData(JsonObject data) {
    if (debugSerial) {
        serializeJson(data, *debugSerial);
        debugSerial->println();
    }
}

void ChirpStackReceiver::onTimeout(unsigned long timeout) {
    if (debugSerial) {
        debugSerial->print(F("Timeout: "));
        debugSerial->println(timeout);
    }
}

void ChirpStackReceiver::onStatus(uint32_t messages, uint32_t bytes, unsigned long uptime) {
    if (debugSerial) {
        debugSerial->print(F("Status - Messages: "));
        debugSerial->print(messages);
        debugSerial->print(F(", Bytes: "));
        debugSerial->print(bytes);
        debugSerial->print(F(", Uptime: "));
        debugSerial->println(uptime);
    }
}

// Implementierung der Statistikmethoden
void ChirpStackStatistics::recordReceived() {
    messagesReceived++;
}

void ChirpStackStatistics::recordProcessed() {
    messagesProcessed++;
}

void ChirpStackStatistics::recordError() {
    parseErrors++;
}

void ChirpStackStatistics::begin(Stream* debug) {
    debugSerial = debug;
    startTime = millis();
    lastDisplayTime = startTime;
}

bool ChirpStackStatistics::shouldDisplay() {
    return (millis() - lastDisplayTime) > ChirpStackConfig::DEFAULT_STATS_INTERVAL_MS;
}

void ChirpStackStatistics::display() {
    unsigned long uptime = millis() - startTime;
    unsigned long uptimeSeconds = uptime / 1000;

    if (debugSerial) {
        debugSerial->println(F("--- STATS ---"));
        debugSerial->print(F("Uptime: "));
        printUptime(uptimeSeconds);
        debugSerial->print(F("Received: "));
        debugSerial->println(messagesReceived);
        debugSerial->print(F("Processed: "));
        debugSerial->println(messagesProcessed);
        debugSerial->print(F("Errors: "));
        debugSerial->println(parseErrors);
        lastDisplayTime = millis();
    }
}

void ChirpStackStatistics::printUptime(unsigned long seconds) {
    unsigned long hours = seconds / 3600;
    unsigned long minutes = (seconds % 3600) / 60;
    unsigned long secs = seconds % 60;
    if (debugSerial) {
        debugSerial->print(hours);
        debugSerial->print(F("h "));
        debugSerial->print(minutes);
        debugSerial->print(F("m "));
        debugSerial->print(secs);
        debugSerial->println(F("s"));
    }
}

size_t ChirpStackMessageProcessor::hexToBytes(const String& hex, uint8_t* buffer, size_t maxSize) {
    const size_t hexLen = hex.length();
    if (hexLen % 2 != 0) {
        return 0;
    }
    const size_t byteCount = hexLen / 2;
    if (byteCount > maxSize) {
        return 0;
    }
    for (size_t i = 0; i < byteCount; i++) {
        const String byteStr = hex.substring(i * 2, i * 2 + 2);
        buffer[i] = static_cast<uint8_t>(strtol(byteStr.c_str(), nullptr, 16));
    }
    return byteCount;
}

void ChirpStackMessageProcessor::decodeSensorData(const uint8_t* data, size_t size) {
    if (!data || size == 0) {
        SerialMon.println(F("[ERROR] Keine Daten zum Dekodieren"));
        return;
    }
    
    // Debug: Zeige empfangene Rohdaten
    SerialMon.println(F("\n=== SENSOR-DEKODIERUNG ==="));
    SerialMon.print(F("Empfangene Bytes: "));
    SerialMon.println(size);
    
    // Verwende die CayenneLPP Dekodierungsfunktion
    Payload_Builder_Cayenne::decodePayload(data, size);
    
    // Zeige Hex-Darstellung für Debug
    Payload_Builder_Cayenne::printPayloadHex(data, size);
    
    SerialMon.println(F("========================\n"));
}

SensorData ChirpStackMessageProcessor::decodeSensorDataToStruct(const uint8_t* data, size_t size, const String& deviceId) {
    // Verwende die CayenneLPP Dekodierungsfunktion
    return Payload_Builder_Cayenne::decodeCayenneToSensorData(data, size, deviceId);
}

ChirpStackMessageProcessor::DeviceInfo ChirpStackMessageProcessor::extractDeviceInfo(const uint8_t* data, size_t size) {
    ChirpStackMessageProcessor::DeviceInfo info = {"", nullptr, 0, false};
    
    if (!data || size < 4) {
        return info; // Zu wenig Daten
    }
    
    // Suche nach dem Null-Terminator, der den Device-Namen beendet
    size_t nameEnd = 0;
    for (size_t i = 0; i < size && i < 64; i++) { // Max 64 Zeichen für Device-Name
        if (data[i] == 0) {
            nameEnd = i;
            break;
        }
    }
    
    if (nameEnd > 0 && nameEnd < size - 1) {
        // Device-Name gefunden
        char deviceName[65];
        memcpy(deviceName, data, nameEnd);
        deviceName[nameEnd] = '\0';
        
        info.deviceName = String(deviceName);
        info.payloadStart = data + nameEnd + 1;
        info.payloadSize = size - nameEnd - 1;
        info.valid = true;
    }
    
    return info;
}

void ChirpStackMessageProcessor::processTextMessage(const String& message, ChirpStackStatistics& stats) {
    // JSON-Parsing für Text-Nachrichten
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, message);
    
    if (error) {
        SerialMon.print(F("[ERROR] JSON-Parsing fehlgeschlagen: "));
        SerialMon.println(error.c_str());
        stats.recordError();
        return;
    }
    
    // Extrahiere relevante Felder
    const char* deviceName = doc["deviceName"] | "unknown";
    const char* payload = doc["data"];
    
    SerialMon.println(F("\n=== TEXT-NACHRICHT ==="));
    SerialMon.print(F("Device: "));
    SerialMon.println(deviceName);
    
    if (payload) {
        // Konvertiere Hex-String zu Bytes
        uint8_t buffer[ChirpStackConfig::PAYLOAD_BUFFER_SIZE];
        size_t byteCount = hexToBytes(String(payload), buffer, sizeof(buffer));
        
        if (byteCount > 0) {
            SerialMon.print(F("Payload-Größe: "));
            SerialMon.print(byteCount);
            SerialMon.println(F(" Bytes"));
            
            // Dekodiere die Sensordaten
            decodeSensorData(buffer, byteCount);
            stats.recordProcessed();
        } else {
            SerialMon.println(F("[ERROR] Hex-Konvertierung fehlgeschlagen"));
            stats.recordError();
        }
    }
    
    SerialMon.println(F("======================\n"));
}

String ChirpStackReceiver::getSensorDataAsJson() const {
    StaticJsonDocument<1024> doc;
    
    doc["deviceId"] = lastSensorData.deviceId;
    doc["timestamp"] = lastSensorData.lastUpdate;
    doc["payloadSize"] = lastSensorData.rawPayloadSize;
    
    JsonArray temps = doc.createNestedArray("temperatures");
    for (size_t i = 0; i < lastSensorData.valueCount; i++) {
        if (lastSensorData.values[i].tag == TAG_TEMPERATURE) {
            JsonObject temp = temps.createNestedObject();
            temp["index"] = lastSensorData.values[i].index;
            temp["value"] = lastSensorData.values[i].value;
            temp["timestamp"] = lastSensorData.values[i].timestamp;
        }
    }
    
    JsonArray defls = doc.createNestedArray("deflections");
    for (size_t i = 0; i < lastSensorData.valueCount; i++) {
        if (lastSensorData.values[i].tag == TAG_DEFLECTION) {
            JsonObject defl = defls.createNestedObject();
            defl["index"] = lastSensorData.values[i].index;
            defl["value"] = lastSensorData.values[i].value;
            defl["timestamp"] = lastSensorData.values[i].timestamp;
        }
    }
    
    JsonArray presses = doc.createNestedArray("pressures");
    for (size_t i = 0; i < lastSensorData.valueCount; i++) {
        if (lastSensorData.values[i].tag == TAG_PRESSURE) {
            JsonObject press = presses.createNestedObject();
            press["index"] = lastSensorData.values[i].index;
            press["value"] = lastSensorData.values[i].value;
            press["timestamp"] = lastSensorData.values[i].timestamp;
        }
    }
    
    JsonArray miscs = doc.createNestedArray("misc");
    for (size_t i = 0; i < lastSensorData.valueCount; i++) {
        if (lastSensorData.values[i].tag == TAG_MISC) {
            JsonObject misc = miscs.createNestedObject();
            misc["index"] = lastSensorData.values[i].index;
            misc["value"] = lastSensorData.values[i].value;
            misc["timestamp"] = lastSensorData.values[i].timestamp;
        }
    }
    
    String result;
    serializeJson(doc, result);
    return result;
}
