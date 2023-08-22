cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(GamesNetworkingTest)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/CPM.cmake)

CPMAddPackage("gh:fmtlib/fmt#10.0.0")
CPMAddPackage("gh:nlohmann/json@3.11.2")

CPMAddPackage(
    NAME spdlog
    GITHUB_REPOSITORY gabime/spdlog
    VERSION 1.12.0
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_definitions(-D _CRT_SECURE_NO_WARNINGS)

set(SERVER_SOURCES
	src/enet.cpp
	src/server.cpp
	src/client.cpp
	src/event.cpp
)

set(ENET_DIR ${CMAKE_SOURCE_DIR}/libs/enet)
set(ENET_SOURCES
	${ENET_DIR}/callbacks.c
	${ENET_DIR}/host.c
	${ENET_DIR}/list.c
	${ENET_DIR}/packet.c
	${ENET_DIR}/peer.c
	${ENET_DIR}/protocol.c
	${ENET_DIR}/unix.c
	${ENET_DIR}/win32.c
)

add_executable(server src/main.cpp ${SERVER_SOURCES} ${ENET_SOURCES})
add_executable(test_client src/test_client.cpp ${SERVER_SOURCES} ${ENET_SOURCES})

target_include_directories(server PRIVATE 
	${ENET_DIR}/include
	${nlohmann_json_SOURCE_DIR}/single_include
	${CMAKE_SOURCE_DIR}/src
)

target_include_directories(test_client PRIVATE 
	${ENET_DIR}/include
	${nlohmann_json_SOURCE_DIR}/single_include
	${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(server PRIVATE fmt Ws2_32 winmm spdlog)
target_link_libraries(test_client PRIVATE fmt Ws2_32 winmm spdlog)



