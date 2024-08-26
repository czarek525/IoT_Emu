#include "../Objects/FSM/state.hpp"
#include "../Headers/helper_functions.hpp"

State::State(std::string s_name, std::vector<std::string> on_entry, std::vector<std::string> on_exit, std::unordered_map<std::string, std::shared_ptr<Transition>> ename_transitionptr):s_name(std::move(s_name)),on_entry(std::move(on_entry)),on_exit(std::move(on_exit)),ename_transitionptr(std::move(ename_transitionptr)) {}
std::string &State::getS_name() { return s_name; }
std::vector<std::string> &State::getOn_entry() { return on_entry; }
std::vector<std::string> &State::getOn_exit() { return on_exit; }

std::shared_ptr<Transition> State::getTransition(const std::string& e_name){
    return Helper_functions::getObjectByName(ename_transitionptr, e_name);
}

 