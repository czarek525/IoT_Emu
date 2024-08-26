#pragma once
#include "../../Headers/headers.hpp"
class Flow
{
    std::string f_name;
    Flow_type f_type;
    std::vector<float> f_parameters;

public:
    Flow(Flow_type, std::vector<float>,std::string f_name = "");

    static short aflow_number;
    std::string &getF_name();
    Flow_type &getF_type();
    float integralPart;
    float fractionalPart;


    std::vector<float> &getF_parameters();
};