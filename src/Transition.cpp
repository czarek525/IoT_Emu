#include "../Objects/FSM/transition.hpp"


Transition::Transition(std::string e_name, std::string s_name, std::vector<std::string> actions):e_name(std::move(e_name)),s_name(std::move(s_name)),actions(std::move(actions)){}

std::string &Transition::getE_name() { return e_name; }
std::string &Transition::getS_name() { return s_name; }
std::vector<std::string> &Transition::getActions() { return actions; }