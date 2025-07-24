/**
 * @file ChirpStackReceiver.cpp
 * @brief Implementierung der ChirpStack MQTT to UART Bridge Receiver Library
 */

#include "ChirpStackReceiver.h"

// Definition der statischen Instanz
ChirpStackReceiver* ChirpStackReceiver::instance = nullptr;

// ========================================
// SensorData Implementierung
// ========================================

float SensorData::getTemperature(uint8_t index) const {
    for (const auto& val : values) {
        if (val.tag == TAG_TEMPERATURE && val.index == index) {
            return val.value;
        }
    }
    return NAN;
}

float SensorData::getDeflection(uint8_t index) const {
    for (const auto& val : values) {
        if (val.tag == TAG_DEFLECTION && val.index == index) {
            return val.value;
        }
    }
    return NAN;
}

float SensorData::getPressure(uint8_t index) const {
    for (const auto& val : values) {
        if (val.tag == TAG_PRESSURE && val.index == index) {
            return val.value;
        }
    }
    return NAN;
}

float SensorData::getMisc(uint8_t index) const {
    for (const auto& val : values) {
        if (val.tag == TAG_MISC && val.index == index) {
            return val.value;
        }
    }
    return NAN;
}

std::vector<float> SensorData::getAllTemperatures() const {
    std::vector<float> temps;
    for (const auto& val : values) {
        if (val.tag == TAG_TEMPERATURE) {
            temps.push_back(val.value);
        }
    }
    return temps;
}

std::vector<float> SensorData::getAllDeflections() const {
    std::vector<float> defls;
    for (const auto& val : values) {
        if (val.tag == TAG_DEFLECTION) {
            defls.push_back(val.value);
        }
    }
    return defls;
}

std::vector<float> SensorData::getAllPressures() const {
    std::vector<float> presses;
    for (const auto& val : values) {
        if (val.tag == TAG_PRESSURE) {
            presses.push_back(val.value);
        }
    }
    return presses;
}

std::vector<float> SensorData::getAllMisc() const {
    std::vector<float> miscs;
    for (const auto& val : values) {
        if (val.tag == TAG_MISC) {
            miscs.push_back(val.value);
        }
    }
    return miscs;
}

void SensorData::clear() {
    deviceId = "";
    values.clear();
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
    
    // Verwende die Payload_Builder Dekodierungsfunktion
    decodePayload(data, size);
    
    // Zeige Hex-Darstellung für Debug
    printPayloadHex(data, size);
    
    SerialMon.println(F("========================\n"));
}

SensorData ChirpStackMessageProcessor::decodeSensorDataToStruct(const uint8_t* data, size_t size, const String& deviceId) {
    SensorData result;
    result.deviceId = deviceId;
    result.rawPayloadSize = size;
    result.lastUpdate = millis();
    
    if (!data || size == 0) {
        return result;
    }
    
    size_t pos = 0;
    uint8_t tempCount = 0, deflCount = 0, pressCount = 0, miscCount = 0;
    
    while (pos < size - 2) { // Mindestens Tag + Length + 1 Byte Daten
        uint8_t tag = data[pos++];
        uint8_t length = data[pos++];
        
        if (pos + length > size) {
            break; // Nicht genug Daten
        }
        
        // Verarbeite je nach Tag
        uint8_t* currentCount = nullptr;
        switch (tag) {
            case TAG_TEMPERATURE:
            case 'T':
                currentCount = &tempCount;
                break;
            case TAG_DEFLECTION:
            case 'D':
                currentCount = &deflCount;
                break;
            case TAG_PRESSURE:
            case 'P':
                currentCount = &pressCount;
                break;
            case TAG_MISC:
            case 'S':
                currentCount = &miscCount;
                break;
            default:
                pos += length; // Überspringe unbekannte Tags
                continue;
        }
        
        // Dekodiere Werte basierend auf der Länge
        if (length == 2) {
            // Ein int16_t Wert
            if (pos + 2 <= size) {
                int16_t rawValue = (data[pos] << 8) | data[pos + 1];
                float value = rawValue / 100.0;
                
                // Normalisiere Tag auf numerischen Wert
                uint8_t normalizedTag = tag;
                if (tag == 'T') normalizedTag = TAG_TEMPERATURE;
                else if (tag == 'D') normalizedTag = TAG_DEFLECTION;
                else if (tag == 'P') normalizedTag = TAG_PRESSURE;
                else if (tag == 'S') normalizedTag = TAG_MISC;
                
                result.values.push_back(SensorValue(normalizedTag, *currentCount, value));
                (*currentCount)++;
            }
        } else if (length == 4) {
            // Zwei int16_t Werte
            for (int i = 0; i < 2 && pos + 2 <= size; i++) {
                int16_t rawValue = (data[pos] << 8) | data[pos + 1];
                float value = rawValue / 100.0;
                pos += 2;
                
                // Normalisiere Tag auf numerischen Wert
                uint8_t normalizedTag = tag;
                if (tag == 'T') normalizedTag = TAG_TEMPERATURE;
                else if (tag == 'D') normalizedTag = TAG_DEFLECTION;
                else if (tag == 'P') normalizedTag = TAG_PRESSURE;
                else if (tag == 'S') normalizedTag = TAG_MISC;
                
                result.values.push_back(SensorValue(normalizedTag, (*currentCount)++, value));
            }
            pos -= 2 * 2; // Zurücksetzen für die nächste Iteration
        } else if (length == 6) {
            // Drei int16_t Werte
            for (int i = 0; i < 3 && pos + 2 <= size; i++) {
                int16_t rawValue = (data[pos] << 8) | data[pos + 1];
                float value = rawValue / 100.0;
                pos += 2;
                
                // Normalisiere Tag auf numerischen Wert
                uint8_t normalizedTag = tag;
                if (tag == 'T') normalizedTag = TAG_TEMPERATURE;
                else if (tag == 'D') normalizedTag = TAG_DEFLECTION;
                else if (tag == 'P') normalizedTag = TAG_PRESSURE;
                else if (tag == 'S') normalizedTag = TAG_MISC;
                
                result.values.push_back(SensorValue(normalizedTag, (*currentCount)++, value));
            }
            pos -= 3 * 2; // Zurücksetzen für die nächste Iteration
        } else if (length == 8) {
            // Vier int16_t Werte
            for (int i = 0; i < 4 && pos + 2 <= size; i++) {
                int16_t rawValue = (data[pos] << 8) | data[pos + 1];
                float value = rawValue / 100.0;
                pos += 2;
                
                // Normalisiere Tag auf numerischen Wert
                uint8_t normalizedTag = tag;
                if (tag == 'T') normalizedTag = TAG_TEMPERATURE;
                else if (tag == 'D') normalizedTag = TAG_DEFLECTION;
                else if (tag == 'P') normalizedTag = TAG_PRESSURE;
                else if (tag == 'S') normalizedTag = TAG_MISC;
                
                result.values.push_back(SensorValue(normalizedTag, (*currentCount)++, value));
            }
            pos -= 4 * 2; // Zurücksetzen für die nächste Iteration
        }
        
        pos += length;
    }
    
    return result;
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
    for (const auto& val : lastSensorData.values) {
        if (val.tag == TAG_TEMPERATURE) {
            JsonObject temp = temps.createNestedObject();
            temp["index"] = val.index;
            temp["value"] = val.value;
            temp["timestamp"] = val.timestamp;
        }
    }
    
    JsonArray defls = doc.createNestedArray("deflections");
    for (const auto& val : lastSensorData.values) {
        if (val.tag == TAG_DEFLECTION) {
            JsonObject defl = defls.createNestedObject();
            defl["index"] = val.index;
            defl["value"] = val.value;
            defl["timestamp"] = val.timestamp;
        }
    }
    
    JsonArray presses = doc.createNestedArray("pressures");
    for (const auto& val : lastSensorData.values) {
        if (val.tag == TAG_PRESSURE) {
            JsonObject press = presses.createNestedObject();
            press["index"] = val.index;
            press["value"] = val.value;
            press["timestamp"] = val.timestamp;
        }
    }
    
    JsonArray miscs = doc.createNestedArray("misc");
    for (const auto& val : lastSensorData.values) {
        if (val.tag == TAG_MISC) {
            JsonObject misc = miscs.createNestedObject();
            misc["index"] = val.index;
            misc["value"] = val.value;
            misc["timestamp"] = val.timestamp;
        }
    }
    
    String result;
    serializeJson(doc, result);
    return result;
}
