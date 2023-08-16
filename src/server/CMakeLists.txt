#####################
# Server executable
#####################

set(SERVER_SOURCES
	server.cpp
	${CMAKE_SOURCE_DIR}/src/shared/state.cpp
)

set(ENET_DIR ${CMAKE_SOURCE_DIR}/src/libs/enet)
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

add_executable(server ${SERVER_SOURCES} ${ENET_SOURCES})

target_include_directories(server PRIVATE 
	${CMAKE_SOURCE_DIR}/src/shared
	${ENET_DIR}/include

	${nlohmann_json_SOURCE_DIR}/single_include
)

target_link_libraries(server PRIVATE fmt Ws2_32 winmm spdlog)
