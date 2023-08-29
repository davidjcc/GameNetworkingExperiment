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
	auto logger = spdlog::stdout_color_mt("SERVER");
	//logger->set_level(spdlog::level::trace);
	ENet enet(logger);

	Game_Server* server = nullptr;
	bool running = true;

	server = enet.create_server(host, port, 100);
	server->start();

	flatbuffers::FlatBufferBuilder builder;

	while (true) {
		server->tick(0);

		auto& packets = server->get_packets();
		while (!packets.empty()) {
			auto packet = packets.pop_front();
			switch (packet.get_type()) {
			case Packet::CONNECT: {
				server->get_logger()->info("A new client has connected: {}", (size_t)packet.get_peer());

				auto message = Game::CreateClientConnectedMessage(builder, Game::MessageType::MessageType_ClientConnected);
				builder.Finish(message);
				enet_peer_send(packet.get_peer(), 0, enet_packet_create(builder.GetBufferPointer(), builder.GetSize(), ENET_PACKET_FLAG_RELIABLE));
				builder.Clear();
				break;
			}

			case Packet::DISCONNECT: {
				server->get_logger()->info("A client has disconnected: {}", (size_t)packet.get_peer());
				break;
			}

			case Packet::EVENT_RECIEVED: {
				auto packet_string = packet.get_string();
				server->get_logger()->info("Recieved a packet from the client: {}", packet_string);

				// Deserialize the packet in to a flatbuffer
				const auto* message = flatbuffers::GetRoot<Game::MessageUnion>(packet_string.c_str());
				auto type = *message;

				// We can now switch on the type returned and act accordingly.
				switch (type) {
				case Game::MessageType::MessageType_ClientConnected: {
				} break;

				case Game::MessageType::MessageType_ClientDisconnected: {
				} break;

				case Game::MessageType::MessageType_ClientReady: {
				} break;

				default:
					UNREACHABLE();
				}
				break;
			}
			}
		}
	}

	enet.destroy_server(server);

	return EXIT_SUCCESS;
}
