cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(SimpleExample)

set(SOURCE_DIR ${SimpleExample_SOURCE_DIR})
set(GENERATED_OUTPUT_DIR ${SOURCE_DIR}/.generated)

set(FBS_FILES 
	${SOURCE_DIR}/game_messages.fbs
)

set(GENERATED_FILES 
	${GENERATED_OUTPUT_DIR}/game_messages_generated.h
)

add_custom_command(
	OUTPUT ${GENERATED_FILES}
	COMMAND flatc --cpp -o ${GENERATED_OUTPUT_DIR} ${FBS_FILES}
	DEPENDS flatc ${FBS_FILES}
	COMMENT "[SIMPLE] Generating ${FBS_FILES}..."
)
add_custom_target(GenerateFlatBuffers DEPENDS ${GENERATED_FILES})

function(simple_executable name entrypoint)
	add_executable(${name} ${entrypoint} ${SERVER_SOURCES} ${ENET_SOURCES})
	target_include_directories(${name} PRIVATE 
		bs/include
		${GENERATED_OUTPUT_DIR}
	)
	target_link_libraries(${name} PRIVATE bs fmt spdlog flatbuffers enet)
	if (WIN32)
		target_link_libraries(bs PUBLIC Ws2_32 winmm)
	endif()

	add_dependencies(${name} GenerateFlatBuffers)
endfunction()

simple_executable(test_client src/simple_client.cpp)
simple_executable(test_server src/simple_server.cpp)

