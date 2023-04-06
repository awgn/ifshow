cmake_minimum_required(VERSION 2.4)

project(ifshow)

include_directories(. lib src)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build: Release or Debug" FORCE)
endif()

set (CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS} -std=c++17 -D_GLIBCXX_DEBUG -g -O0 -Wall -Wextra")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -std=c++17 -DNDEBUG -O3 -march=native -Wall -Wextra")

add_executable(ifshow src/ifshow.cpp lib/iwlib.c src/proc/net_dev.cpp src/proc/net_wireless.cpp src/proc/interrupt.cpp)

target_link_libraries(ifshow -lpci)

install(TARGETS ifshow DESTINATION bin/)
