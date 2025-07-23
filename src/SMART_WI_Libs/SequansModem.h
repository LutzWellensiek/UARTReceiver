/*This Class migrates the SequansController class to the smart-products-firmware in order to have the same member functions which are beeing used
Created by Csaba Freiberger 02.2023*/

#ifndef SequansModem_h
#define SequansModem_h

#if defined (DXCORE)
#include "sequans_controller.h"
#include "lte.h"

/**
 * @brief enumaration for setting the Sim status
 * 
 */
enum SimStatus {
  SIM_ERROR            = 0,
  SIM_READY            = 1,
  SIM_LOCKED           = 2,
  SIM_TIMEOUT          = 3,
};

class SequansModem
{
private:
     volatile bool gprs_connected = false;
public:
    /**
     * @brief Constructor for Sequans modem
     */
    SequansModem(void);
    ~SequansModem(void);

    /**
     * @brief Initialize the modem with the begin() method from the SequansController class from sequans_controller library
     * 
     * @return true if succesfull
     * @return false if initialization failed
     */
    bool init(void);

    /**
     * @brief Initialize the modem and waits for the initial CEREG response after set full functionality
     * 
     * @return true if initialization was successfull. This does not mean you have always connection to the network!
     * @return false if something went wrong
     */
    bool restart();

    /**
     * @brief This function performs a hardware reset of the modem via AT command
     * 
     * @return true if reset was successfull
     * @return false if something went wrong
     */
    bool reset();

    /**
     * @brief  Disables the interface with the LTE module. Disconnects from
     * operator. Disables interrupts used for the Sequans module and closes the
     * serial interface.
     */
    void end();

    /**
     * @brief Test whether the modem is awake and available
     * 
     * @return true if modem listens
     * @return false if modem is not awake
     */
    bool testAT();

    /**
     * @brief This command chooses the operating mode between LTE-M and NB-IoT on a device. This command can be run only if the device is in CFUN=0 state.
     * 
     * @return true if mode change was successful
     * @return false if couldn't change operating mode
     */
    bool changeOparatingMode();

    /**
     * @brief Get the Modem Info object
     * 
     * @return String of the modem manufacturer, model and UE version
     */
    String getModemInfo();

    /**
     * @brief Get the Sim Status object
     * 
     * @param timeout after the timeout the SimStatus is 3
     * @return SimStatus enumaration
     */
    SimStatus getSimStatus(uint32_t timeout_ms = 7000U);

    /**
     * @brief Unlocks the SIM with the PIN
     * 
     * @param pin 
     * @return true if SIM was unlocked
     * @return false if an ERROR accured
     */
    bool simUnlock(const char* pin);

    /**
     * @brief  this is only a dummy function in order to have the compatibility with the TinyGsm library. 
     * It is the same as The restart() member function, as this already connects the gprs
     * 
     * @param apn 
     * @param user 
     * @param pwd 
     * @return true if gprs is connected, 
     * @return false if not
     */
    bool gprsConnect(const char* apn, const char* user = NULL,
                       const char* pwd = NULL);

    /**
     * @brief Checks if the gprs is connected with AT+CGATT?
     * 
     * @return true or
     * @return false 
     */
    bool isGprsConnected();

    /**
     * @brief Checks the status of the gprs after the ifGprsConnected() member fuction
     * 
     * @param buffer is the response of the modem after calling the ifGprsConnect()
     */
    void GprsStatus(char * buffer);

    /**
     * @brief Get the Local IP Address from PDP
     * 
     * @return ip4 Address 
     */
    String getLocalIP();

    /**
     * @brief Waits for network attachment
     * 
     * @param timeout_ms  Timeout for waiting in ms
     * @param check_signal if true, the signal quality is beeing checked. Default is false
     * @param changeOPMode if true, the modem changes between LTE-M and NB-IoT after waiting is timed out and trys to connect
     * to one of them. Default is false
     * @return true if modem is attached, 
     * @return false if not
     */
    bool waitForNetwork(uint32_t timeout_ms = 60000L, 
                            bool check_signal = false,
                            bool changeOPmade = false);
    
    /**
     * @brief Checks network conection
     * 
     * @return true if modem is connected, 
     * @return false if not
     */
    bool isNetworkConnected();

    /**
     * @brief Get the Signal Quality of the modem
     * 
     * @return int16_t value of signal strength indication. returns 99 if signal is not known or not detectable
     */
    int16_t getSignalQuality();

    /**
     * @brief Get the network operator for the modem
     * 
     * @return String of current network operator
     */
    String getOperator();

    /**
     * @brief Get the Network Time object
     * 
     * @param year 
     * @param month 
     * @param day 
     * @param hour 
     * @param minute 
     * @param second 
     * @param timezone 
     * @return true  
     */
    bool getNetworkTime(int* year, int* month, int* day, int* hour,
                          int* minute, int* second, float* timezone); 
};

#endif
#endif