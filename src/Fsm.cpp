#include "../Objects/FSM/fsm.hpp"
#include "../Headers/helper_functions.hpp"

Fsm::Fsm(std::string m_name, std::unordered_map<std::string, std::shared_ptr<State>> sname_statesptr, std::string initial):m_name(std::move(m_name)),sname_statesptr(std::move(sname_statesptr)),changeRequested(false), s_name(std::move(initial)){}

std::string &Fsm::getM_name() { return m_name; }
std::string Fsm::getS_name() {         
    std::lock_guard<std::mutex> lock(mtx);
    return s_name; 
    }
void Fsm::setS_name(std::string s_name)
{
    {
        std::lock_guard<std::mutex> lock(mtx);
        this->s_name = s_name;
    }
    
    changeRequested.store(true);
}

 std::shared_ptr<State> Fsm::getState(const std::string& s_name){
    return Helper_functions::getObjectByName(sname_statesptr, s_name);
}
