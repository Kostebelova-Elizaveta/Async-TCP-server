cmake_minimum_required(VERSION 3.15.0)
cmake_policy(SET CMP0016 NEW)

project(client VERSION 1.0.0 DESCRIPTION "Знакомство с асинхронным вводом/выводом на C++ (клиент)")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#find_package(Boost 1.65.1 REQUIRED)
FIND_PACKAGE(Boost COMPONENTS program_options REQUIRED)

include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

add_executable (ClientTry "blocking_tcp_echo_client.cpp")

target_link_libraries(ClientTry ${Boost_LIBRARIES})
