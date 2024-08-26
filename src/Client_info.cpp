#include "../Objects/Port/client_info.hpp"
#include "../Headers/helper_functions.hpp"

Client_info::Client_info(std::string remote_IP, int remote_port, std::string m_name, std::unordered_map<std::string,std::shared_ptr<Flow>>sname_flow) : remote_IP(std::move(remote_IP)), m_name(std::move(m_name)),remote_port(remote_port), sname_flow(std::move(sname_flow)) {}

std::string& Client_info::getM_name() { return m_name; }
std::string &Client_info::getRemote_IP() { return remote_IP; }
int &Client_info::getRemote_port() { return remote_port; }
std::shared_ptr<Flow> Client_info::getFlow(const std::string& s_name){
    return Helper_functions::getObjectByName(sname_flow, s_name);
}
 
