#include "../Objects/Port/port.hpp"

Port::Port(std::string p_name, Port_type p_type, Transport_type p_transport, std::string local_IP, int local_port, std::shared_ptr<Client_info> client_infoptr) : p_name(std::move(p_name)),p_type(p_type),p_transport(p_transport),local_IP(std::move(local_IP)),local_port(local_port), client_infoptr(std::move(client_infoptr)){}

std::string &Port::getP_name() { return p_name; }
std::string &Port::getLocal_IP() { return local_IP; }
int &Port::getLocal_port() { return local_port; }
Port_type &Port::getP_type() { return p_type; }
Transport_type &Port::getP_transport() { return p_transport; }
std::shared_ptr<Client_info> Port::getClient_info(){
    if (client_infoptr)
        return client_infoptr;
    return nullptr;
}