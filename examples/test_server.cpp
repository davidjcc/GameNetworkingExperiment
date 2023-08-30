#include "server.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <thread>

#include "enet.h"
#include "game_messages_generated.h"

const char* host = "localhost";
int32_t port = 1234;

int main() {
	auto logger = spdlog::stdout_color_mt("SERVER");
	//logger->set_level(spdlog::level::trace);
	ENet enet(logger);

	Host_Server* server = nullptr;
	bool running = true;

	server = enet.create_server(host, port, 100);
	server->start();

	flatbuffers::FlatBufferBuilder builder;

	while (true) {
		server->tick(0);

		auto& packets = server->get_packets();
		while (!packets.empty()) {
			auto packet = packets.pop_back();
			switch (packet.get_type()) {
			case Packet::CONNECT: {
				server->get_logger()->info("A new client has connected: {}", (size_t)packet.get_peer());
				break;
			}

			case Packet::DISCONNECT: {
				server->get_logger()->info("A client has disconnected: {}", (size_t)packet.get_peer());
				break;
			}

			case Packet::EVENT_RECIEVED: {
				auto packet_bytes = packet.get_bytes();

				flatbuffers::Verifier verifier(packet_bytes.data(), packet_bytes.size());
				if (!Game::VerifyMessageBuffer(verifier)) {
					server->get_logger()->error("Verifier failed");
					break;
				}

				const auto* message = flatbuffers::GetRoot<Game::Message>(packet_bytes.data());

				// We can now switch on the type returned and act accordingly.
				auto type = message->payload_type();
				server->get_logger()->info("Packet type is: {}", Game::EnumNameAnyMessage(type));

				switch (type) {
				case Game::AnyMessage_ClientConnectedMessage: {
					const auto* client_msg = message->payload_as_ClientConnectedMessage();
					break;
				}

				case Game::AnyMessage_ClientDisconnectedMessage: {
					const auto* client_msg = message->payload_as_ClientDisconnectedMessage();
				} break;

				case Game::AnyMessage_ClientReadyMessage: {
					const auto* client_msg = message->payload_as_ClientReadyMessage();
				} break;

				case Game::AnyMessage_PlayerMovedMessage: {
					const auto* client_msg = message->payload_as_PlayerMovedMessage();
					break;
				}

				case Game::AnyMessage_BallMovedMessage: {
					const auto* client_msg = message->payload_as_BallMovedMessage();

					server->get_logger()->info("Ball moved to: {}, {}", client_msg->pos()->x(), client_msg->pos()->y());
					break;
				}

				default:
					server->get_logger()->error("Unknown message type");
				}
				break;
			}
			}
		}
	}

	enet.destroy_server(server);

	return EXIT_SUCCESS;
}
