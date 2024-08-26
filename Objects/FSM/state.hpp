#pragma once
#include "transition.hpp"

class State
{
    std::string s_name;
    std::vector<std::string> on_entry, on_exit;
    std::unordered_map<std::string, std::shared_ptr<Transition>> ename_transitionptr;
public:
    State(
        std::string, 
        std::vector<std::string>, 
        std::vector<std::string>, 
        std::unordered_map<std::string, std::shared_ptr<Transition>>);

    std::string &getS_name();
    std::vector<std::string> &getOn_entry(), &getOn_exit();
    std::shared_ptr<Transition> getTransition(const std::string& e_name);

};