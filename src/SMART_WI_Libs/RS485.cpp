#if defined (DXCORE)
#include "RS485.h"
#include <util/atomic.h>
#include "CRC8.h"  // For checksum calculation
#include "SerialMon.h"  // For consistent serial output

// Default baudrate if not defined elsewhere
#ifndef RS485_BAUDRATE
#define RS485_BAUDRATE 115200
#endif

// Default max ports if not defined elsewhere
#ifndef RS485_MAX_PORTS
#define RS485_MAX_PORTS 1
#endif

RS485Class::RS485Class(int txPin, int rxPin, uint8_t nrReceiver)
{
    _txPin = txPin;
    _rxPin = rxPin;
    _currentReceiver = 2;  // Start with receiver 2 for testing
    Serial2.pins(_txPin,_rxPin);
    setNumOfReceiver(nrReceiver);
}

RS485Class::~RS485Class()
{
}

void RS485Class::begin(uint32_t baudRate){
    Serial2.begin(baudRate, (SERIAL_8N1 | SERIAL_RS485 | SERIAL_RX_ONLY)); //SERIAL_8N1 || SERIAL_RS485 || SERIAL_RX_ONLY

    /*Set the Pins for controlling the 3_to_1_RS485_Multiplexer */
    if (getNumOfReceiver() > 1)
    {
        pinMode(RS485_SELECT_CH1_PIN, OUTPUT);
        pinMode(RS485_SELECT_CH2_PIN, OUTPUT);
        pinMode(RS485_MUX_ENABLE_PIN, OUTPUT);
        digitalWrite(RS485_SELECT_CH1_PIN, LOW);
        digitalWrite(RS485_SELECT_CH2_PIN, LOW);
        digitalWrite(RS485_MUX_ENABLE_PIN, HIGH); //Disable (the ~RE PINs) the 3_to_1_RS485_Multiplexer
        
        // Select initial port
        SerialMon.print(F("[RS485] Initial port selection: "));
        SerialMon.println(_currentReceiver);
        chooseReceiver(_currentReceiver);
    }
    
}

void RS485Class::end(){
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
        /*Disable the 3_to_1_RS485_Multiplexer*/
        if (getNumOfReceiver() > 1)
        {
            digitalWrite(RS485_MUX_ENABLE_PIN, HIGH);
        }
        Serial2.end();
    }
}

int RS485Class::available(void){
    return Serial2.available();
}

int RS485Class::read(void){
    return Serial2.read();
}

void RS485Class::receive(){
    /*We don't need this function vor AVR-IoT board. This is only for consistancy*/
}

void RS485Class::setNumOfReceiver(uint8_t nrReceiver){
    _nrReceiver = nrReceiver;
}

uint8_t RS485Class::getNumOfReceiver(){
    return _nrReceiver;
}

void RS485Class::chooseReceiver(uint8_t currentReceiver){
    _currentReceiver = currentReceiver;  // Store current receiver
    
    /*Enable the 3_to_1_RS485_Multiplexer*/
    digitalWrite(RS485_MUX_ENABLE_PIN, LOW);
    /*Choose only one of the RS485 Receiver one at a time, see truth table in header*/
    switch (currentReceiver){
        case 1:
            /* Choose A1 and B1 */
            digitalWrite(RS485_SELECT_CH1_PIN, LOW);
            digitalWrite(RS485_SELECT_CH2_PIN, LOW);
            break;
        case 2:
            /* Choose A2 and B2 */
            digitalWrite(RS485_SELECT_CH1_PIN, LOW);
            digitalWrite(RS485_SELECT_CH2_PIN, HIGH);
            break;

        case 3:
            /* Choose A3 and B3 */
            digitalWrite(RS485_SELECT_CH1_PIN, HIGH);
            digitalWrite(RS485_SELECT_CH2_PIN, LOW);
            break;   

        default:
            break;
    }
}

bool RS485Class::processFrame(byte frame[4], byte& input, word& data) {
    // Calculate checksum for first 3 bytes
    uint8_t calculatedChecksum = CRC8.Compute_CRC8(frame, 3);
    
    // Check if checksum matches
    if (calculatedChecksum == frame[3]) {
        input = frame[0];
        data = word(frame[1], frame[2]);
        data = data & 0x0FFF;  // Mask to 12 bits
        return true;
    }
    return false;
}

bool RS485Class::readFrameByte(byte frame[4]) {
    if (available()) {
        // Shift frame buffer left
        frame[0] = frame[1];
        frame[1] = frame[2];
        frame[2] = frame[3];
        frame[3] = read();
        return true;
    }
    return false;
}

void RS485Class::switchToNextReceiver() {
    if (getNumOfReceiver() > 1) {
        // Calculate next receiver
        uint8_t nextReceiver = _currentReceiver + 1;
        if (nextReceiver > getNumOfReceiver()) {
            nextReceiver = 1;
        }
        chooseReceiver(nextReceiver);
    }
}

void RS485Class::reset(uint32_t baudRate) {
    // Disable multiplexer if used
    if (getNumOfReceiver() > 1) {
        digitalWrite(RS485_MUX_ENABLE_PIN, HIGH);
    }
    
    // Small delay for hardware reset
    delay(10);
    
    // Restart communication
    end();
    delay(10);
    begin(baudRate);
    
    // Re-select current receiver if using multiplexer
    if (getNumOfReceiver() > 1 && _currentReceiver > 0) {
        chooseReceiver(_currentReceiver);
    }
}

void RS485Class::checkSensorInputCompletion(bool RS485InputIsFilled[], bool& RS485PacketIsFilled, 
                                           int pressureSensorPort, uint8_t nrOfSensors) {
    RS485PacketIsFilled = true; // Assume all are filled until proven otherwise
    
    // Check temp1 (index 0)
    if (!RS485InputIsFilled[0]) {
        RS485PacketIsFilled = false;
    }
    
    // Check temp2 (index 1)
    if (!RS485InputIsFilled[1]) {
        RS485PacketIsFilled = false;
    }
    
    // Check deflection (index 2)
    if (!RS485InputIsFilled[2]) {
        RS485PacketIsFilled = false;
    }
    
    // Check pressure sensor (index 3) only if it's enabled
    if (pressureSensorPort != -1 && nrOfSensors >= 4 && !RS485InputIsFilled[3]) {
        RS485PacketIsFilled = false;
    }
    
    // Always check PIC temperature sensor (index 4)
    if (!RS485InputIsFilled[4]) {
        RS485PacketIsFilled = false;
    }
}

void RS485Class::resetSensorInputs(bool RS485InputIsFilled[], bool& RS485PacketIsFilled,
                                 int pressureSensorPort, uint8_t nrOfSensors,
                                 uint8_t& temp1ArrayPointer, uint8_t& temp2ArrayPointer,
                                 uint8_t& deflectionArrayPointer, uint8_t& pressureArrayPointer,
                                 uint8_t& picTempArrayPointer) {
    // Reset only the active sensor flags
    RS485InputIsFilled[0] = false;  // temp1
    RS485InputIsFilled[1] = false;  // temp2
    RS485InputIsFilled[2] = false;  // deflection
    if (pressureSensorPort != -1 && nrOfSensors >= 4) {
        RS485InputIsFilled[3] = false;  // pressure (only if enabled)
    }
    RS485InputIsFilled[4] = false;  // PIC temperature
    
    // Reset packet filled flag
    RS485PacketIsFilled = false;
    
    if (RS485_MAX_PORTS > 1) {
        // Reset all array pointers
        temp1ArrayPointer = 0;
        temp2ArrayPointer = 0;
        deflectionArrayPointer = 0;
        pressureArrayPointer = 0;
        picTempArrayPointer = 0;
    }
}

void RS485Class::handleRS485PortSwitching(bool RS485InputIsFilled[], bool& RS485PacketIsFilled,
                                        uint8_t& rs485currentPort,
                                        uint8_t& temp1ArrayPointer, uint8_t& temp2ArrayPointer,
                                        uint8_t& deflectionArrayPointer, uint8_t& pressureArrayPointer,
                                        uint8_t& picTempArrayPointer) {
    if (getNumOfReceiver() > 1) {
        // Reset all RS485 input flags
        for (size_t i = 0; i < 6; i++) {  // Assuming 6 elements in array
            RS485InputIsFilled[i] = false;
        }
        
        // Reset packet filled flag
        RS485PacketIsFilled = false;
        
        // Reset all sensor pointers
        temp1ArrayPointer = 0;
        temp2ArrayPointer = 0;
        deflectionArrayPointer = 0;
        pressureArrayPointer = 0;
        picTempArrayPointer = 0;
        
        // Set the next RS485 Receiver Port
        if (rs485currentPort >= getNumOfReceiver()) {
            rs485currentPort = 1;
        } else {
            rs485currentPort++;
        }
        
        SerialMon.print(F("[RS485] Switching to port: "));
        SerialMon.println(rs485currentPort);
        
        // Choose current RS485 Receiver Port
        chooseReceiver(rs485currentPort);
        
        // Small delay to allow hardware stabilization after port switch
        delay(50);
        // Watchdog.reset(); // Should be handled by caller if needed
    }
}

void RS485Class::outputRS485Status(uint8_t rs485notAvailableCounter, uint8_t calculatedChecksum_false) {
    SerialMon.print(F("RS485-Status: "));
    if (rs485notAvailableCounter == 0 && calculatedChecksum_false < 10) {
        SerialMon.println(F("OK"));
    } else {
        SerialMon.print(F("Fehler - Nicht verfügbar: "));
        SerialMon.print(rs485notAvailableCounter);
        SerialMon.print(F(", Falsche Checksummen: "));
        SerialMon.println(calculatedChecksum_false);
    }
}

void RS485Class::handleNotAvailable(uint8_t& rs485notAvailableCounter, bool RS485InputIsFilled[],
                                   bool& RS485PacketIsFilled, uint8_t& rs485currentPort,
                                   uint8_t& temp1ArrayPointer, uint8_t& temp2ArrayPointer,
                                   uint8_t& deflectionArrayPointer, uint8_t& pressureArrayPointer,
                                   uint8_t& picTempArrayPointer) {
    rs485notAvailableCounter++;
    
    // Output only for critical values
    if (rs485notAvailableCounter > 200) {
        SerialMon.print(F("RS485 nicht verfügbar: "));
        SerialMon.println(rs485notAvailableCounter);
    }
    
    if (rs485notAvailableCounter >= 225) {
        rs485notAvailableCounter = 0;
        
        #ifdef USE_SD_CARD
            // Note: WriteSDCard needs to be called from firmware_functions.cpp
            // due to getTime() dependency
        #endif
        
        // Handle port switching for multi-port configurations
        if (getNumOfReceiver() > 1) {
            handleRS485PortSwitching(RS485InputIsFilled, RS485PacketIsFilled,
                                   rs485currentPort,
                                   temp1ArrayPointer, temp2ArrayPointer,
                                   deflectionArrayPointer, pressureArrayPointer,
                                   picTempArrayPointer);
        }
        
        // Reset RS485 communication
        reset(RS485_BAUDRATE);
    }
}

void RS485Class::handleChecksumError(uint8_t& calculatedChecksum_false, bool RS485InputIsFilled[],
                                    bool& RS485PacketIsFilled, uint8_t& rs485currentPort,
                                    uint8_t& temp1ArrayPointer, uint8_t& temp2ArrayPointer,
                                    uint8_t& deflectionArrayPointer, uint8_t& pressureArrayPointer,
                                    uint8_t& picTempArrayPointer) {
    calculatedChecksum_false++;
    
    // Output error message only for critical values
    if (calculatedChecksum_false > 100) {
        SerialMon.print(F("RS485-Fehler: "));
        SerialMon.print(calculatedChecksum_false);
        SerialMon.println(F(" falsche Checksummen"));
    }
    
    if (calculatedChecksum_false >= 225) {
        #ifdef USE_SD_CARD
            // Note: WriteSDCard needs to be called from firmware_functions.cpp
            // due to getTime() dependency
        #endif
        
        // Handle port switching for multi-port configurations
        if (getNumOfReceiver() > 1) {
            handleRS485PortSwitching(RS485InputIsFilled, RS485PacketIsFilled,
                                   rs485currentPort,
                                   temp1ArrayPointer, temp2ArrayPointer,
                                   deflectionArrayPointer, pressureArrayPointer,
                                   picTempArrayPointer);
        }
        
        // Reset RS485 communication
        reset(RS485_BAUDRATE);
    }
}

bool RS485Class::readAndProcessFrame(byte frame[4], uint8_t& calculatedChecksum_false,
                                   void (*processSensorCallback)(byte input, word data)) {
    // Check if data is available
    if (!available()) {
        return false;
    }
    
    // Read and shift frame buffer
    frame[0] = frame[1];
    frame[1] = frame[2];
    frame[2] = frame[3];
    frame[3] = read();
    
    // Calculate checksum
    uint8_t calculatedChecksum = CRC8.Compute_CRC8(frame, 3);
    
    if (calculatedChecksum == frame[3]) {
        // Valid frame - extract data
        byte input = frame[0];
        word data = word(frame[1], frame[2]);
        data = data & 0x0FFF;
        
        // Reset checksum error counter
        calculatedChecksum_false = 0;
        
        // Call callback to process sensor data
        if (processSensorCallback != nullptr) {
            processSensorCallback(input, data);
        }
        
        return true;
    } else {
        // Invalid checksum - increment error counter
        calculatedChecksum_false++;
        return false;
    }
}

#endif
