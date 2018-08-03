cmake_minimum_required(VERSION 3.9)
project(mpd_discord_richpresence)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -O2 -s -DNDEBUG")

# find discord rpc
find_library(DISCORD_RPC_LIB discord-rpc)
find_path(DISCORD_RPC_H discord_rpc.h)

message(STATUS "discord_rpc library is at: ${DISCORD_RPC_LIB}")
message(STATUS "discord_rpc headers is at: ${DISCORD_RPC_H}")

if(NOT DISCORD_RPC_LIB OR NOT DISCORD_RPC_H)
    message(STATUS "Could not find either the discord_rpc libary or headers. Building from source.")

    # pull submodules
    execute_process(
        COMMAND git submodule update --init --recursive
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

    set(BUILD_EXAMPLES OFF CACHE BOOL "build examples")
    set(USE_STATIC_CRT ON CACHE BOOL "use static crt")
    set(BUILD_SHARED_LIBS ON CACHE BOOL "build shared libs")

    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/discord-rpc)

    set(DISCORD_RPC_LIB discord-rpc)
    set(DISCORD_RPC_H ${CMAKE_CURRENT_SOURCE_DIR}/lib/discord-rpc/include/)
endif()

set(SOURCE_FILES
    src/main.cpp
    src/MpdClient.cpp
    src/MpdClient.h)

# copy systemd .service to binary dir
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/mpd-discord.service
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_executable(mpd_discord_richpresence ${SOURCE_FILES})

target_include_directories(mpd_discord_richpresence PUBLIC
    ${DISCORD_RPC_H})

target_link_libraries(mpd_discord_richpresence PUBLIC
    ${DISCORD_RPC_LIB}
    pthread
    mpdclient)
