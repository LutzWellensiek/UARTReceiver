/**
 * @file MQTTClient.h
 * @author Csaba Freiberger
 * @brief Library for MQTT Conenction and for compatibility to smart-products-firmware
 * @version 0.1
 * @date 2023-08-30
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#if defined (DXCORE)
#ifndef MQTTClient_h
#define MQTTClient_h

#include <mqtt_client.h>
#include <timeout_timer.h>
#include "watchdogAVR.h"
/*For MQTT connection*/
class MQTTClient//: public MqttClient
{
    friend class MQTTTopic;
private:
    const char* _host;
    uint16_t _port;
    const char* _clientid;
    const char* _username;
    const char* _password;
    uint16_t _keepalive;
    static bool isPublishAcknowledged;
public:

    /**
     * @brief Construct a new MQTTClient object
     * 
     */
    MQTTClient();

    /**
     * @brief Construct a new MQTTClient object
     * 
     * @param host 
     * @param port 
     * @param clientid 
     * @param username 
     * @param password 
     * @param keepalive 
     */
    MQTTClient(const char* host, uint16_t port, const char* clientid, const char* username, const char* password, size_t keepalive = 1200);

    /**
     * @brief Destroy the MQTTClient object
     * 
     */
    ~MQTTClient();

    /**
     * @brief Sets the required parameters for the MQTT Client
     * 
     * @param host 
     * @param port 
     * @param clientid 
     * @param username 
     * @param password 
     * @param keepalive 
     */
    void setMQTTClientParam(const char* host, uint16_t port, const char* clientid, const char* username, const char* password, size_t keepalive = 1200);

    /**
     * @brief Checks if the published message was acknowledged by the broker
     * 
     * @return true if acknowledged,
     */
    bool publishAcknowledged();

    /**
     * @brief Cehcks if MQTT Client is connected to broker
     * 
     * @return true connected,
     */
    bool connected();

    /**
     * @brief Connects to the MQTT broker
     * 
     * @return true if connection successfull,
     */
    bool connect();

    /**
     * @brief Disconnects from the broker and resets the state in the MQTT
     * client.
     * 
     * @return true if disconnection was successfull
     */
    bool end();
protected:
    /**
     * @brief Publish MQTT message
     * 
     * @param topicname 
     * @param payload 
     * @return true if publishing was successfull
     */
    bool publish(const char* topicname, const char* payload);

};

/*MQTT Topic Class*/
class MQTTTopic {
  private:
    MQTTClient* _client;
    char* _topicname;
    bool _retain;

  public:
    /**
    * @brief Construct a new MQTTTopic object
    * 
    */
    MQTTTopic();

    /**
     * @brief Construct a new MQTTTopic object
     * 
     * @param client 
     * @param topicname 
     */
    MQTTTopic(MQTTClient* client, const char* topicname);

    /**
     * @brief Destroy the MQTTTopic object
     * 
     */
    virtual ~MQTTTopic();

    /**
     * @brief Sets the MQTT Topic parameters
     * 
     * @param client 
     * @param topicname 
     */
    void setMQTTTopicParam(MQTTClient* client, const char* topicname);

    /**
     * @brief Publishes payload to the specified topic
     * 
     * @param payload 
     * @return true if publishing was successfull
     */
    bool publish(const char* payload);
};

#endif
#endif