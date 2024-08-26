cmake_minimum_required(VERSION 3.20)
project(IoT_Emulator)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB OBJECT_SOURCES "src/*.cpp")

add_executable(${PROJECT_NAME} ${OBJECT_SOURCES})

if (WIN32)
    # Windows specific configurations
    set(C_LIBRARY "C:/PahoC/lib")
    set(CPP_LIBRARY "C:/PahoCpp/lib")

    set(HEADERS_C "C:/PahoCPP/include")
    set(HEADERS_CPP "C:/PahoC/include")

    include_directories(${HEADERS_C})
    include_directories(${HEADERS_CPP})

    add_library(cpp_paho_lib STATIC IMPORTED)
    set_target_properties(cpp_paho_lib PROPERTIES IMPORTED_LOCATION "${CPP_LIBRARY}/paho-mqttpp3-static.lib")

    add_library(c_paho_lib STATIC IMPORTED)
    set_target_properties(c_paho_lib PROPERTIES IMPORTED_LOCATION "${C_LIBRARY}/paho-mqtt3a-static.lib")

    target_link_libraries(${PROJECT_NAME} cpp_paho_lib c_paho_lib Ws2_32.lib winmm)
else()
    # Linux specific configurations
    find_library(PAHO_MQTT_CPP paho-mqttpp3 PATHS /usr/local/lib)
    find_library(PAHO_MQTT_C paho-mqtt3a PATHS /usr/local/lib)

    target_link_libraries(${PROJECT_NAME} ${PAHO_MQTT_CPP} ${PAHO_MQTT_C})
endif()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
