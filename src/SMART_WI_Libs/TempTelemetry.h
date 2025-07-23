/**
 * @file TempTelemetry.h
 * @author Csaba Freiberger (csaba.freiberger@witzenmann.com)
 * @brief This class is for saving telemetry data sets to the external EEPROM of the SMART.WI.Kit from v1.01
 * @version 0.1
 * @date 2024-10-11
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef TempTelemetry_h
#define TempTelemetry_h

    #include "EEPROM_SPI.h"
    #include "CRC8.h"
    #include "ID.h"

    /**
     * @brief a struct for storing the sensor data for telemetry
     * 
    */
    // struct TelemetryData{
    //     float temp1 = {0.0};
    //     float temp2 = {0.0};
    //     float deflection = {0.0};
    //     float pressure = {0.0};
    //     float picTemp = {0.0};
    //     //uint32_t timestamp = {0};
    //     char deviceID[SIZE_DEVICE_ID] = {0};
    //     uint8_t crcValue = {0};
    // };

    class TelemetryData
    {
    private:
        
    public:
        float temp1 = {0.0};
        float temp2 = {0.0};
        float deflection = {0.0};
        float deflection2 = {0.0};
        float pressure = {0.0};
        float picTemp = {0.0};
        //uint32_t timestamp = {0};
        char deviceID[SIZE_DEVICE_ID] = {0};
        uint8_t crcValue = {0};

        TelemetryData(){} //Default constructor
        //Constructor for 3 sensors
        TelemetryData(float _temp1, float _temp2, float _deflection, float _picTemp, char (&_deviceID)[SIZE_DEVICE_ID], uint8_t _crcValue = 0): 
             temp1(_temp1), temp2(_temp2), deflection(_deflection), picTemp(_picTemp), crcValue(_crcValue)
        {
            snprintf(deviceID, SIZE_DEVICE_ID, _deviceID);
        }

        //Constructor for 4 sensors
        TelemetryData(float _temp1, float _temp2, float _deflection, float _pressure, float _picTemp, char (&_deviceID)[SIZE_DEVICE_ID], uint8_t _crcValue = 0): 
             temp1(_temp1), temp2(_temp2), deflection(_deflection), pressure(_pressure), picTemp(_picTemp), crcValue(_crcValue)
        {
            snprintf(deviceID, SIZE_DEVICE_ID, _deviceID);
        }

        //Constructor for 5 sensors
        TelemetryData(float _temp1, float _temp2, float _deflection, float _deflection2, float _pressure, float _picTemp, char (&_deviceID)[SIZE_DEVICE_ID], uint8_t _crcValue = 0):
             temp1(_temp1), temp2(_temp2), deflection(_deflection), deflection2(_deflection2), pressure(_pressure), picTemp(_picTemp), crcValue(_crcValue)
        {
            snprintf(deviceID, SIZE_DEVICE_ID, _deviceID);
        }

        //Default destructor
        ~TelemetryData(){};
    };
    
    

    /**
     * @brief a struct for storing addresses on the EEPROM wiht a CRC value
     * 
     */
    struct EEPROM_address
    {
        uint32_t value = {0};
        uint8_t crc = {0};
    };


    /**
     * @brief Class for saving and extracting telemetry data to/from the external EEPROM
     * 
     */
    class TempTelemetry
    {
        private:
            uint32_t currentReadAddress;
            volatile bool is_allExtracted;
            uint32_t numSavedTelem;
            size_t sizeTelem;
        
        public:
            TempTelemetry(TelemetryData &telem);
            ~TempTelemetry();

            /**
             * @brief Resets the value of ADDRESS_LAST_TELEM_ADDRESS_01 and ADDRESS_PAGE_FLAG
             * 
             */
            void resetTelemAddresses();

            /**
             * @brief Updates the current page, by going to the next page and 
             * sets the address of the current last telemetry address to the beginning of the page
             * 
             * @param _currentPage array for the page value
             * @param _lastTelemAddress struct for the last telemetry address
            */
            void updateTelemAddresses(uint8_t (_currentPage)[2], EEPROM_address _lastTelemAddress);

            /**
             * @brief Initialize the current page value & the current last telemetry address
             * 
             * @param _currentPage array for the page value
             * @param _lastTelemAddress struct for the last telemetry address
             * @return true if initialization was successfull,
             * @return false if something was currupted
            */
            bool initTelemAddresses(uint8_t (&_currentPage)[2], EEPROM_address &_lastTelemAddress);

            /**
             * @brief Checks the EEPROM wether there are new saved telemetry data
             * 
             * @return true if there a re new telemetry set(s) saved
             * @return false if not
            */
            bool checkForNewSavedTelem();

            /**
             * @brief Get the Num Saved Telem object
             * 
             * @return uint32_t 
             */
            uint32_t getNumSavedTelem();

            /**
             * @brief Claculatze the number of saved telemetry set(s) on the EEPROM
             * 
             * @param _firstTelemAddress 
             * @param _lastTelemAddress
             * @return uint32_t 
             */
            uint32_t calcNumSavedTelem(uint32_t _firstTelemAddress, uint32_t _lastTelemAddress);

            /**
             * @brief Save telemetry data to the external EEPROM
             * 
             * @param telemetry which should be written
             */
            void saveTelemetry(TelemetryData &telemetry);

            /**
             * @brief Extract telemetry data from external EEPROM
             * 
             * @param _currentPage array for the page value
             * @param _lastTelemAddress struct for the last telemetry address
             * @param _savedTelemetry[out] The memory area to copy to
             * @return true if all telemetry data has been extracted,
             * @return false if something went wrong
            */
            bool extractAllTelemetry(uint8_t (&_currentPage)[2], EEPROM_address &_lastTelemAddress, TelemetryData &_savedTelemetry);

            /**
             * @brief Extract the only saved telemetry data from external EEPROM
             * 
             * @param _lastTelemAddress address for the last telemetry data
             * @param _savedTelemetry[out]  The memory area to copy to
             * @return true if telemetry data has been extracted,
             * @return false if something went wrong
            */
            bool extractOnlyTelemetry(uint32_t _lastTelemAddress, TelemetryData &_savedTelemetry);

            /**
             * @brief Checks if every saved telemtry set(s) have been extracted from EEPROM
             * 
             * @return true if al have been extracted
             * @return false if not yet
             */
            bool allTelemExtracted(void);

            /**
             * @brief Write data to the external EEPROM, till it is successfull
             * 
             * @tparam T typename for the address
             * @tparam N size of array
             * @param address where the data on the EEPROM should be written
             * @param frame reference to the array which should be written
             * @return true if wirting process was successfull,
             * @return false if not or timed out
            */
            template <typename T, size_t N>
            bool writeTillCorrectCRC(const T address, uint8_t (&frame)[N]);

            /**
             * @brief 
             * 
             * @param address where the data on the EEPROM should be written
             * @param frame reference to the data frame which should be written
             * @return true if wirting process was successfull,
             * @return false if not or timed out
             */
            bool writeTillCorrectCRC(const uint32_t address, const EEPROM_address (&frame));

            /**
             * @brief Print the telemetry data to the Serial Monitor
             * 
            */
            void printTelemetry(TelemetryData &_telemetry);
    };

#endif