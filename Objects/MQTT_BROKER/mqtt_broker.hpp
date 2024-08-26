#pragma once
#include "../../Headers/headers.hpp"
class MQTT_Broker
{
    std::string endpoint_IP;
    std::string endpoint_port;

public:
    MQTT_Broker(std::string, std::string);

    std::string &getEndpoint_IP(), &getEndpoint_port();
};