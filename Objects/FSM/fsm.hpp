#pragma once
#include "state.hpp"
class Fsm
{
    std::string m_name, s_name;
    std::mutex mtx;
    std::unordered_map<std::string, std::shared_ptr<State>> sname_statesptr;

public:
    std::condition_variable cv;

    std::atomic<bool> changeRequested;
    Fsm(std::string, std::unordered_map<std::string, std::shared_ptr<State>>, std::string);

    std::string &getM_name(), getS_name();
    std::mutex cv_mtx;
    std::shared_ptr<State> getState(const std::string& );
    void setS_name(std::string);
};