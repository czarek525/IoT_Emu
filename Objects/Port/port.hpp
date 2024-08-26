#pragma once
#include "client_info.hpp"


class Port
{
    std::string p_name, local_IP;
    int local_port;
    Port_type p_type;
    Transport_type p_transport;
    std::shared_ptr<Client_info> client_infoptr;

public:
    Port(std::string, Port_type, Transport_type, std::string, int, std::shared_ptr<Client_info> client_infoptr = nullptr);

    std::string &getP_name(), &getLocal_IP() ;
    int &getLocal_port();
    Port_type &getP_type();
    Transport_type &getP_transport();
    std::shared_ptr<Client_info> getClient_info();
};