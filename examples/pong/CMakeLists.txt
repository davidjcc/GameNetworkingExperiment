cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(ExamplePong)

# Dependencies
include(raylib.cmake)

set(SOURCE_DIR ${ExamplePong_SOURCE_DIR})

set(GENERATED_OUTPUT_DIR ${SOURCE_DIR}/.generated)
set(FBS_FILES ${SOURCE_DIR}/game_messages.fbs)
set(GENERATED_FILES ${GENERATED_OUTPUT_DIR}/game_messages_generated.h)

add_custom_command(
	OUTPUT ${GENERATED_FILES}
	COMMAND flatc --cpp -o ${GENERATED_OUTPUT_DIR} ${FBS_FILES}
	DEPENDS flatc ${FBS_FILES}
	COMMENT "[PONG] Generating ${FBS_FILES}..."
)
add_custom_target(GeneratePongMessages DEPENDS ${GENERATED_FILES})

# -----------------------------
#  Pong client executable
# -----------------------------
add_executable(pong_client client/pong_client.cpp client/client_state.cpp)
target_include_directories(pong_client PRIVATE 
	${CMAKE_SOURCE_DIR}/bs/include
	${SOURCE_DIR}
	${GENERATED_OUTPUT_DIR}
	${raygui_SOURCE_DIR}/src
)
target_link_libraries(pong_client PRIVATE bs fmt spdlog flatbuffers enet raylib)
if (WIN32)
	target_link_libraries(bs PUBLIC Ws2_32 winmm)
endif()

add_dependencies(pong_client GeneratePongMessages)

# -----------------------------
#  Pong server executable
# -----------------------------
add_executable(pong_server server/pong_server.cpp)
target_include_directories(pong_server PRIVATE 
	${CMAKE_SOURCE_DIR}/bs/include
	${SOURCE_DIR}
	${GENERATED_OUTPUT_DIR}
	${raygui_SOURCE_DIR}/src
)
target_link_libraries(pong_server PRIVATE bs fmt spdlog flatbuffers enet raylib)
if (WIN32)
	target_link_libraries(bs PUBLIC Ws2_32 winmm)
endif()


add_dependencies(pong_server GeneratePongMessages)


