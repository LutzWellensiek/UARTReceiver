#include "ID.h"

Device::Device(){
    setDeviceParam("0");
}

Device::Device(const char* _deviceID){
    setDeviceParam(_deviceID);
}

Device::~Device(){}

void Device::setDeviceParam(const char *_deviceID){
    if (_deviceID == nullptr)
    {
        errorCode = ErrorDeviceID;
        return;
    }
    
    bufferTooSmall = snprintf(deviceId, sizeof(deviceId)/sizeof(deviceId[0]), _deviceID);
    if (bufferTooSmall >= sizeof(deviceId)/sizeof(deviceId[0]))
    {
        errorCode = ErrorDeviceID;
        return;
    }else if (bufferTooSmall == -1)
    {
        errorCode = ErrorSnprintf;
        return;
    }
        
    // ID.toCharArray(clientId, ID.length() + 1);
    bufferTooSmall = snprintf(clientId, sizeof(clientId)/sizeof(clientId[0]), _deviceID);
    if (bufferTooSmall >= sizeof(clientId)/sizeof(clientId[0]))
    {
        errorCode = ErrorClientID;
        return;
    }else if (bufferTooSmall < 0)
    {
        errorCode = ErrorSnprintf;
        return;
    }

    // username_buffer = username_1 + deviceId + username_2;
    bufferTooSmall = snprintf(username, sizeof(username)/sizeof(username[0]),"%s/%s%s", host_buffer, _deviceID, username_end);
    if (bufferTooSmall >= sizeof(username)/sizeof(username[0]))
    {
        errorCode = ErrorUsername;
        return;
    }else if (bufferTooSmall < 0)
    {
        errorCode = ErrorSnprintf;
        return;
    }
        
    // topicname_buffer = topicname_1 + deviceId + topicname_2;
    bufferTooSmall = snprintf(topicname, sizeof(topicname)/sizeof(topicname[0]), "%s%s%s", topicname_1, _deviceID, topicname_2);
    if (bufferTooSmall >= sizeof(topicname)/sizeof(topicname[0]))
    {
        errorCode = ErrorTopicname;
        return;
    }else if (bufferTooSmall < 0)
    {
        errorCode = ErrorSnprintf;
        return;
    }

    //host_buffer.toCharArray(host, host_buffer.length() + 1);
    bufferTooSmall = snprintf(host, sizeof(host)/sizeof(host[0]), host_buffer);
    if (bufferTooSmall >= sizeof(host)/sizeof(host[0]))
    {
        errorCode = ErrorHost;
        return;
    }else if (bufferTooSmall < 0)
    {
        errorCode = ErrorSnprintf;
        return;
    }
}

ErrorDeviceCode Device::getErrorDeviceCode(){
    return errorCode;
}