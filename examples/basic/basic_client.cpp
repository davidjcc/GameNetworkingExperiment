#include <bs/enet.h>

#include <spdlog/sinks/stdout_color_sinks.h>

int main() {
	// Create the main enet object.
	auto logger = spdlog::stdout_color_mt("CLIENT");
	bs::ENet enet(logger);

	// Create the client.
	bs::Host_Client* client = enet.create_host_client();

	// Start the client running and connect to the server.
	client->start("127.0.0.1", 1234);

	while (1) {
		// Update the client's packets (if there are any).
		client->tick(0);

		// Process the packets.
		auto& packets = client->get_packets();
		while (!packets.empty()) {
			auto packet = packets.pop_front();
			switch (packet.get_type()) {
			case bs::Packet::NONE: break;

			case bs::Packet::CONNECT: {
				client->get_logger()->info("Connected to server");
				break;
			}

			case bs::Packet::DISCONNECT: {
				client->get_logger()->info("Disconnected from serve");
				break;
			}

			case bs::Packet::EVENT_RECIEVED: {
				client->get_logger()->info("Packet recieved");
				break;
			}
			}
		}
	}

	return 0;
}