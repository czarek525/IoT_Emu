#pragma once
#include <fstream>
#include <vector>
#include <atomic>
#include <unordered_map>
#include <iomanip>
#include <regex>
#include <future>
#include <cmath>
#include <random>
#include <mutex>
#include <sstream>
#include <condition_variable>
#include "mqtt/async_client.h"
#ifdef _WIN32
    #include <WinSock2.h>
    #include <WS2tcpip.h>
    #include <windows.h>

#else
    #include <dirent.h>
    #include <sys/select.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif


enum ph_type
{
    I,
    BE,
    M
};
enum E_type
{
    e,
    i,
    o,
    io,
    l
};
enum Port_type
{
    c,
    s
};

enum Transport_type
{
    T,
    U
};
enum Flow_type
{
    simple,
    on_off
};