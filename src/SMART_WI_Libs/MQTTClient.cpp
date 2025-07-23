/**
 * @file MQTTClient.cpp
 * @author Csaba Freiberger 
 * @brief MQTT Class member function definitions
 * @version 0.1
 * @date 2024-04-11
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#if defined (DXCORE)

#include "MQTTClient.h"

MQTTClient::MQTTClient()
{
    _host = nullptr;
    _clientid = nullptr;
    _username = nullptr;
    _password = nullptr;
    _keepalive = 0;
}

MQTTClient::MQTTClient(const char* host, uint16_t port, const char* clientid, const char* username, const char* password, size_t keepalive)
{
    _host = host;
    _port = port;
    _clientid = clientid;
    _username = username;
    _password = password;
    _keepalive = keepalive;
}

MQTTClient::~MQTTClient()
{
    _host = nullptr;
    _clientid = nullptr;
    _username = nullptr;
    _password = nullptr;
   
}

bool MQTTClient::isPublishAcknowledged = false;

void MQTTClient::setMQTTClientParam(const char* host, uint16_t port, const char* clientid, const char* username, const char* password, size_t keepalive){
    _host = host;
    _port = port;
    _clientid = clientid;
    _username = username;
    _password = password;
    _keepalive = keepalive;
}

bool MQTTClient::publishAcknowledged(){
    if(isPublishAcknowledged){
        return true;
    }else{
        return false;
    }
    
    
}

bool MQTTClient::publish(const char* topicname, const char* payload){
    //TimeoutTimer timeout_ms(28000);
    
    isPublishAcknowledged = MqttClient.publish(topicname, payload, AT_LEAST_ONCE, 60000U);
    return isPublishAcknowledged;
}

bool MQTTClient::connected(){
    return MqttClient.isConnected();//isConnected;
}

bool MQTTClient::connect(){
    Watchdog.reset();
    if (MqttClient.begin(_clientid, _host, _port, true, _keepalive, true, _username, _password, 120000U))
    {   
        Watchdog.reset();
        //Wait till begin() is truely finished and modem sent callback for succesfull connection
        while (!MqttClient.isConnected())
        {
            Watchdog.reset();
            delay(500);
        }

        return true;
        
    }else{
        Watchdog.reset();
        return false;
    }  
}

bool MQTTClient::end(){
   return MqttClient.end();
}

/*MQTT Topic Class*/

MQTTTopic::MQTTTopic(){
    _client = nullptr;
    _topicname = nullptr;
}

MQTTTopic::MQTTTopic(MQTTClient* client, const char* topicname) {
  _client = client;
  _topicname = strdup(topicname);
}

MQTTTopic::~MQTTTopic() {
  delete _topicname;
  _topicname = nullptr;
}

void MQTTTopic::setMQTTTopicParam(MQTTClient* client, const char* topicname){
    _client = client;
    _topicname = strdup(topicname);
}

bool MQTTTopic::publish(const char* payload) {
  return _client->publish(_topicname, payload);
}

#endif