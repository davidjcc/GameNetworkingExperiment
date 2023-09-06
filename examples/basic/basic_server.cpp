#include <bs/enet.h>

#include <spdlog/sinks/stdout_color_sinks.h>

int main() {
	auto logger = spdlog::stdout_color_mt("SERVER");

	ENet enet(logger);

	Host_Server* server = enet.create_server("127.0.0.1", 1234, 1);
	server->start();

	while (1) {
		server->tick(0);

		auto& packets = server->get_packets();
		while (!packets.empty()) {
			auto packet = packets.pop_front();
			switch (packet.get_type()) {
			case Packet::NONE: break;

			case Packet::CONNECT: {
				server->get_logger()->info("Client connected");
				break;
			}

			case Packet::DISCONNECT: {
				server->get_logger()->info("Client disconnected");
				break;
			}

			case Packet::EVENT_RECIEVED: {
				server->get_logger()->info("Packet recieved");
				break;
			}
			}
		}
	}

	return 0;
}