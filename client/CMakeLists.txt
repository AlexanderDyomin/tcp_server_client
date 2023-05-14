cmake_minimum_required(VERSION 3.22)
set(CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED COMPONENTS system)

set(SOURCES
        main.cpp
        GenerateMessage.cpp
        ../utils/NetUtils.cpp
        ../utils/Utils.cpp)

set(HEADERS
        GenerateMessage.h
        ../utils/NetUtils.h
        ../utils/Utils.h)

add_executable(client ${SOURCES} ${HEADERS})