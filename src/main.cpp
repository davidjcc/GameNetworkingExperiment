#include "server.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <thread>

#include "enet.h"

void poll_callback(Game_Server* server, Event* event) {
}

int main() {
	auto logger = spdlog::stdout_color_mt("SERVER");

	ENet enet(logger);
	enet.init();

	Game_Server* server = nullptr;
	bool running = true;

	std::thread server_thread([&] {
		server = enet.create_server("localhost", 5651, 100);
		server->start();

		server->poll([](Game_Server* sv, Game_Client* client, Event* ev) {
			ASSERT_PANIC(client->get_state() == Game_Client::CONNECTED, "Client is not connected");

			// Handle events
			const auto data = ev->get_string();
			sv->get_logger()->info("Packet received: {}", data);
			});
		});

	std::this_thread::sleep_for(std::chrono::seconds(3));

	Host_Client* client = enet.create_host_client();

	while (running) {
		client->poll([](Host_Client* c, Event* event) {

			});
	}

	server_thread.join();

	enet.destroy_server(server);

	return EXIT_SUCCESS;
}
