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
	//logger->set_level(spdlog::level::trace);
	ENet enet(logger);

	Game_Server* server = nullptr;
	bool running = true;

	server = enet.create_server(host, port, 100);
	server->start();

	while (true) {
		server->tick(0);

		auto& packets = server->get_packets();
		while (!packets.empty()) {
			auto packet = packets.pop_front();
			switch (packet.get_type()) {
			case Packet::CONNECT: {
				server->get_logger()->info("Connected to server!");

				Packet response(packet.get_peer());

				break;
			}

			case Packet::DISCONNECT: {
				server->get_logger()->info("Disconnected to server!");
				break;
			}

			case Packet::EVENT_RECIEVED: {
				server->get_logger()->info("Recieved a packet from the client: {}", packet.get_string());
				break;
			}
			}
		}

		Packet reply;
		reply.set_string("Hello there!");
		server->broadcast_to_clients(reply, true);
	}

	enet.destroy_server(server);

	return EXIT_SUCCESS;
}
