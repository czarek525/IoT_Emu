#pragma once
#include "../../Headers/headers.hpp"

class Event
{
    std::string e_name, mqtt_e_name;
    E_type type;
    int timeout;

public:
    Event(std::string, int);
    Event(std::string, E_type, std::string);

    std::string &getE_name(), &getMqtt_e_name();
    E_type &getType();
    int &getTimeout();
};