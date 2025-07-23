/**
 * @file KitConfig.h
 * @author Csaba Freiberger (csaba.freiberger@witzenmann.com)
 * @brief In this config file, we define any and every constants, which are relevant for the smart-products-firmware.ino file
 * @version 0.1
 * @date 2025-02-25
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef KitConfig_h
#define KitConfig_h

//#define SMART_WI_0_9
//#define SMART_WI_1_0
#define SMART_WI_1_1

#if defined (DXCORE)
   #if defined (__AVR__)
  enum InputPort{
        IN1 = 1,
        IN2,
        IN3,
        IN4,
        IN5,
        IN6,
        IN7,
        IN8,
        IN9,
        INA,
        INB
};
#endif
#endif

// Network mode configuration
enum class NetworkMode {
    LORAWAN,
    CELLULAR_MQTT,
    HYBRID  // Optional für zukünftige Nutzung
};

const NetworkMode currentMode = NetworkMode::LORAWAN; // oder CELLULAR_MQTT

const uint8_t RS485_MAX_PORTS = {2};            // Number of connected RS485 ports
const uint32_t RS485_BAUDRATE = {115200};       // Baudrate for RS485 communication
const uint32_t SERIAL_MON_BAUDRATE = {115200};  // Baudrate for Serial Monitor
const uint16_t WATCHDOG_TIMEOUT = {16000};      // Watchdog timeout in ms

// Define how you're planning to connect to the internet.
// This is only needed for this example, not in other code.
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false

// set GSM PIN, if any
#define GSM_PIN ""

/*defining of sensor measurement variables and offsets
here you have to define which sensor you are currently using and adjust the offstes for each measuring unit*/
const uint8_t sensorArraySize = {12};   //Array size for Sensor values

const int angularSensorPort = IN3;          //Port for angular sensor
const int pressureSensorPort = -1;//IN4;         //Port for pressure sensor
//const int capaSensorPort = IN5;             //Port for capacitive sensor

const uint8_t nrOfSensors = {3};            //Number of sensors attached to the SMART.WI.Kit

/*
    *Define constants for sensor calculation
*/
#ifdef SMART_WI_1_0
  const float ADCResolution = 4095.0F;    //Resolution of 12Bit ADC
  const float refVoltageMax = 6.6F;      //Reference voltage for sensor input in V
  const float refVoltageMin = 3.3F;       //Reference voltage for sensor input in V
#else
  const float ADCResolution = 4095.0F;    //Resolution of 12Bit ADC
  const float refVoltageMax = 10.0F;      //Reference voltage for sensor input in V
  const float refVoltageMin = 5.0F;       //Reference voltage for sensor input in V
#endif

const float resistance = 500.0F;  //328.7F;//        //Resistance for measuring current in Ohm 500
const float KP_length = 350.5F;         //Length of the expansion joint in mm
const float KP_radius = 185.39F;        //Radius of the expansion joint in mm
const float sensorLength = 223.5F;      //Length of the LVDT sensor in mm
const float screwGradient = 10.0F;//72.0F;      //Gradient of the screw in mm

/*
    *Define constants for reading and sending intervall
*/
#define TELEMETRY_SEND_INTERVAL 25000 / RS485_MAX_PORTS     // old 25000 // telemetry data sent every 24/3 seconds
#if defined (__SAMD21G18A__)
  #define SERIAL_READ_INTERVAL  100       // read sensors every 100 ms
#elif defined(__AVR__)
  #define SERIAL_READ_INTERVAL  5       // read sensors every 20/4 ms
#endif

// Your GPRS credentials, if any
const char apn[]      = "lpwa.vodafone.com";
const char gprsUser[] = "";
const char gprsPass[] = "";

// UART2 Konfiguration
#define SerialPIN_TX PIN_PF4
#define SerialPIN_RX PIN_PF5
#define UART2_BAUDRATE 115200

#endif
