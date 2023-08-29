#include "server.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <thread>

#include "enet.h"
#include "messages_generated.h"

const char* host = "localhost";
int32_t port = 1234;

int main() {
	auto logger = spdlog::stdout_color_mt("CLIENT");
	ENet enet(logger);

	Host_Client* client = enet.create_host_client();
	ASSERT_PANIC(client->connect(host, port), "Error connecting client to server");

	flatbuffers::FlatBufferBuilder fbb;

	while (true) {
		client->tick(0);

		auto& packets = client->get_packets();
		while (!packets.empty()) {
			auto packet = packets.pop_front();
			switch (packet.get_type()) {
			case Packet::CONNECT: {
				client->get_logger()->info("Connected to server!");

				fbb.Clear();
				auto message = Game::CreateClientReadyMessage(fbb);
				fbb.Finish(message);

				Packet response(fbb.GetBufferPointer(), fbb.GetSize());
				client->broadcast_to_server(response);
				break;
			}

			case Packet::DISCONNECT: {
				client->get_logger()->info("Disconnected to server!");

				break;
			}

			case Packet::EVENT_RECIEVED: {
				client->get_logger()->info("Recieved a packet from the client: {}", packet.get_string());
				break;
			}
			}
		}
	}

	enet.destroy_client(client);

	return EXIT_SUCCESS;
}
