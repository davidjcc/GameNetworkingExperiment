#include <bs/server.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <thread>

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

		default:
			server.get_logger()->error("Unknown message type");
		}
		});

	while (1) {
		server.tick();
	}

	return EXIT_SUCCESS;
}
