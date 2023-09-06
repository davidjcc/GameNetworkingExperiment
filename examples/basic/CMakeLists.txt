cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(BasicExample)

function(basic_executable name entrypoint)
	add_executable(${name} ${entrypoint} ${SERVER_SOURCES} ${ENET_SOURCES})
	target_include_directories(${name} PRIVATE 
		bs/include
		${GENERATED_OUTPUT_DIR}
	)
	target_link_libraries(${name} PRIVATE bs fmt Ws2_32 winmm spdlog enet)
	add_dependencies(${name} GenerateFlatBuffers)
endfunction()

basic_executable(basic_client basic_client.cpp)
basic_executable(basic_server basic_server.cpp)

