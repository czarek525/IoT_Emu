#include "../Objects/Comp_log/comp_log.hpp"


std::vector<std::shared_ptr<Comp_log>> Comp_log::full_logs;
std::vector<std::shared_ptr<Comp_log>> Comp_log::be_logs;

std::mutex Comp_log::mtx;

std::string Comp_log::getFormattedTime(const std::chrono::system_clock::time_point &currentTime) {
        std::time_t currentTimeT = std::chrono::system_clock::to_time_t(currentTime);
        std::tm currentTimeTM;
        #ifdef _WIN32
            localtime_s(&currentTimeTM, &currentTimeT);
        #else
            localtime_r(&currentTimeT, &currentTimeTM);
        #endif
        std::ostringstream formattedTimeStream;
        formattedTimeStream << std::put_time(&currentTimeTM, "%H:%M:%S") << ":"
                            << std::setfill('0') << std::setw(6)
                            << std::chrono::duration_cast<std::chrono::microseconds>(currentTime.time_since_epoch()).count() % 1000000;
        return formattedTimeStream.str();
    }

void Comp_log::Comp_logCreator(const std::string &name, const unsigned int &pid,
                            const ph_type &ph_value, const std::vector<std::string> &cat, 
                            const std::vector<std::string> &args) {
    std::lock_guard<std::mutex> lock(mtx);
    if (ph_value == ph_type::BE) {
        bool found = false;
        std::shared_ptr<Comp_log> it;
        for (auto &log : Comp_log::be_logs) { //only be_logs search
            if (log->pid == pid && log->name == name && log->cat == cat && log->args == args && log->end.empty()) { //IF the same log incomes, with proper type, than write end time(current)
                log->end = getFormattedTime(std::chrono::system_clock::now());
                found = true;
                it = log;
                break;
            }
        }
        if (!found){
            auto comp_log = std::make_shared<Comp_log>(name, getFormattedTime(std::chrono::system_clock::now()), pid, ph_value, cat, args); //same ptr
            Comp_log::be_logs.push_back(comp_log);
            Comp_log::full_logs.push_back(comp_log);
        }else
            Comp_log::be_logs.erase(std::remove(Comp_log::be_logs.begin(), Comp_log::be_logs.end(), it), Comp_log::be_logs.end()); // to faster search remove find be
    } else
        Comp_log::full_logs.push_back(std::make_shared<Comp_log>(name, getFormattedTime(std::chrono::system_clock::now()), pid, ph_value, cat, args));
}
Comp_log::Comp_log(const std::string &name, const std::string &ts, const unsigned int &pid, 
            const ph_type &ph_value, const std::vector<std::string> &cat, 
            const std::vector<std::string> &args)
    : name(name), ts(ts), pid(pid), ph_value(ph_value), cat(cat), args(args) {}


