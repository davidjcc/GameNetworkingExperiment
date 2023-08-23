#include "server.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <thread>

#include "enet.h"

const char* host = "localhost";
int32_t port = 1234;

int main() {
	auto logger = spdlog::stdout_color_mt("SERVER");
	ENet enet(logger);

	Game_Server* server = nullptr;
	bool running = true;

	server = enet.create_server(host, port, 100);
	server->start();

	while (true) {
		server->poll(5000, [](Game_Server* sv, Server_Client* client, Event* ev) {
			ASSERT_PANIC(client->get_state() == Server_Client::CONNECTED, "Client is not connected");

			client->get_logger()->info("Server receieved a packet");
			});
	}

	enet.destroy_server(server);

	return EXIT_SUCCESS;
}
