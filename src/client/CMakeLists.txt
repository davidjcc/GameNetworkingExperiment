#####################
# Client executable
#####################

CPMAddPackage("gh:fmtlib/fmt#10.0.0")
CPMAddPackage("gh:nlohmann/json@3.11.2")
CPMAddPackage("gh:gabime/spdlog@1.12.0")
CPMAddPackage(
    name raylib
    GITHUB_REPOSITORY raysan5/raylib
    GIT_TAG 4.5.0
    OPTIONS
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
)

set(CLIENT_SOURCES
	assets.cpp
	pong.cpp
	client.cpp
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


add_executable(client ${CLIENT_SOURCES} ${ENET_SOURCES})

target_include_directories(client PRIVATE 
	${CMAKE_SOURCE_DIR}/src/shared
	${ENET_DIR}/include

	${nlohmann_json_SOURCE_DIR}/single_include
)

target_link_libraries(client PRIVATE raylib winmm fmt Ws2_32 spdlog)
