#include "../Objects/Event/event.hpp"

Event::Event(std::string e_name, int timeout):e_name(std::move(e_name)),type(E_type(l)), timeout(timeout) {};

Event::Event(std::string e_name, E_type type, std::string mqtt_e_name):e_name(std::move(e_name)),type(type),mqtt_e_name(std::move(mqtt_e_name)){}

std::string &Event::getE_name() { return e_name; }
std::string &Event::getMqtt_e_name() { return mqtt_e_name; }
E_type &Event::getType() { return type; }
int &Event::getTimeout() { return timeout; }