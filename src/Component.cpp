#include "../Headers/ReceiveCallback.hpp"
#include "../Headers/helper_functions.hpp"

std::unordered_map<std::string, std::shared_ptr<Component>> Component::cnames_components;
std::vector<std::shared_ptr<ReceiveCallback>> ReceiveCallback::activecallbacks;
std::condition_variable Component::comp_cv;
std::atomic<bool> Component::terminateFlag;
Component::Component(std::string c_name, unsigned int pid, 
                std::unordered_map<std::string, std::shared_ptr<Event>> events, 
                std::unordered_map<std::string, std::shared_ptr<Fsm>> fsms,
                std::unordered_map<std::string, std::shared_ptr<Port>> ports,
                std::shared_ptr<MQTT_Broker> MQTT_broker,
                std::unordered_map<std::string, std::shared_ptr<Flow>> flows) : client_(MQTT_broker->getEndpoint_IP() + ":" + MQTT_broker->getEndpoint_port(),c_name),
                c_name(c_name),pid(pid),enames_events(std::move(events)),mnames_fsms(fsms),pnames_ports(std::move(ports)),MQTT_broker(MQTT_broker),fnames_flows(std::move(flows)) {
    for (auto &fsm : fsms) // When the component is created, log the FSM's initial state
    {
            Comp_log::Comp_logCreator(
                fsm.second->getM_name(),
                this->pid,
                ph_type::BE, 
                {"state"}, 
                {fsm.second->getS_name()});
    
    }
}
unsigned int &Component::getPid() { return pid; }

std::shared_ptr<Event> Component::egetEvent(const std::string& e_name) {
        return Helper_functions::getObjectByName(enames_events, e_name);
}
    
std::shared_ptr<Component> Component::getComponent(const std::string& c_name) {
        return Helper_functions::getObjectByName(cnames_components, c_name);
}
    
std::shared_ptr<Event> Component::mgetEvent(const std::string& mqtt_topic) {
        return Helper_functions::getObjectByName(mqtttopic_events, mqtt_topic);
}
    
std::shared_ptr<Fsm> Component::getFsm(const std::string& m_name) {
        return Helper_functions::getObjectByName(mnames_fsms, m_name);
}


/**
 * @brief Handles actions associated with an event.
 *
 * This method performs actions related to a specific event depending on its type.
 * 
 * Input-output ('io') and output ('o') events cause the publication of messages.
 * 
 * Local events ('l') trigger an event after a specified time.
 * 
 * Additional info:
 * 
 * `egetEvent` retrieves a pointer from an unordered map based on the event name (action).
 * 
 * `futures` is a vector containing every newly started thread in the program for all components.
 * 
 * @param action The name of the event to be processed.
 */
void Component::handleEventActions(const std::string action)
{
    std::shared_ptr<Event> eventPointer;

        if (eventPointer = egetEvent(action)){
            if (eventPointer->getType() == E_type::io || eventPointer->getType() == E_type::o)
            {
                try {
                    this->client_.publish(mqtt::make_message(eventPointer->getMqtt_e_name(), action));
                }
                catch (const mqtt::exception& e)
                {
                    std::cout<<"["<<c_name <<" (" << pid << ")] MQTT publish exception event: " << eventPointer->getE_name() << std::endl;
                }
                Comp_log::Comp_logCreator(
                    eventPointer->getE_name(),
                    this->pid,
                    ph_type::I,
                    {"event", "app", "event_snd"}
                );
            }
            else if (eventPointer->getType() == E_type::l)
            {
                {
                    // Starting a thread with a local event (requires protection), cannot be ended by terminateFlag.
                    std::lock_guard<std::mutex> lock(fut_mtx);
                    futures.push_back(std::async(std::launch::async, &Component::local_message_arrived, this,eventPointer));
                }
                Comp_log::Comp_logCreator(
                    eventPointer->getE_name(),
                    this->pid,
                    ph_type::I,
                    {"event", "local", "event_snd"}
                );
            }
        }
}
/**
* @brief This method handles local events
* 
* Method need to use cv - to sleep and to wake up when teminate flag set.
*
*@param eventPointer Incoming local event to component
*/
void Component::local_message_arrived(std::shared_ptr<Event> eventPointer)
{
    {
        std::unique_lock<std::mutex> lock(fut_mtx);
        if (comp_cv.wait_for(lock, std::chrono::milliseconds(eventPointer->getTimeout()), [] {return terminateFlag.load();})){
            std::cout<<"["<<c_name <<" (" << pid << ")] Terminate local event "<< eventPointer->getE_name()<<std::endl;
                return;
        }
    }
    Comp_log::Comp_logCreator(
            eventPointer->getE_name(),
                getPid(), 
                ph_type::I, 
                {"event", "local", "event_rcv"}
    );
    receiveEvent(eventPointer->getE_name());
}
/**
 * @brief Logs a state change for a Finite State Machine (FSM).
 *
 * This method creates a log entry indicating a change in the FSM's state.
 * 
 * @param fsm The FSM whose state change is being logged.
 */
void Component::logStateChange(std::shared_ptr<Fsm> fsm) {
    Comp_log::Comp_logCreator(
        fsm->getM_name(), 
        this->pid, 
        ph_type::BE, 
        {"state"}, 
        {fsm->getS_name()}
    );
}
/**
 * @brief Handles an incoming event.
 *
 * This method checks each Finite State Machine (FSM) to determine if the occurrence of an event affects a state change (transition). 
 * If not, it proceeds to the next FSM.
 * 
 * Additional info:
 * 
 * `mnames_fsms` is an unordered map that correlates FSM names with pointers to FSM objects.
 * 
 * `getTransition` returns a pointer to the transition associated with the event name.
 * 
 * `getState` returns a pointer to the state associated with the state name.
 * 
 * @param e_name The name of the event.
 */
void Component::receiveEvent(const std::string e_name)
{
    std::shared_ptr<Transition> transitionPointer;
    for (auto &fsmEntry : this->mnames_fsms)
    {
        std::shared_ptr<Fsm> fsm = fsmEntry.second;
        std::shared_ptr<State> state = fsm->getState(fsm->getS_name());

        // Check if a transition exists for the current state or continue to the next FSM
        if (!(transitionPointer = state->getTransition(e_name)))
            continue;

        // Exit actions from the state
        auto on_exit_actions = std::async(std::launch::async, [this, state]() {
            for (auto &action : state->getOn_exit())
                handleEventActions(action);
        });
        
        // Log the state change before exiting the state(to end)
        logStateChange(fsm);

        // Wait for exit actions to complete before performing the transition
        on_exit_actions.get();

        // Actions for transitioning between states
        auto transit_actions = std::async(std::launch::async, [this, transitionPointer]() {
            for (auto &action : transitionPointer->getActions())
                handleEventActions(action);
        });

        // Set the new state
        fsm->setS_name(transitionPointer->getS_name());

        // Notify about the state change (ports with correlated FSMs will see the change)
        fsm->cv.notify_all();
        
        // Log the new state
        logStateChange(fsm);

        // Wait for transition actions to complete before continuing
        transit_actions.get();

        // Start on-entry actions (and ensure they complete before a potential new state change)
        std::async(std::launch::async, [this, fsm]() {
            for (auto &action : fsm->getState(fsm->getS_name())->getOn_entry())
                handleEventActions(action);
        }).get();
    }
}

/**
 * @brief Subscribes to events for the component.
 *
 * This method subscribes to specific events (e, i, or io) for the component.
 * 
 * Additional info:
 * 
 * `enames_events` is an unordered map that correlates event names with pointers to event objects.
 */
void Component::subscribeEvents()
{
    // Create a callback with c_name (for better search performance for incoming MQTT messages)
    auto cb = std::make_shared<ReceiveCallback>(c_name);

    // Add this new callback to the vector of active callbacks (to handle incoming MQTT messages)
    ReceiveCallback::activecallbacks.push_back(cb);

    // Assign callback to the client_ (each Component has one client_ and this has one callback)
    client_.set_callback(*cb);
    
    // If the component loses connection, it will lose information about previous events and try to reconnect
    mqtt::connect_options connOpts;
    connOpts.set_clean_session(true);
    connOpts.set_automatic_reconnect(true);

    // Lost connection can be noticed in the logs using a will message
    const std::string WILL_TOPIC {"ERROR CONNECTION"};
    const std::string WILL_PAYLOAD {"Connection lost pid: " + std::to_string(pid)};
    const int QoS(1);
    mqtt::will_options will_msg(WILL_TOPIC, WILL_PAYLOAD, QoS, false);
    connOpts.set_will(will_msg);

    // Check if the component has any events to subscribe to
    bool hasActiveSubscriptions = false;

    std::cout << "[" << c_name << " (" << pid << ")] Connecting to the MQTT server...\n";
    // Try to connect with the broker and wait until the connection is established
    try
    {
        this->client_.connect(connOpts)->wait();
    }
    catch (const mqtt::exception &exc)
    {
        std::cerr << "[" << c_name << " (" << pid << ")] Connection error: " << exc.what() << std::endl;
        return;
    }

    // Subscribe to specific topics
    for (const auto& event_pair : this->enames_events)
    {
        std::shared_ptr<Event> event = event_pair.second;
        if (event->getType() == E_type::e || event->getType() == E_type::i || event->getType() == E_type::io)
        {
            mqtttopic_events[event->getMqtt_e_name()] = event; // Create map for fast searching in the future when a message arrives
            hasActiveSubscriptions = true;
            std::cout << "[" << c_name << " (" << pid << ")] Subscribing to topic '" << event->getMqtt_e_name() << "'\n";
            try
            {
                this->client_.subscribe(event->getMqtt_e_name(), QoS)->wait();
            }
            catch (const std::exception& exc)
            {
                std::cerr << "[" << c_name << " (" << pid << ")] Subscription error for event: " << event->getE_name() << std::endl;
            }
        }
    }

    // Client will start consuming when the client port starts working
    client_.stop_consuming();

    // Remove callback if no topics are subscribed to
    if (!hasActiveSubscriptions)
        ReceiveCallback::activecallbacks.pop_back();
}


/**
 * @brief Closes an open socket.
 *
 * This method closes an open socket.
 * 
 * Additional info:
 * 
 * Different closing methods are used depending on the system.
 * 
 * @param sockfd The socket file descriptor to be closed.
 * @param p_name The port name for logging purposes.
 */
void Component::cleanupSocket(int &sockfd,const std::string& p_name)
{
    std::cout<<"["<<c_name <<" (" << pid << ")] Closing port "<<p_name<< std::endl;
    #ifdef _WIN32
        if (closesocket(sockfd) == SOCKET_ERROR)
            std::cerr<<"["<<c_name <<" (" << pid << ")] error closing socket "<<p_name<< std::endl;
    #else
        if (close(sockfd) == -1)
            std::cerr<<"["<<c_name <<" (" << pid << ")] error closing socket "<<p_name<< std::endl;
    #endif
}
/**
 * @brief Sets up a server socket.
 *
 * This method opens a server socket (TCP or UDP).
 * 
 * Additional info:
 * 
 * The server socket can handle multiple TCP sessions and multiple TCP/UDP messages simultaneously.
 * It can also handle new sessions during active TCP connections.
 * 
 * @param port The port pointer with necessary information about the socket.
 * @param listenFlag Indicates whether the socket should be set up for TCP (true) or UDP (false).
 */
void Component::setupServerSocket(const std::shared_ptr<Port> &port, bool listenFlag)
{
    int server_socket;
    if (listenFlag)
        server_socket = socket(AF_INET, SOCK_STREAM, 0); // Create a TCP socket if listening
    else
        server_socket = socket(AF_INET, SOCK_DGRAM, 0); // Create a UDP socket if not listening
    
    auto p_name = port->getP_name();
    if (server_socket < 0)
    {
        std::cerr << "[" << c_name << " (" << pid << ")] Server " << p_name << " error creating server socket" << std::endl;
        return;
    }

    sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    auto local_port = port->getLocal_port();
    // Convert port number to network byte order
    server_addr.sin_port = htons(local_port); 
    sockaddr_in client_addr = {};
    socklen_t client_len = sizeof(client_addr);

    fd_set readfds;
    
    // For multiple TCP connections, handle clients in separate threads
    std::mutex client_mtx;
    std::vector<std::future<void>> client_futures;
   
    auto local_IP = port->getLocal_IP();

    // Convert IP address to network byte order
    if (inet_pton(AF_INET, local_IP.c_str(), &server_addr.sin_addr) <= 0) 
    {
        std::cerr << "[" << c_name << " (" << pid << ")] Server " << p_name << " invalid address or address not supported" << std::endl;
        cleanupSocket(server_socket, p_name);
        return;
    }

    if (bind(server_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) < 0)
    {
        std::cerr << "[" << c_name << " (" << pid << ")] Server " << p_name << " bind failed" << std::endl;
        cleanupSocket(server_socket, p_name);
        return;
    }
    if (listenFlag && listen(server_socket, SOMAXCONN) < 0)
    {
        std::cerr << "[" << c_name << " (" << pid << ")] Server " << p_name << " listen failed" << std::endl;
        cleanupSocket(server_socket, p_name);
        return;
    }

    int client_socket;

    // Use select to block for a limited time (can check terminateFlag)
    while (!terminateFlag.load())
    {
        FD_ZERO(&readfds);
        FD_SET(server_socket, &readfds);
        
        // Block for 5 seconds
        struct timeval timeout;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        // Check if there is activity on the socket (potential new tcp connection or UDP datagram)
        int activity = select(server_socket + 1, &readfds, nullptr, nullptr, &timeout);

        if (activity < 0)
        {
            std::cerr << "[" << c_name << " (" << pid << ")] Server " << p_name << " select error" << std::endl;
            cleanupSocket(server_socket, p_name);
            return;
        }
        // If activity detected, process it
        if (FD_ISSET(server_socket, &readfds))
        {
            if (listenFlag)
            {
                client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len); // Accept new TCP connection (non-persistance mode)
                if (client_socket < 0)
                {
                    std::cerr << "[" << c_name << " (" << pid << ")] Server " << p_name << " accept failed" << std::endl;
                    continue;
                }
                else
                    std::cout << "[" << c_name << " (" << pid << ")] Server " << p_name << " accepted connection" << std::endl;

                // Create a new thread for the new TCP client (can end when client disconnects)
                {
                    std::lock_guard<std::mutex> lock(client_mtx);
                    client_futures.push_back(std::async(std::launch::async, [this, client_socket, p_name]() {
                        while (!terminateFlag.load()) { // Maintain connection while client is in persistent mode
                            if (handleClient(false, client_socket, p_name))
                                break;
                        }
                    }));
                }

            }
            else
                handleClient(true, server_socket, p_name, &client_addr, &client_len); // Handle UDP client without threading
        }
    }
    // Clean up client threads after termination
    if (listenFlag)
    {
        std::lock_guard<std::mutex> lock(client_mtx);
        for (auto &client : client_futures)
            if (client.valid())
                client.get();
    }
    // Close the server socket
    cleanupSocket(server_socket, p_name);               
}
/**
 * @brief Handles communication with a UDP/TCP client.
 *
 * This method processes data received from a client.
 * 
 * @param isUDP Indicates whether the connection is UDP (true) or TCP (false).
 * @param socket The server socket.
 * @param p_name The port name for logging purposes.
 * @param client_addr Optional client address (used for logging but not currently used).
 * @param client_len Optional client length(Determiantes IPv4 or IPv6) (used for logging but not currently used).
 * @return Returns true if the server should disconnect from the client.
 */
bool Component::handleClient(bool isUDP, int socket, const std::string &p_name, sockaddr_in* client_addr, socklen_t* client_len) {
    
    // Specify a buffer size for receiving data
    std::vector<char> buffer(1024);
    int bytes_received;

    // Receive data depending on whether the connection is UDP or TCP
    if (isUDP)
        bytes_received = recvfrom(socket, buffer.data(), buffer.size(), 0, reinterpret_cast<sockaddr *>(client_addr), client_len);
    else
        bytes_received = recv(socket, buffer.data(), buffer.size(), 0);


    if (bytes_received < 0)
        std::cerr << "[" << c_name << " (" << pid << ")] Server " << p_name << " error receiving data" << std::endl; // Can also indicate client stop or failure
    else if (bytes_received == 0 && !isUDP) {
        std::cout << "[" << c_name << " (" << pid << ")] Server " << p_name << " client disconnected" << std::endl; 
        cleanupSocket(socket, "client port");
        return true;  
    } else {
        // Log the received data
        std::string packet_data(buffer.data(), bytes_received);
        Comp_log::Comp_logCreator(
            p_name,
            pid,
            ph_type::I,
            {"port", "packet", "packet_rcv"},
            {isUDP ? "UDP" : "TCP", packet_data}
        );
    }
    return false;
}

/**
 * @brief Sets up a client socket.
 *
 * This method opens a client socket (TCP or UDP).
 * 
 * Additional info:
 * 
 * TCP works in persistent mode (one connection only).
 * 
 * `getFlow` returns a flow pointer for the given state name.
 * 
 * `getFsm` returns an FSM pointer for the given FSM name.
 * 
 * Flow types are used only for predefined buffer sizes and times between transport packets.
 * 
 * The message value is a random number uniformly distributed within the buffer size, expressed in bytes.
 * 
 * @param port The port pointer with necessary information about the socket.
 * @param listenFlag Indicates whether the socket should be set up for TCP (true) or UDP (false).
 */
void Component::setupClientSocket(std::shared_ptr<Port> port, bool listenFlag) {
    int client_socket;
    if (listenFlag)
        client_socket = socket(AF_INET, SOCK_STREAM, 0);
    else
        client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    auto p_name = port->getP_name();

    if (client_socket == -1) {
        std::cerr<<"["<<c_name <<" (" << pid << ")] Client "<<p_name<<" error reating client socket" << std::endl;
        return;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;

    //Client info have information about neccessary informations 
    auto client_info = port->getClient_info();
    // Convert port to network format
    server_addr.sin_port = htons(client_info->getRemote_port());

    //Convert IP address to network format
    if (inet_pton(AF_INET, client_info->getRemote_IP().c_str(), &server_addr.sin_addr) <= 0) {
        std::cerr<<"["<<c_name <<" (" << pid << ")] Client "<<p_name<<" invalid address or address not supported" << std::endl;
        cleanupSocket(client_socket,p_name);
        return;
    }
    // Loop for connecting to server(1 second delay between connection).
    if (listenFlag) {
        while (connect(client_socket, reinterpret_cast<struct sockaddr *>(&server_addr), sizeof(server_addr)) < 0 && !terminateFlag.load()) {
            std::cerr<<"["<<c_name <<" (" << pid << ")] Client "<<p_name<<" failed to connect to server (try again in 1 second)" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        std::cout<<"["<<c_name <<" (" << pid << ")] Client "<<p_name<<" connect to Server" << std::endl;
    }
    //Interval - time between transport packets
    std::chrono::milliseconds interval,on_interval,off_interval;
    // Variables for generating random value
    std::minstd_rand generator(std::random_device{}()); 
    std::uniform_int_distribution<unsigned int> distribution;
    std::shared_ptr<Flow> actualFlow;     //Actual Flow pointer use in program
    std::vector<std::string> args;     // Args use in logs
    std::vector<char> buffer;  
    bool on_state; //Actual state(for SIMPLE Flow always true)
    unsigned int randomvalue;
    std::chrono::steady_clock::time_point start, now; // Use to compute on/off time
    auto fsm = getFsm(client_info->getM_name()); // Fsm which control flow
    std::unique_lock<std::mutex> lock(fsm->cv_mtx); // Lock mutex(need for cv) 
    while (!terminateFlag) {
        auto flow = client_info->getFlow(fsm->getS_name());  //Take actual flow
        if (!flow){ //Check new flow(if nullptr error)
            std::cerr<<"["<<c_name <<" (" << pid << ")] Client "<<p_name<<" have invalid flow.\n";
            break;
        }
        if (flow != actualFlow) { 
            if (actualFlow) { // Log for previous flow(to end)
                Comp_log::Comp_logCreator(
                    actualFlow->getF_name(), 
                    this->pid, 
                    ph_type::BE, 
                    {"port", "flow"}, 
                    args
                );
            }
            actualFlow = std::move(flow); 
            if (actualFlow->getF_type() == Flow_type::simple) {
                args = {"simple", fsm->getM_name(), std::to_string(actualFlow->getF_parameters().at(0)),std::to_string(actualFlow->getF_parameters().at(1))};
                Comp_log::Comp_logCreator(
                    actualFlow->getF_name(), 
                    this->pid, 
                    ph_type::BE, 
                    {"port", "flow"}, 
                    args
                );  
            }else{
                args = {"on_off", fsm->getM_name(), std::to_string(actualFlow->getF_parameters().at(0)),std::to_string(actualFlow->getF_parameters().at(1)),std::to_string(actualFlow->getF_parameters().at(2)),std::to_string(actualFlow->getF_parameters().at(3))};
                Comp_log::Comp_logCreator(
                    actualFlow->getF_name(), 
                    this->pid, 
                    ph_type::BE, 
                    {"port", "flow"}, 
                    args
                    
                );
                start = std::chrono::steady_clock::now();
                on_interval = std::chrono::milliseconds(static_cast<int>(actualFlow->getF_parameters().at(2)));
                off_interval = std::chrono::milliseconds(static_cast<int>(actualFlow->getF_parameters().at(3)));
            }
            buffer.resize(actualFlow->getF_parameters().at(0),0);
            interval = std::chrono::milliseconds(static_cast<int>(actualFlow->integralPart));
            on_state = true;
        }
        distribution.param(std::uniform_int_distribution<unsigned int>::param_type(0, actualFlow->getF_parameters().at(0)));
        randomvalue = distribution(generator);

        if (actualFlow->getF_type() == Flow_type::on_off){
            now = std::chrono::steady_clock::now();
            if (now - start > on_interval && on_state)
            {
                on_state = false;
                start = now;
            }
            else if (now - start > off_interval && !on_state)
            {
                on_state = true;
                start = now;
            }
        }
        #ifdef _WIN32
            timeBeginPeriod(1);
        #endif
        // Thread go sleep for an interval(to send packets in proper time), but can be wake up by change of FSM state(when cv is used lock is unlock for that time), or because of termination. To wake up is need to use cv.notify 
        if (fsm->cv.wait_for(lock, interval+std::chrono::microseconds(static_cast<int>(actualFlow->fractionalPart)), [&fsm] {
                    return fsm->changeRequested.load()|| terminateFlag.load();
                })) {
                fsm->changeRequested.store(false); // When changerequest or terminateflag, thread see this and store false 
                continue; // fsm was change so shouldn send next packet
        }
        // Fill buffer with data
        std::string msg = std::to_string(randomvalue);
        std::fill(buffer.begin(), buffer.end(), 0); 
        std::copy(msg.begin(), msg.end(), buffer.begin());
        if (on_state){ // Send only in proper state.(on/off feature)
            Comp_log::Comp_logCreator(
                p_name, 
                this->pid, 
                ph_type::I, 
                {"port", "packet", "packet_snd"}, 
                {listenFlag ? "TCP" : "UDP", fsm->getM_name(), fsm->getS_name(), std::to_string(randomvalue)}
            );

            int sent_bytes;
            if (listenFlag)
                sent_bytes = send(client_socket, buffer.data(), buffer.size(), 0);
            else
                sent_bytes = sendto(client_socket, buffer.data(), buffer.size(), 0, reinterpret_cast<const sockaddr *>(&server_addr), sizeof(server_addr));
            
            if (sent_bytes == -1)
                std::cerr<<"["<<c_name <<" (" << pid << ")] Client "<<p_name<<" error sending data" << std::endl;
        }
        #ifdef _WIN32
            timeEndPeriod(1);
        #endif
    }
    // Close client socket
    cleanupSocket(client_socket,p_name);
}

/**
 * @brief Computes the number of seconds until a specified time.
 *
 * This method calculates the number of seconds remaining until a target time based on the current system time.
 * 
 * @param target_time_str The target time as a string in the format h:m:s.
 * @return The number of seconds until the target time.
 */
int Component::seconds_until(const std::string& target_time_str) {
    // Get current time as time_t and convert to struct tm
    auto now_tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm now_tm;
    
    #ifdef _WIN32
        if (localtime_s(&now_tm, &now_tt) != 0) {
            std::cerr << "[" << c_name << " (" << pid << ")] Error converting time\n";
            return -1; // Return error code
        }
    #else
        if (!localtime_r(&now_tt, &now_tm)) {
            std::cerr << "[" << c_name << " (" << pid << ")] Error converting time\n";
            return -1; // Return error code
        }
    #endif

    // Copy current time to target time struct
    std::tm target_tm = now_tm;

    // Parse target time
    std::istringstream ss(target_time_str);
    if (!(ss >> std::get_time(&target_tm, "%H:%M:%S"))) {
        std::cerr << "[" << c_name << " (" << pid << ")] Error parsing target time string\n";
        return -1; // Return error code
    }

    // Calculate seconds from start of the day for both times
    int now_seconds = now_tm.tm_hour * 3600 + now_tm.tm_min * 60 + now_tm.tm_sec;
    int target_seconds = target_tm.tm_hour * 3600 + target_tm.tm_min * 60 + target_tm.tm_sec;

    return target_seconds - now_seconds;
}

/**
 * @brief Starts the flow for the component.
 *
 * This method starts new threads for each port in the component.
 * 
 * @param target_time_str Specifies the time when client ports should start operating (in the format h:m:s).
 */
void Component::startFlow(const std::string target_time_str)
{
    // Need to lock because futures is multithread variable.
    std::unique_lock<std::mutex> lock(fut_mtx);
    for (auto& port : pnames_ports){
        if (port.second->getP_type() == Port_type::s)
        {
            if (port.second->getP_transport() == Transport_type::U)
                futures.push_back(std::async(std::launch::async, &Component::setupServerSocket, this,port.second,false));
            else if (port.second->getP_transport() == Transport_type::T)
                futures.push_back(std::async(std::launch::async, &Component::setupServerSocket, this,port.second,true));
        }
    }
    

    
    std::cout<<"["<<c_name <<" (" << pid << ")] Client will start operating at "<<target_time_str<< std::endl;
    // Go sleep for specify time or can be terminate.
    if (comp_cv.wait_for(lock, std::chrono::seconds(seconds_until(target_time_str)), [] {return terminateFlag.load();})){
        std::cout<<"["<<c_name <<" (" << pid << ")] Terminate starting clients\n";
            return;
    }
    // Start mqtt client
    client_.start_consuming();

    for (auto& port : pnames_ports)
    {
        if (port.second->getP_type() == Port_type::c){
            if (port.second->getP_transport() == Transport_type::U)
                futures.push_back(std::async(std::launch::async, &Component::setupClientSocket, this,port.second,false));
            else if (port.second->getP_transport() == Transport_type::T)
                futures.push_back(std::async(std::launch::async, &Component::setupClientSocket, this,port.second,true));
        }
    }
}