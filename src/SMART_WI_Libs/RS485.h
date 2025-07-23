/*Defines the RS485 communication via the UART2 Pins for AVR-IoT-Cellular
Created by Csaba Freiberger 01.2023*/
#ifndef RS485_h
#define RS485_h
#if defined (DXCORE)
#include <Arduino.h>
#include "UART.h" 
#include "UART_private.h"

/*
 * Electrical Schematic (Black Box Model) of the 3_to_1_RS485_Multiplexer
 *
 *        +--------------------+
 * Vcc o--|                     |--o A_1
 * RX  o--|                     |--o B_1
 * A0  o--|                     |--o A_2
 * A1  o--|     Black Box       |--o B_2
 * A2  o--|                     |--o A_3
 * GND o--|                     |--o B_3
 *        +--------------------+
 * 
 * Inputs: 
 * - Vcc: Power supply voltage
 * - RX: Receive data on TTL level
 * - A0: Analog input 0 for choosing RS485 canal
 * - A1: Analog input 1 for choosing RS485 canal
 * - A2: Analog input 2 for enabling the multiplexer
 * - GND: Ground
 */
#define RS485_SELECT_CH1_PIN    PIN_PD6  // A0 – wählt RS485-Kanal
#define RS485_SELECT_CH2_PIN    PIN_PD1  // A1 – wählt RS485-Kanal
#define RS485_MUX_ENABLE_PIN    PIN_PD3  // A2 – aktiviert Multiplexer
/* 
 * Outputs: 
 * - A1 not inverted wire of 1st measuring unit 
 * - B1 inverted wire of =||=
 * - A2 not inverted wire of 2nd measuring unit 
 * - B2 inverted wire of =||=
 * - A3 not inverted wire of 3rd measuring unit 
 * - B3 inverted wire of =||=
 * 
 * Truth Table
 *
 * A0 | A1 | A2 | ~RE       -> RS485 Lines
 * ----------------------------------
 *  x |  x |  1 |     -
 *  0 |  0 |  0 |   S1 = RE1 -> A1 & B1
 *  0 |  1 |  0 |   S2 = RE2 -> A2 & B2
 *  1 |  0 |  0 |   S3 = RE3 -> A3 & B3
 * 
 * Note: Internal connections are handled as a black box.
 */

class RS485Class
{
private:
    int _txPin;
    int _rxPin;
    HardwareSerial *uart;
    uint8_t _nrReceiver;
    uint8_t _currentReceiver;

    /**
     * @brief Set the Num Of RS485 Receiver
     * 
     * @param _nrReceiver 
     */
    void setNumOfReceiver(uint8_t nrReceiver);
    
public:
    /**
     * @brief Construct a new RS485Class object
     * 
     * @param txPin int. TX PIN
     * @param rxPin int. RX PIN
     * @param nrReceiver uint8_t number of connected RS485 receiver, default is one
     */
    RS485Class(int txPin, int rxPin, uint8_t nrReceiver = 1);

    /**
     * @brief Destroy the RS485Class object
     * 
     */
    ~RS485Class();

    /**
     * @brief Begin the USART2 a.k.a Serial2 communication and set the PINs for the RS495 Multiplexer if needed
     * 
     * @param baudRate uint32_t. Baudrate for USART2 communication
     */
    void begin(uint32_t baudRate);

    /**
     * @brief !!!Do not call this, somehow it breaks code!!!! ends the RS485 communication
     * 
     */
    void end();

    /**
     * @brief Get the number of bytes (characters) available for reading from the RS485 port
     * This is data that already arrived and is stored in the USART2 receive buffer.
     * 
     * @return int. The number of bytes available to read.
     */
    int available(void);

    /**
     * @brief Reads incoming serial data
     * 
     * @return int. The first byte of incoming serial data available or -1 if no data is available.
     */
    int read(void);

    /**
     * @brief we don't need this function for the AVR-IoT board. It is defined only for cosistancy
     * 
     */
    void receive();

    /**
     * @brief Get the Num Of RS485 Receiver
     * 
     * @return uint8_t number of set & connected receiver
     */
    uint8_t getNumOfReceiver();

    /**
     * @brief Chooses one of maximum 3 RS485 receiver
     * 
     * @param currentReceiver from 1 to 3
     */
    void chooseReceiver(uint8_t currentReceiver);

    /**
     * @brief Process incoming RS485 frame and validate checksum
     * 
     * @param frame Frame buffer (4 bytes)
     * @param input Output: extracted input byte
     * @param data Output: extracted data word
     * @return true if frame is valid, false if checksum failed
     */
    bool processFrame(byte frame[4], byte& input, word& data);

    /**
     * @brief Read next frame byte and shift buffer
     * 
     * @param frame Frame buffer to update
     * @return true if byte was read, false if no data available
     */
    bool readFrameByte(byte frame[4]);

    /**
     * @brief Get current receiver port
     * 
     * @return uint8_t current receiver (1-3)
     */
    uint8_t getCurrentReceiver() { return _currentReceiver; }

    /**
     * @brief Switch to next receiver port in round-robin fashion
     */
    void switchToNextReceiver();

    /**
     * @brief Reset communication (for error recovery)
     * 
     * @param baudRate Baudrate to reinitialize with
     */
    void reset(uint32_t baudRate);

    // Sensor data management functions
    /**
     * @brief Check if all configured sensors have provided their data
     * 
     * @param RS485InputIsFilled Array of sensor completion flags
     * @param RS485PacketIsFilled Reference to packet completion flag
     * @param pressureSensorPort Pressure sensor port (-1 if disabled)
     * @param nrOfSensors Number of configured sensors
     */
    static void checkSensorInputCompletion(bool RS485InputIsFilled[], bool& RS485PacketIsFilled, 
                                         int pressureSensorPort, uint8_t nrOfSensors);
    
    /**
     * @brief Reset sensor input flags and array pointers
     * 
     * @param RS485InputIsFilled Array of sensor completion flags
     * @param RS485PacketIsFilled Reference to packet completion flag
     * @param pressureSensorPort Pressure sensor port (-1 if disabled)
     * @param nrOfSensors Number of configured sensors
     * @param temp1ArrayPointer Reference to temp1 array pointer
     * @param temp2ArrayPointer Reference to temp2 array pointer
     * @param deflectionArrayPointer Reference to deflection array pointer
     * @param pressureArrayPointer Reference to pressure array pointer
     * @param picTempArrayPointer Reference to PIC temp array pointer
     */
    static void resetSensorInputs(bool RS485InputIsFilled[], bool& RS485PacketIsFilled,
                                int pressureSensorPort, uint8_t nrOfSensors,
                                uint8_t& temp1ArrayPointer, uint8_t& temp2ArrayPointer,
                                uint8_t& deflectionArrayPointer, uint8_t& pressureArrayPointer,
                                uint8_t& picTempArrayPointer);
    
    /**
     * @brief Switch to the next RS485 port in multi-port configurations
     * 
     * @param RS485InputIsFilled Array of sensor completion flags
     * @param RS485PacketIsFilled Reference to packet completion flag
     * @param rs485currentPort Reference to current port number
     * @param temp1ArrayPointer Reference to temp1 array pointer
     * @param temp2ArrayPointer Reference to temp2 array pointer
     * @param deflectionArrayPointer Reference to deflection array pointer
     * @param pressureArrayPointer Reference to pressure array pointer
     * @param picTempArrayPointer Reference to PIC temp array pointer
     */
    void handleRS485PortSwitching(bool RS485InputIsFilled[], bool& RS485PacketIsFilled,
                                 uint8_t& rs485currentPort,
                                 uint8_t& temp1ArrayPointer, uint8_t& temp2ArrayPointer,
                                 uint8_t& deflectionArrayPointer, uint8_t& pressureArrayPointer,
                                 uint8_t& picTempArrayPointer);
    
    /**
     * @brief Handle not-available RS485 and execute port switching/rereset
     * 
     * @param rs485notAvailableCounter Reference to RS485 not available counter
     * @param RS485InputIsFilled Array for RS485 input filled status
     * @param RS485PacketIsFilled Reference to RS485 packet filled status
     * @param rs485currentPort Reference to current RS485 port
     * @param temp1ArrayPointer Reference to temp1 array pointer
     * @param temp2ArrayPointer Reference to temp2 array pointer
     * @param deflectionArrayPointer Reference to deflection array pointer
     * @param pressureArrayPointer Reference to pressure array pointer
     * @param picTempArrayPointer Reference to PIC temp array pointer
     */
    void handleNotAvailable(uint8_t& rs485notAvailableCounter, bool RS485InputIsFilled[],
                            bool& RS485PacketIsFilled, uint8_t& rs485currentPort,
                            uint8_t& temp1ArrayPointer, uint8_t& temp2ArrayPointer,
                            uint8_t& deflectionArrayPointer, uint8_t& pressureArrayPointer,
                            uint8_t& picTempArrayPointer);

    /**
     * @brief Handle checksum errors and execute recovery
     * 
     * @param calculatedChecksum_false Reference to checksum error counter
     * @param RS485InputIsFilled Array for RS485 input filled status
     * @param RS485PacketIsFilled Reference to RS485 packet filled status
     * @param rs485currentPort Reference to current RS485 port
     * @param temp1ArrayPointer Reference to temp1 array pointer
     * @param temp2ArrayPointer Reference to temp2 array pointer
     * @param deflectionArrayPointer Reference to deflection array pointer
     * @param pressureArrayPointer Reference to pressure array pointer
     * @param picTempArrayPointer Reference to PIC temp array pointer
     */
    void handleChecksumError(uint8_t& calculatedChecksum_false, bool RS485InputIsFilled[],
                             bool& RS485PacketIsFilled, uint8_t& rs485currentPort,
                             uint8_t& temp1ArrayPointer, uint8_t& temp2ArrayPointer,
                             uint8_t& deflectionArrayPointer, uint8_t& pressureArrayPointer,
                             uint8_t& picTempArrayPointer);
    /**
     * @brief Output RS485 status summary
     * 
     * @param rs485notAvailableCounter RS485 not available counter
     * @param calculatedChecksum_false Failed checksum counter
     */
    static void outputRS485Status(uint8_t rs485notAvailableCounter, uint8_t calculatedChecksum_false);

    /**
     * @brief Read and process next RS485 frame
     * Handles frame reading, checksum validation and error counting
     * 
     * @param frame Frame buffer (4 bytes)
     * @param calculatedChecksum_false Reference to checksum error counter
     * @param processSensorCallback Callback function to process sensor data
     * @return true if frame was successfully processed, false otherwise
     */
    bool readAndProcessFrame(byte frame[4], uint8_t& calculatedChecksum_false,
                            void (*processSensorCallback)(byte input, word data));

};
#endif
#endif
