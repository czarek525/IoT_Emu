#pragma once
#include "../Objects/Event/event.hpp"
#include "../Objects/Comp_log/comp_log.hpp"
#include "../Objects/Component/component.hpp"
/**
 * @brief This class is designed to handle incoming MQTT messages and local events.
 *
 * 
 * To capture MQTT messages, inheritance from the callback class is required.
 * 
 */
class ReceiveCallback : public virtual mqtt::callback
{
public:
    std::string c_name;
    static std::vector<std::shared_ptr<ReceiveCallback>> activecallbacks;
    ReceiveCallback(const std::string& c_name):c_name(c_name){};
    /**
     * @brief This method handles incoming MQTT messages and checks if they are events available on component.
     * 
     * The method checks the available events for specific component, 
     * 
     * and if it finds an event with a matching topic with proper type(input-output('io') or input('i')),
     * 
     * it is further processed.
     * 
     */
    void message_arrived(mqtt::const_message_ptr msg) override
    {
        
        std::shared_ptr<Event> eventPointer;
        std::vector<std::string> cat;
        auto comp =Component::getComponent(c_name); // takes component ptr from c_name(related to callback)
        if(eventPointer = comp->mgetEvent(msg->get_topic())){ //Takes event ptr by mqtt_topic
            if (eventPointer->getType() == E_type::io || eventPointer->getType() == E_type::i)
                cat = {"event", "app", "event_rcv"};
            else if (eventPointer->getType() == E_type::e)
                cat = {"event", "env", "event_rcv"};
            Comp_log::Comp_logCreator(
                eventPointer->getE_name(), 
                comp->getPid(), 
                ph_type::I, 
                cat
            );
            //Proccess event
            comp->receiveEvent(eventPointer->getE_name());
        }
    }


};