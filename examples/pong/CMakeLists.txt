cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(ExamplePong)

# Dependencies
set(RAYLIB_VERSION 4.2.0)
find_package(raylib ${RAYLIB_VERSION} QUIET) # QUIET or REQUIRED
if (NOT raylib_FOUND) # If there's none, fetch and build raylib
  include(FetchContent)
  FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
  )
  FetchContent_GetProperties(raylib)
  if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
    set(FETCHCONTENT_QUIET NO)
    FetchContent_Populate(raylib)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
    add_subdirectory(${raylib_SOURCE_DIR} ${raylib_BINARY_DIR})
  endif()
endif()

set(SOURCE_DIR ${ExamplePong_SOURCE_DIR})

set(GENERATED_OUTPUT_DIR ${SOURCE_DIR}/.generated)
set(FBS_FILES ${SOURCE_DIR}/game_messages.fbs)
set(GENERATED_FILES ${GENERATED_OUTPUT_DIR}/game_messages_generated.h)

add_custom_command(
	OUTPUT ${GENERATED_FILES}
	COMMAND ${flatbuffers_BINARY_DIR}/flatc --cpp -o ${GENERATED_OUTPUT_DIR} ${FBS_FILES}
	DEPENDS flatc ${FBS_FILES}
	COMMENT "[SIMPLE] Generating ${FBS_FILES}..."
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
)
target_link_libraries(pong_client PRIVATE bs fmt Ws2_32 winmm spdlog flatbuffers enet raylib)
add_dependencies(pong_client GeneratePongMessages)

# -----------------------------
#  Pong server executable
# -----------------------------
add_executable(pong_server server/pong_server.cpp)
target_include_directories(pong_server PRIVATE 
	${CMAKE_SOURCE_DIR}/bs/include
	${SOURCE_DIR}
	${GENERATED_OUTPUT_DIR}
)
target_link_libraries(pong_server PRIVATE bs fmt Ws2_32 winmm spdlog flatbuffers enet raylib)
add_dependencies(pong_server GeneratePongMessages)


