#include "server.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <thread>

#include "enet.h"
#include "game_messages_generated.h"

#include "config.h"
#include "base_game_host.h"

int main() {
	auto logger = spdlog::stdout_color_mt("SERVER");

	auto server = Game_Host<Host_Server>(logger, SAMPLES_HOST, SAMPLES_PORT);

	server.set_tick_callback([&](const Game::Message* message, const Packet* packet) {
		auto type = message->payload_type();

		switch (type) {
		case Game::Any_ClientConnectedRequest: {
			const auto* client_msg = message->payload_as_ClientConnectedRequest();

			auto client = server.get_host_type()->get_client_manager().get_client(packet->get_peer());

			Packet packet = server.create_client_connect_response(client->get_id());
			packet.set_peer(client->get_peer());
			packet.send(true);
			break;
		}

		case Game::Any_ClientDisconnected: {
			const auto* client_msg = message->payload_as_ClientDisconnected();
		} break;

		case Game::Any_ClientReady: {
			const auto* client_msg = message->payload_as_ClientReady();
		} break;

		case Game::Any_PlayerMoved: {
			const auto* client_msg = message->payload_as_PlayerMoved();
			break;
		}

		case Game::Any_BallMoved: {
			const auto* client_msg = message->payload_as_BallMoved();

			server.get_logger()->info("Ball moved to: {}, {}", client_msg->pos()->x(), client_msg->pos()->y());
			break;
		}

		default:
			server.get_logger()->error("Unknown message type");
		}
		});

	while (1) {
		server.tick();

		// Once we have connected clients we can start
		// sending them game updates.
		if (!server->get_client_manager().empty()) {

			Packet packet = server.create_ball_moved(10.0f, 20.0f);
			server->broadcast_to_clients(packet, true);
		}
	}

	return EXIT_SUCCESS;
}
