#pragma once
#include "../Event/event.hpp"
#include "../FSM/transition.hpp"
#include "../FSM/state.hpp"
#include "../FSM/fsm.hpp"
#include "../Flow/flow.hpp"
#include "../Port/port.hpp"
#include "../MQTT_BROKER/mqtt_broker.hpp"
#include "../Component/component.hpp"


class ComponentFactory {
        static std::vector<int> findNumbers(const std::string& );
        static std::vector<std::string> stringToVector(std::string& );
        static float parseTimeout(const std::string& , const std::string& , const std::string& , const std::string& );
    public:
        static std::unordered_map<std::string, std::shared_ptr<Flow>> fname_flowsptr;
        static std::unordered_map<std::string, std::shared_ptr<Event>> ename_eventsptr;
        static std::unordered_map<std::string, std::shared_ptr<Fsm>> mname_fsmsptr;
        static std::unordered_map<std::string, std::shared_ptr<Port>> pname_portsptr;
        static std::shared_ptr<MQTT_Broker> MQTT_broker;

        static void eventsCreator(const std::string& value, 
                                const std::vector<std::string>& list, 
                                const std::string& c_name, 
                                const std::string& pid);
        static std::shared_ptr<Event> eventCreator(const std::string &e_name,
                                                   const E_type &type,
                                                   const std::string &third_arg,
                                                   const std::string &c_name,
                                                   const std::string &pid);
        static void fsmsCreator(const std::string& value, 
                                const std::vector<std::string>& list, 
                                const std::string& c_name, 
                                const std::string& pid);
        static std::unordered_map<std::string, std::shared_ptr<State>> statesCreator(const std::string& value, 
                                const std::vector<std::string>& list, 
                                const std::string& c_name, 
                                const std::string& pid);
        static std::unordered_map<std::string, std::shared_ptr<Transition>> transitionsCreator(const std::string& value, 
                                        const std::vector<std::string>& list, 
                                        const std::string& c_name, 
                                        const std::string& pid);
        static void flowsCreator(const std::string& value, 
                                const std::vector<std::string>& list, 
                                const std::string& c_name, 
                                const std::string& pid);  

        static void portsCreator(const std::string& value, 
                                const std::vector<std::string>& list, 
                                const std::string& c_name, 
                                const std::string& pid);  
        static std::shared_ptr<Port> createPort(const std::string& p_name, 
                                    Port_type p_type, 
                                    Transport_type p_transport, 
                                    const std::string& localIP, 
                                    int localPort, 
                                    std::string& remoteend,
                                    const std::string& port,
                                    const std::vector<std::string>& list,
                                    const std::string& c_name, 
                                    const std::string& pid);
        static std::unordered_map<std::string, std::shared_ptr<Flow>> stateflowsCreator(const std::string& value, 
                                        const std::vector<std::string>& list, 
                                        const std::string& c_name, 
                                        const std::string& pid);      
        static std::pair<Flow_type, std::vector<float>> flowanonymousCreator(const std::string& value,
                                                                                const std::string&c_name,
                                                                                const std::string& pid,
                                                                                const std::string& f_name="Anonymous Flow");
        static void MQTTbrokerCreator(const std::string& value, 
                                        const std::vector<std::string>& list, 
                                        const std::string& c_name, 
                                        const std::string& pid);
        static std::shared_ptr<Component> componentCreator(const std::string& c_name,const std::string& pid);
};