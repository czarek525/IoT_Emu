#include "../Objects/MQTT_BROKER/mqtt_broker.hpp"

MQTT_Broker::MQTT_Broker(std::string endpoint_IP, std::string endpoint_port):endpoint_IP(std::move(endpoint_IP)),endpoint_port(std::move(endpoint_port)){};

std::string& MQTT_Broker::getEndpoint_IP() { return endpoint_IP; }
std::string& MQTT_Broker::getEndpoint_port() { return endpoint_port;}