#include <bs/enet.h>

#include <spdlog/sinks/stdout_color_sinks.h>

int main() {
	auto logger = spdlog::stdout_color_mt("CLIENT");

	ENet enet(logger);

	Host_Client* client = enet.create_host_client();
	client->start("127.0.0.1", 1234);

	while (1) {
		client->tick(0);

		auto& packets = client->get_packets();
		while (!packets.empty()) {
			auto packet = packets.pop_front();
			switch (packet.get_type()) {
			case Packet::NONE: break;

			case Packet::CONNECT: {
				client->get_logger()->info("Connected to server");
				break;
			}

			case Packet::DISCONNECT: {
				client->get_logger()->info("Disconnected from serve");
				break;
			}

			case Packet::EVENT_RECIEVED: {
				client->get_logger()->info("Packet recieved");
				break;
			}
			}
		}
	}

	return 0;
}