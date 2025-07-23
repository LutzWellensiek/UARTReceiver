#ifndef ID_h
#define ID_h

#include <Arduino.h>
/// @brief Constant for storing the deviceId size
const uint8_t SIZE_DEVICE_ID = 21; 
// const uint8_t sizeClientId = 25;
// const uint8_t sizeUsername = 100;
// const uint8_t sizeTopicname = 100;
// const uint8_t sizeHost = 60;

enum ErrorDeviceCode{
    NoErrors,
    ErrorDeviceID,
    ErrorClientID,
    ErrorUsername,
    ErrorTopicname,
    ErrorHost,
    ErrorSnprintf
};

class Device
{
private:
    const char host_buffer[47] = "SmartProductsIoTHub.azure-devices.net";   //"SmartProductsIoTHub-DEVTEST.azure-devices.net";   //  
    const char username_end[26] = "/?api-version=2018-06-30";
    const char topicname_1[10] = "devices/";
    const char topicname_2[53] = "/messages/events/$.ct=application%2Fjson&$.ce=utf-8";
    uint8_t bufferTooSmall = {0};

public:
    char deviceId[SIZE_DEVICE_ID], clientId[25], username[100], topicname[100], host[60];

    Device();
    Device(const char* _deviceID);
    ~Device();
    void setDeviceParam(const char* _deviceID);

    ErrorDeviceCode errorCode = NoErrors;//    ErrorDeviceCode::NoErrors;
    ErrorDeviceCode getErrorDeviceCode();
};


#endif