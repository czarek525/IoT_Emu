#pragma once
#include "../Flow/flow.hpp"

class Client_info
{
    std::string remote_IP, m_name;
    int remote_port;
    std::unordered_map<std::string,std::shared_ptr<Flow>> sname_flow;

public:
    Client_info(std::string, int, std::string, std::unordered_map<std::string,  std::shared_ptr<Flow>>);
    std::string &getRemote_IP(), &getM_name();
    int &getRemote_port();
    std::shared_ptr<Flow> getFlow(const std::string&);
};