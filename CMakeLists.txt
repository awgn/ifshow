cmake_minimum_required(VERSION 2.4)

include_directories(. more)
add_executable(ifshow ifshow.cpp mii-tool.cpp iwlib.c proc_net_dev.cpp proc_net_wireless.cpp proc_interrupt.cpp)

target_link_libraries(ifshow -lpci)
