#pragma once
#include "../../Headers/headers.hpp"
#include "../Flow/flow.hpp"
#include "../FSM/fsm.hpp"
#include "../Port/port.hpp"
#include "../MQTT_BROKER/mqtt_broker.hpp"

class Component
{
    std::string c_name;
    unsigned int pid;
    std::shared_ptr<MQTT_Broker> MQTT_broker;
    std::unordered_map<std::string, std::shared_ptr<Flow>> fnames_flows;
    std::unordered_map<std::string, std::shared_ptr<Port>> pnames_ports;
    std::unordered_map<std::string, std::shared_ptr<Event>> enames_events;
    std::unordered_map<std::string, std::shared_ptr<Event>> mqtttopic_events;

public:
    mqtt::async_client client_;
    Component(std::string, unsigned int, 
                std::unordered_map<std::string, std::shared_ptr<Event>>, 
                std::unordered_map<std::string, std::shared_ptr<Fsm>>,
                std::unordered_map<std::string, std::shared_ptr<Port>>,
                std::shared_ptr<MQTT_Broker>,
                std::unordered_map<std::string, std::shared_ptr<Flow>>);

    static std::unordered_map<std::string, std::shared_ptr<Component>> cnames_components;
    std::unordered_map<std::string, std::shared_ptr<Fsm>> mnames_fsms;

    std::vector<std::future<void>> futures;
    std::mutex fut_mtx;

    static std::atomic<bool> terminateFlag;
    static std::condition_variable comp_cv;

    void local_message_arrived(std::shared_ptr<Event>);
    void receiveEvent(std::string);
    void handleEventActions(std::string);
    void subscribeEvents();
    void logStateChange(std::shared_ptr<Fsm>);
    void setupServerSocket(const std::shared_ptr<Port> &, bool);
    bool handleClient(bool ,int , const std::string &, sockaddr_in* client_addr = nullptr, socklen_t* client_len = nullptr);
    void setupClientSocket(std::shared_ptr<Port>,bool);
    void startFlow(std::string );
    void cleanupSocket(int &,const std::string& );
    int seconds_until(const std::string& );


    std::shared_ptr<Event> egetEvent(const std::string&);
    std::shared_ptr<Event> mgetEvent(const std::string&);
    std::shared_ptr<Fsm> getFsm(const std::string&);
    static std::shared_ptr<Component> getComponent(const std::string&);


    unsigned int &getPid();
};