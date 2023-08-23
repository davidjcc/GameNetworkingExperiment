#include "server.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <thread>

#include "enet.h"

const char* host = "localhost";
int32_t port = 1234;

std::shared_ptr<spdlog::logger> logger;
int main() {
	logger = spdlog::stdout_color_mt("SERVER");
	ENet enet(logger);
	enet.init();

	Game_Server* server = nullptr;
	bool running = true;

	server = enet.create_server(host, port, 100);
	server->start();

	while (true) {
		server->poll([](Game_Server* sv, Game_Client* client, Event* ev) {
			ASSERT_PANIC(client->get_state() == Game_Client::CONNECTED, "Client is not connected");

			logger->info("Server receieved a packet");
			});
	}

	enet.destroy_server(server);

	return EXIT_SUCCESS;
}
