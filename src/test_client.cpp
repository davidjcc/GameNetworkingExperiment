#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <thread>

#include "enet.h"

void poll_callback(Game_Server* server, Event* event) {
}

int main() {
	auto logger = spdlog::stdout_color_mt("CLIENT");

	ENet enet(logger);
	enet.init();

	Host_Client* client = enet.create_host_client();

	bool running = true;
	while (running) {
		client->poll([](Host_Client* c, Event* event) {
			});
	}

	enet.destroy_client(client);

	return EXIT_SUCCESS;
}
