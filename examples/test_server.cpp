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
		server->tick(5000);
		auto& packets = server->get_packets();
		while (!packets.empty()) {
			auto packet = packets.pop_front();
			switch (packet.get_type()) {
			case Packet::CONNECT: {
				break;
			}

			case Packet::DISCONNECT: {
				break;
			}

			case Packet::EVENT_RECEIVED: {
				break;
			}
			}
		}
	}

	enet.destroy_server(server);

	return EXIT_SUCCESS;
}
