#include "../Objects/Flow/flow.hpp"
short Flow::aflow_number = 1;

Flow::Flow(Flow_type f_type, std::vector<float> f_parameters2, std::string f_name) : f_type(f_type), f_parameters(f_parameters2)
{
    if (f_name.empty())
        this->f_name = "Anonymous Flow " + std::to_string(aflow_number++);
    else
        this->f_name = f_name;
    auto full_interval = f_parameters2.at(1);
    fractionalPart = modff(full_interval, &integralPart) * 100;
};
std::string &Flow::getF_name() { return f_name; }
Flow_type &Flow::getF_type() { return f_type; }
std::vector<float> &Flow::getF_parameters() { return f_parameters; }