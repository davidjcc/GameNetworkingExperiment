#include "server.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

int main() {
	auto logger = spdlog::stdout_color_mt("SERVER");
	Game_Server server("localhost", 1234, 100, logger);
}