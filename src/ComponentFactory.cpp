#include "../Objects/ComponentFactory/ComponentFactory.hpp"
#include "../Headers/helper_functions.hpp"


std::unordered_map<std::string, E_type> etypemap= {
    {"e", E_type::e},
    {"i", E_type::i},
    {"o", E_type::o},
    {"io", E_type::io},
    {"l", E_type::l}};
std::unordered_map<std::string, Flow_type> flowtypemap= {
    {"simple", Flow_type::simple},
    {"on_off", Flow_type::on_off}};
std::unordered_map<std::string, Port_type> porttypemap= {
    {"c", Port_type::c},
    {"s", Port_type::s}};
std::unordered_map<std::string, Transport_type> transportmap= {
    {"T", Transport_type::T},
    {"U", Transport_type::U}};

 /**
 * @brief Find numbers following an exclamation mark in a string.
 *
 * This function searches for numbers that are preceded by an exclamation mark
 * in the given string. For example, in the string "ABC !13 AD!54 53 65",
 * it will return a vector with the values [13, 54].
 *
 * @param input The input string to search.
 * @return A vector of integers representing the numbers found.
 */
std::vector<int> ComponentFactory::findNumbers(const std::string& input) {
    std::vector<int> numbers;
    std::regex pattern("!(\\d+)"); // Regular expression to match !number
    for (std::sregex_iterator it(input.begin(), input.end(), pattern), end; it != end; ++it) {
        // Extract the number from the match and add to the result vector
        int number = std::stoi((*it)[1].str());
        numbers.push_back(number);
    }
    return numbers;
}
 /**
 * @brief This function is used for actions
 *
 * This function from this string: [ename1,ename2,ename3]
 * Returns vector with value ename1,ename2,ename3 
 *
 * @param input The input string to convert.
 * @return A vector of string.
 */
std::vector<std::string> ComponentFactory::stringToVector(std::string& str) {
    std::vector<std::string> result;
    Helper_functions::deleteBrace(str);
    std::stringstream ss(str);
    std::string item;
    while (std::getline(ss, item, ','))
        result.push_back(item);
    return result;
}
/**
 * @brief This function is to check and convert timeout to milliseconds
 *
 * @param value The input timeout to convert
 * Rest params for logs.
 * @return -1 if failure or proper float value.
 */
float ComponentFactory::parseTimeout(const std::string& value, const std::string& c_name, const std::string& pid, const std::string& object) {
    size_t pos = value.find_first_not_of("0123456789.eE+-");
    if (pos == std::string::npos) {
        std::cout << "[" << c_name << " (" << pid << ")] OBJECT (" << object << ") DON'T HAVE UNIT" << std::endl;
        return -1.0f;
    }

    std::string numberStr = value.substr(0, pos);
    std::string unitStr = value.substr(pos);

    try {
        float value = std::stof(numberStr);

        if (unitStr == "s")
            return value * 1000.0f;
        else if (unitStr == "ms")
            return value;
        else{
            std::cout << "[" << c_name << " (" << pid << ")] OBJECT (" << object << ") UNSUPPORTED UNIT FORMAT" << std::endl;
            return -1.0f;
        }
    } catch (const std::exception& e) {
        std::cout << "[" << c_name << " (" << pid << ")] OBJECT (" << object << ") ERROR PARSING NUMBER: " << e.what() << std::endl;
        return -1.0f;
    }
}

std::unordered_map<std::string, std::shared_ptr<Flow>> ComponentFactory::fname_flowsptr;
std::unordered_map<std::string, std::shared_ptr<Event>> ComponentFactory::ename_eventsptr;
std::unordered_map<std::string, std::shared_ptr<Fsm>> ComponentFactory::mname_fsmsptr;
std::unordered_map<std::string, std::shared_ptr<Port>> ComponentFactory::pname_portsptr;
std::shared_ptr<MQTT_Broker> ComponentFactory::MQTT_broker;

void ComponentFactory::eventsCreator(const std::string& value, 
                                    const std::vector<std::string>& list, 
                                    const std::string& c_name, 
                                    const std::string& pid) {
    std::vector<int> events_index = findNumbers(value);
    
    if(events_index.empty())
        std::cerr<<"["<<c_name <<" (" << pid << ")] NO EVENTS FOR COMPONENT"<<std::endl;
    else{
        std::string event, e_name, third_arg;
        E_type type;
        for (int& index: events_index){
                event = list.at(index); // string e_name;type;MQTT_TOPIC OR TIME
                e_name = Helper_functions::getTokenAtIndex(event,0); 
                if (ename_eventsptr.find(e_name) != ename_eventsptr.end()) {
                    std::cerr<<"["<<c_name <<" (" << pid << ")] EVENT (" << e_name << ") DUPLICATE"<<std::endl;
                    continue;
                }
                type = etypemap.at(Helper_functions::getTokenAtIndex(event,1));
                third_arg = Helper_functions::getTokenAtIndex(event,2);
                std::shared_ptr<Event> eventPtr = eventCreator(e_name, type, third_arg, c_name, pid);
                if (eventPtr)
                    ename_eventsptr[e_name] = std::move(eventPtr);
        }
    }
}
//Used to select proper third argument.
std::shared_ptr<Event> ComponentFactory::eventCreator(const std::string& e_name,
                                                    const E_type& type, 
                                                    const std::string& third_arg,
                                                    const std::string& c_name, 
                                                    const std::string& pid){
    if (type != E_type::l )
        return std::make_shared<Event>(e_name,type,third_arg);
    else{
        float timeout = parseTimeout(third_arg,c_name,pid,e_name);
        if (timeout==-1.0f)
            return nullptr;
        
        return std::make_shared<Event>(e_name,static_cast<int>(timeout));
    }
}
void ComponentFactory::fsmsCreator(const std::string& value, 
                                    const std::vector<std::string>& list, 
                                    const std::string& c_name, 
                                    const std::string& pid) {
    std::vector<int> fsms_index = findNumbers(value);
    
    if(fsms_index.empty())
        std::cerr<<"["<<c_name <<" (" << pid << ")] NO FSMS FOR COMPONENT"<<std::endl;
    else{
        std::unordered_map<std::string, std::shared_ptr<State>> sname_statesptr;
        std::string fsm,m_name,initial;
        for (int& index: fsms_index){
                fsm = list.at(index); //m_name;states;initial_state
                m_name = Helper_functions::getTokenAtIndex(fsm,0); 
                // Check the same e_name
                if (mname_fsmsptr.find(m_name) != mname_fsmsptr.end()) {
                    std::cerr<<"["<<c_name <<" (" << pid << ")] FSM (" << m_name << ") DUPLICATE"<<std::endl;
                    continue;
                }
                sname_statesptr = statesCreator(Helper_functions::getTokenAtIndex(fsm,1),list,c_name,pid);
                if(sname_statesptr.empty()){
                    std::cerr<<"["<<c_name <<" (" << pid << ")] NO STATES FOR FSM "<< m_name << std::endl;
                    continue;
                }
                initial = Helper_functions::getTokenAtIndex(fsm,2); 
                mname_fsmsptr[m_name] = std::make_shared<Fsm>(m_name,sname_statesptr,initial);

        }
    }
}
std::unordered_map<std::string, std::shared_ptr<State>> ComponentFactory::statesCreator(const std::string& value, 
                                    const std::vector<std::string>& list, 
                                    const std::string& c_name, 
                                    const std::string& pid) {
    std::vector<int> states_index = findNumbers(value);
    std::unordered_map<std::string, std::shared_ptr<State>> sname_statesptr;
    if(!states_index.empty()){

        std::string state,s_name,on_value_s;
        std::vector<std::string> on_entry,on_exit;
        std::unordered_map<std::string, std::shared_ptr<Transition>> ename_transitionptr;

        for (int& index: states_index){
            state = list.at(index);  // s_name;[on_entry];[on_exit];transition
            s_name = Helper_functions::getTokenAtIndex(state,0); 
            // Check the same s_name
            if (sname_statesptr.find(s_name) != sname_statesptr.end()) {
                std::cerr<<"["<<c_name <<" (" << pid << ")] STATE (" << s_name << ") DUPLICATE"<<std::endl;
                continue;
            }
            on_value_s = Helper_functions::getTokenAtIndex(state,1); //[ename1,ename2...]

            if (on_value_s.empty())
                on_entry = {};
            else
                on_entry = stringToVector(on_value_s);
            on_value_s = Helper_functions::getTokenAtIndex(state,2); //[ename1,ename2...]
            if (on_value_s.empty())
                on_exit = {};
            else
                on_exit = stringToVector(on_value_s);

            ename_transitionptr= transitionsCreator(Helper_functions::getTokenAtIndex(state,3),list,c_name,pid);
            sname_statesptr[s_name] = std::make_shared<State>(s_name,on_entry,on_exit,ename_transitionptr);
        }
    }
    return sname_statesptr;

}
std::unordered_map<std::string, std::shared_ptr<Transition>> ComponentFactory::transitionsCreator(const std::string& value, 
                                    const std::vector<std::string>& list, 
                                    const std::string& c_name, 
                                    const std::string& pid){
    std::vector<int> transitions_index = findNumbers(value);

    std::unordered_map<std::string, std::shared_ptr<Transition>> ename_transitionptr;

    if(!transitions_index.empty()){

        std::string transition, e_name, s_name,actions_s;
        std::vector<std::string> actions;
        
        for (int& index: transitions_index){
            transition = list.at(index); //e_name;s_name;[transition_actions]
            e_name = Helper_functions::getTokenAtIndex(transition,0);
            if (ename_transitionptr.find(e_name) != ename_transitionptr.end()) {
                std::cerr<<"["<<c_name <<" (" << pid << ")] TRANSITION WITH EVENT (" << e_name << ") DUPLICATE"<<std::endl;
                continue;
            }
            s_name = Helper_functions::getTokenAtIndex(transition,1);
            actions_s = Helper_functions::getTokenAtIndex(transition,2); //[ename1,ename2...]
            if (actions_s.empty())
                actions = {};
            else
                actions = stringToVector(actions_s);

            ename_transitionptr[e_name] = std::make_shared<Transition>(e_name,s_name,actions);
        }
    }
    return ename_transitionptr;
}
void ComponentFactory::flowsCreator(const std::string& value, 
                                    const std::vector<std::string>& list, 
                                    const std::string& c_name, 
                                    const std::string& pid) {
    std::vector<int> flows_index = findNumbers(value);

    
    if(flows_index.empty())
        std::cerr<<"["<<c_name <<" (" << pid << ")] NO FLOWS FOR COMPONENT"<<std::endl;
    else{
        std::string flow,f_name,flow_instance_s;
        std::pair<Flow_type, std::vector<float>> flow_instance;
        for (int& index: flows_index){
                flow = list.at(index); // f_name;!number where !number => <flow_anonymous>(always only one)
                f_name = Helper_functions::getTokenAtIndex(flow,0);
                if (ComponentFactory::fname_flowsptr.find(f_name) != ComponentFactory::fname_flowsptr.end()) {
                    std::cerr<<"["<<c_name <<" (" << pid << ")] FLOW (" << f_name << ") DUPLICATE"<<std::endl;
                    continue;
                }
                flow_instance_s = Helper_functions::getTokenAtIndex(flow,1);
                flow_instance_s= list.at(findNumbers(flow_instance_s).at(0)); //Flowtype;Buffersize;interval;on_interval;off_interval(last 2 depends on flowtype)
                flow_instance = ComponentFactory::flowanonymousCreator(flow_instance_s,c_name,pid,f_name);
                if (flow_instance.second.empty())
                    continue;
                ComponentFactory::fname_flowsptr[f_name] = std::make_shared<Flow>(flow_instance.first,flow_instance.second,f_name);
        }
    }
}
void ComponentFactory::portsCreator(const std::string& value, 
                                    const std::vector<std::string>& list, 
                                    const std::string& c_name, 
                                    const std::string& pid) {
    std::vector<int> ports_index = findNumbers(value);

    if(ports_index.empty())
        std::cerr<<"["<<c_name <<" (" << pid << ")] NO PORTS FOR COMPONENT"<<std::endl;
    else{
        std::string port,p_name,m_name,localend,localIP,localPort,remoteend;
        Port_type p_type;
        Transport_type p_transport;
        for (int& index: ports_index){
                port = list.at(index); //p_name;port_type;transport_type;local_end;remote_end;m_name(fsm);stateflow
                p_name = Helper_functions::getTokenAtIndex(port,0); //p_name
                if (pname_portsptr.find(p_name) != pname_portsptr.end()) { 
                    std::cerr<<"["<<c_name <<" (" << pid << ")] PORT (" << p_name << ") DUPLICATE"<<std::endl;
                    continue;
                }
                p_type = porttypemap.at(Helper_functions::getTokenAtIndex(port,1));
                p_transport = transportmap.at(Helper_functions::getTokenAtIndex(port,2));
                if(p_type == Port_type::s){
                    localend = list.at(findNumbers(Helper_functions::getTokenAtIndex(port,3)).at(0)); //local_IP;local_port
                    localIP = Helper_functions::getTokenAtIndex(localend,0); 
                    if (localIP.empty())
                        localIP = "127.0.0.1";
                    localPort = Helper_functions::getTokenAtIndex(localend,1);
                }else{ //for client
                    localIP = "0.0.0.0";
                    localPort = "0";
                }
                remoteend = Helper_functions::getTokenAtIndex(port,4);
                std::shared_ptr<Port> portPtr = createPort(p_name, p_type, p_transport, localIP, std::stoi(localPort), remoteend,port,list,c_name,pid);
                if (!portPtr)
                    continue;
                pname_portsptr[p_name] = std::move(portPtr);
        }
    }
}
std::shared_ptr<Port> ComponentFactory::createPort(const std::string& p_name, 
                                 Port_type p_type, 
                                 Transport_type p_transport, 
                                 const std::string& localIP, 
                                 int localPort, 
                                 std::string& remoteend,
                                 const std::string& port,
                                 const std::vector<std::string>& list,
                                 const std::string& c_name, 
                                 const std::string& pid){
    if (remoteend.empty()) {
        return std::make_shared<Port>(p_name, p_type, p_transport, localIP, localPort);
    } else {
        remoteend = list.at(findNumbers(remoteend).at(0)); //remote_IP;remote_port
        std::string remoteIP = Helper_functions::getTokenAtIndex(remoteend, 0);
        std::string remotePort = Helper_functions::getTokenAtIndex(remoteend, 1);
        std::string m_name = Helper_functions::getTokenAtIndex(port, 5);
        std::unordered_map<std::string, std::shared_ptr<Flow>> sname_flow = stateflowsCreator(Helper_functions::getTokenAtIndex(port, 6), list, c_name, pid);
        if (sname_flow.empty()) {
            std::cerr << "[" << c_name << " (" << pid << ")] NO STATES_FLOWS FOR PORT " << p_name << std::endl;
            return nullptr;
        } 
        return std::make_shared<Port>(p_name, p_type, p_transport, localIP, localPort, std::make_shared<Client_info>(remoteIP, std::stoi(remotePort), m_name, sname_flow));
    }
}

std::unordered_map<std::string, std::shared_ptr<Flow>> ComponentFactory::stateflowsCreator(const std::string& value, 
                                    const std::vector<std::string>& list, 
                                    const std::string& c_name, 
                                    const std::string& pid) {
    std::vector<int> stateflows_index = findNumbers(value);
    std::unordered_map<std::string, std::shared_ptr<Flow>> sname_flow;
    std::string stateflow,s_name,flow_instance_s;
    for (int& index: stateflows_index){
        stateflow = list.at(index); //s_name;f_name or s_name;!number where !number => <flow_anonymous>(always only one)
        s_name = Helper_functions::getTokenAtIndex(stateflow,0);
        flow_instance_s = Helper_functions::getTokenAtIndex(stateflow,1);
        if (findNumbers(flow_instance_s).empty())
            sname_flow[s_name] = ComponentFactory::fname_flowsptr.at(flow_instance_s); //Change to s_name;f_name -> sname_flow map s_name -> flow_ptr
        else{//create new anonymous flow
            std::pair<Flow_type, std::vector<float>> flow_instance;
            flow_instance_s = list.at(findNumbers(flow_instance_s).at(0));//
            flow_instance = flowanonymousCreator(flow_instance_s,c_name,pid); //Flowtype;Buffersize;interval;on_interval;off_interval(last 2 depends on flowtype)
            if (flow_instance.second.empty())
                continue;
            sname_flow[s_name] = std::make_shared<Flow>(flow_instance.first,flow_instance.second);
        }
    }
    return sname_flow;
}
std::pair<Flow_type, std::vector<float>> ComponentFactory::flowanonymousCreator(const std::string& value,
                                                                            const std::string&c_name,
                                                                            const std::string& pid,
                                                                            const std::string& f_name){
    std::pair<Flow_type, std::vector<float>> FlowAnonymous;
    Flow_type f_type;
    float arg1,arg2;
    f_type = flowtypemap.at(Helper_functions::getTokenAtIndex(value,0));
    arg1 = std::stof(Helper_functions::getTokenAtIndex(value,1));
    if (arg1<0){
        std::cerr<<"["<<c_name <<" (" << pid << ")] PROBLEM IN BUFFERSIZE FLOW: "<<f_name<< std::endl;
        return FlowAnonymous;
        
    }
    arg2 = parseTimeout(Helper_functions::getTokenAtIndex(value,2),c_name,pid,f_name);
    if (arg2<0){
        std::cerr<<"["<<c_name <<" (" << pid << ")] PROBLEM IN INTERVAL FLOW: "<<f_name<<std::endl;
        return FlowAnonymous;
    }
    if (f_type == Flow_type::on_off){
        float arg3,arg4;

        arg3 = parseTimeout(Helper_functions::getTokenAtIndex(value,3),c_name,pid,f_name);
        if (arg3<0){
            std::cerr<<"["<<c_name <<" (" << pid << ")] PROBLEM IN ON_INTERVAL FLOW: "<<f_name<<std::endl;
            return FlowAnonymous;
        }
        arg4 = parseTimeout(Helper_functions::getTokenAtIndex(value,4),c_name,pid,f_name);
        if (arg4<0){
            std::cerr<<"["<<c_name <<" (" << pid << ")] PROBLEM IN OFF_INTERVAL FLOW: "<<f_name<<std::endl;
            return FlowAnonymous;
        }
        if (arg1>arg3){
            std::cerr<<"["<<c_name <<" (" << pid << ")] INTERVAL GREATER THAN ON_INTERVAL FLOW: "<<f_name<<std::endl;
            return FlowAnonymous;
        }
        FlowAnonymous.second = std::vector<float>{arg1,arg2,arg3,arg4};

    }else
        FlowAnonymous.second = std::vector<float>{arg1,arg2};

    FlowAnonymous.first = f_type;
    return FlowAnonymous;
}
void ComponentFactory::MQTTbrokerCreator(const std::string& value, 
                                    const std::vector<std::string>& list, 
                                    const std::string& c_name, 
                                    const std::string& pid) {
    std::vector<int> MQTTbroker_index = findNumbers(value);
    
    if(MQTTbroker_index.empty())
        std::cerr<<"["<<c_name <<" (" << pid << ")] NO MQTT BROKER FOR COMPONENT"<<std::endl;
    else{
        std::string MQTTbroker = list.at(MQTTbroker_index.at(0)); //BROKER_IP;BROKER_PORT
        std::string MQTT_BrokerIP, MQTT_BrokerPort; 
        MQTT_BrokerIP = Helper_functions::getTokenAtIndex(MQTTbroker,0);
        MQTT_BrokerPort = Helper_functions::getTokenAtIndex(MQTTbroker,1);
        if (MQTT_BrokerPort.empty())
            MQTT_BrokerPort = "1883";
        MQTT_broker = std::make_shared<MQTT_Broker>(MQTT_BrokerIP,MQTT_BrokerPort);
    }
}
std::shared_ptr<Component> ComponentFactory::componentCreator(const std::string& c_name,const std::string& pid) {
    Flow::aflow_number = 1;
    return std::make_shared<Component>(c_name,stoi(pid),std::move(ename_eventsptr),std::move(mname_fsmsptr),std::move(pname_portsptr),std::move(MQTT_broker),std::move(fname_flowsptr));
}
