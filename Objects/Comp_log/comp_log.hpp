#pragma once
#include "../../Headers/headers.hpp"

class Comp_log
{
public:
    std::string name, ts, end;
    unsigned int pid;
    ph_type ph_value;
    std::vector<std::string> cat, args;
    static std::mutex mtx;

    Comp_log(const std::string &, const std::string &, const unsigned int &, 
            const ph_type &,const std::vector<std::string> &,
            const std::vector<std::string> & args = {});
    static std::vector<std::shared_ptr<Comp_log>>  full_logs;
    static std::vector<std::shared_ptr<Comp_log>>be_logs;

    static std::string getFormattedTime(const std::chrono::system_clock::time_point &currentTime);
    static void Comp_logCreator(const std::string &name, const unsigned int &pid, 
                                const ph_type &ph_value, const std::vector<std::string> &cat, 
                                const std::vector<std::string> &args = {});
};
