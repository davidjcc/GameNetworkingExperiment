cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(SimpleExample)

set(GENERATED_OUTPUT_DIR ${CMAKE_PROJECT_DIR})

set(FBS_FILES 
	${CMAKE_PROJECT_DIR}game_messages.fbs
)

set(GENERATED_FILES 
	${GENERATED_OUTPUT_DIR}/game_messages_generated.h
)

add_custom_command(
	OUTPUT ${GENERATED_FILES}
	COMMAND ${flatbuffers_BINARY_DIR}/flatc --cpp -o ${GENERATED_OUTPUT_DIR} ${FBS_FILES}
	DEPENDS flatc ${FBS_FILES}
	COMMENT "[SIMPLE] Generating ${FBS_FILES}..."
)
add_custom_target(GenerateFlatBuffers DEPENDS ${GENERATED_FILES})

function(simple_executable name entrypoint)
	add_executable(${name} ${entrypoint} ${SERVER_SOURCES} ${ENET_SOURCES})
	target_include_directories(${name} PRIVATE 
		bs/include
	)
	target_link_libraries(${name} PRIVATE bs fmt Ws2_32 winmm spdlog flatbuffers enet)
	add_dependencies(${name} GenerateFlatBuffers)
endfunction()

simple_executable(test_client simple_client.cpp)
simple_executable(test_server simple_server.cpp)

