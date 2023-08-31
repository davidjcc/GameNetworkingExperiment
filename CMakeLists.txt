cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(BS)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/CPM.cmake)

CPMAddPackage("gh:fmtlib/fmt#10.0.0")
CPMAddPackage("gh:google/flatbuffers@23.5.26")
CPMAddPackage("gh:lsalzman/enet#v1.3.17")

CPMAddPackage(
    NAME spdlog
    GITHUB_REPOSITORY gabime/spdlog
    VERSION 1.12.0
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(BS_SOURCES
	bs/src/enet.cpp
	bs/src/server.cpp
	bs/src/host_client.cpp
	bs/src/server_client_manager.cpp
	bs/src/packet.cpp
)

add_library(bs STATIC ${BS_SOURCES})
target_include_directories(bs PUBLIC bs/include ${enet_SOURCE_DIR}/include)
target_link_libraries(bs PUBLIC spdlog fmt flatbuffers enet Ws2_32)

set(FBS_FILES ${CMAKE_SOURCE_DIR}/examples/simple/game_messages.fbs)
set(GENERATED_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/examples/simple)
set(GENERATED_FILES ${GENERATED_OUTPUT_DIR}/game_messages_generated.h)

add_custom_command(
	OUTPUT ${GENERATED_FILES}
	COMMAND ${flatbuffers_BINARY_DIR}/flatc --cpp -o ${GENERATED_OUTPUT_DIR} ${FBS_FILES}
	DEPENDS flatc ${FBS_FILES}
	COMMENT "Generating messages..."
)
add_custom_target(GenerateFlatBuffers DEPENDS ${GENERATED_FILES})

function(bs_executable name entrypoint)
	add_executable(${name} ${entrypoint} ${SERVER_SOURCES} ${ENET_SOURCES})
	target_include_directories(${name} PRIVATE 
		bs/include
	)
	target_link_libraries(${name} PRIVATE bs fmt Ws2_32 winmm spdlog flatbuffers enet)
	add_dependencies(${name} GenerateFlatBuffers)
endfunction()

bs_executable(test_client examples/simple/test_client.cpp)
bs_executable(test_server examples/simple/test_server.cpp)


add_definitions(-D NOMINMAX -D _WINSOCK_DEPRECATED_NO_WARNINGS -D _CRT_SECURE_NO_WARNINGS)
