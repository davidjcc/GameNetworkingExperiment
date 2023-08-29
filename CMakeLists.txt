cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(GamesNetworkingTest)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/CPM.cmake)

CPMAddPackage("gh:fmtlib/fmt#10.0.0")
CPMAddPackage("gh:nlohmann/json@3.11.2")
CPMAddPackage("gh:google/flatbuffers@23.5.26")

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
	src/clients/host_client.cpp
	src/clients/server_client_manager.cpp
	src/packet.cpp
	
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

set(FBS_FILES ${CMAKE_SOURCE_DIR}/src/messages/messages.fbs)
set(GENERATED_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/.generated)
set(GENERATED_FILES ${GENERATED_OUTPUT_DIR}/messages_generated.h)

add_custom_command(
	OUTPUT ${GENERATED_FILES}
	COMMAND ${CMAKE_SOURCE_DIR}/bin/flatc.exe --cpp -o ${GENERATED_OUTPUT_DIR} ${FBS_FILES}
	DEPENDS flatc ${FBS_FILES}
	COMMENT "Generating messages..."
)
add_custom_target(GenerateFlatBuffers DEPENDS ${GENERATED_FILES})

function(bs_executable name entrypoint)
	add_executable(${name} ${entrypoint} ${SERVER_SOURCES} ${ENET_SOURCES})
	target_include_directories(${name} PRIVATE 
		${ENET_DIR}/include
		${nlohmann_json_SOURCE_DIR}/single_include
		${CMAKE_SOURCE_DIR}/src
		${GENERATED_OUTPUT_DIR}
	)
	target_link_libraries(${name} PRIVATE fmt Ws2_32 winmm spdlog flatbuffers)
	add_dependencies(${name} GenerateFlatBuffers)
endfunction()

bs_executable(test_client examples/test_client.cpp)
bs_executable(test_server examples/test_server.cpp)

add_definitions(-DNOMINMAX)
