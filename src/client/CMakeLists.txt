#####################
# Client executable
#####################

CPMAddPackage(
    name raylib
    GITHUB_REPOSITORY raysan5/raylib
    GIT_TAG 4.5.0
    OPTIONS
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
)

CPMAddPackage(
	NAME rlimgui
	GITHUB_REPOSITORY  raylib-extras/rlimgui
	GIT_TAG MAIN
)

CPMAddPackage(
	NAME imgui
	GITHUB_REPOSITORY  ocornut/imgui
	VERSION 1.89.8
	OPTIONS
	DOWNLOAD_ONLY
)

set(CLIENT_SOURCES
	assets.cpp
	pong.cpp
	client.cpp

	${rlimgui_SOURCE_DIR}/rlImGui.cpp

	${CMAKE_SOURCE_DIR}/src/shared/state.cpp

	${imgui_SOURCE_DIR}/imgui.cpp
	${imgui_SOURCE_DIR}/imgui_demo.cpp
	${imgui_SOURCE_DIR}/imgui_draw.cpp
	${imgui_SOURCE_DIR}/imgui_widgets.cpp
	${imgui_SOURCE_DIR}/imgui_tables.cpp

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

	${rlimgui_SOURCE_DIR}
	${imgui_SOURCE_DIR}
	${nlohmann_json_SOURCE_DIR}/single_include
)

target_link_libraries(client PRIVATE raylib winmm fmt Ws2_32 spdlog)
