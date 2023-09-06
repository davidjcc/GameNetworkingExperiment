#include <bs/server.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <thread>

#include "game_messages_generated.h"

#include "config.h"
#include "base_game_host.h"

int main() {
	Game_Host<Host_Client> client(
		spdlog::stdout_color_mt("CLIENT"),
		SAMPLES_HOST, SAMPLES_PORT);

	client.set_connect_callback([&] {
		// When we connect send over a connection request so we
		// can get our client id and store it. We shouldn't 
		// continue until we do so.
		client.create_client_connect_request().send(true);
		});

	client.set_disconnect_callback([&] {
		});

	client.set_tick_callback([&](const Game::Message* message, const Packet* packet) {
		ASSERT_PANIC(client->get_state() == Base_Client::CONNECTED, "Not currently connected");

		// We can now switch on the type returned and act accordingly.
		auto type = message->payload_type();
		client.get_logger()->info("Packet type is: {}", Game::EnumNameAny(type));

		switch (type) {
		case Game::Any_ClientConnectedResponse: {
			const auto* client_msg = message->payload_as_ClientConnectedResponse();
			auto id = client_msg->client_id();

			client.get_logger()->info("Server responded to connect with id: {}. Setting client id.", id);
			client->set_id(id);
			break;
		}

		case Game::Any_ClientDisconnected: {
			const auto* client_msg = message->payload_as_ClientDisconnected();
		} break;

		case Game::Any_ClientReady: {
			const auto* client_msg = message->payload_as_ClientReady();
		} break;

		default:
			client.get_logger()->error("Unknown message type");
		}

		});

	while (true) {
		client.tick(0);

	}

	return EXIT_SUCCESS;
}
