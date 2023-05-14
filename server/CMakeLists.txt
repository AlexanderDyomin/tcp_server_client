cmake_minimum_required(VERSION 3.22)
set(CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED COMPONENTS system)

set(SOURCES
        main.cpp
        Registry.cpp
        RegistryHandler.cpp
        ../utils/FileReaderWriter.cpp
        ../utils/NetUtils.cpp
        ../utils/Utils.cpp)

set(HEADERS
        Server.h
        Session.h
        Registry.h
        RegistryHandler.h
        ../utils/FileReaderWriter.h
        ../utils/NetUtils.h
        ../utils/Utils.h)

add_executable(server ${SOURCES} ${HEADERS})
