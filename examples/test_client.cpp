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

	flatbuffers::FlatBufferBuilder builder;

	while (true) {
		client->tick(1000);

		auto& packets = client->get_packets();
		while (!packets.empty()) {
			auto packet = packets.pop_back();
			switch (packet.get_type()) {
			case Packet::CONNECT: {
				client->get_logger()->info("Connected to server!");
				break;
			}

			case Packet::DISCONNECT: {
				client->get_logger()->info("Disconnected to server!");

				break;
			}

			case Packet::EVENT_RECIEVED: {
				client->get_logger()->info("Recieved a packet from the server: {}", packet.get_string());

				break;
			}
			}

			if (client->get_peer()) {
				builder.Clear();

				auto ball_moved = Game::CreateBallMovedMessage(builder, Game::CreateVec2(builder, 10.0f, 20.0f));
				auto message = Game::CreateMessage(builder, Game::AnyMessage_BallMovedMessage, ball_moved.Union());
				builder.Finish(message);

				packet.set_bytes(builder.GetBufferPointer(), builder.GetSize());
				client->broadcast_to_server(packet, true);
			}
		}
	}

	enet.destroy_client(client);

	return EXIT_SUCCESS;
}
