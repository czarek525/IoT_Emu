#pragma once
#include "../../Headers/headers.hpp"

class Transition
{
    std::string e_name, s_name;
    std::vector<std::string> actions;

public:
    Transition(std::string, std::string, std::vector<std::string>);

    std::string &getE_name() , &getS_name() ;
    std::vector<std::string> &getActions() ;
};