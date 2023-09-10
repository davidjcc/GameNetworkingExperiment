#include <bs/server.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <thread>

#include "game_messages_generated.h"

#include "config.h"
#include "base_game_host.h"

#include <raylib.h>


#define TICK_RATE (int)((float)(1.0f / 60.0f) * 1000.0f)

struct Player {
	int id = -1;
	float x = 0.0f;
	float y = 0.0f;
	bool ready = false;
	int score = 0;
};
float ball_x = 0.0f;
float ball_y = 0.0f;
float ball_vx = 0.0f;
float ball_vy = 0.0f;

static Player players[2] = {};
bool game_started = false;

enum Game_State {
	WAITING = 0,
	PLAYING,
	PLAYER_SCORED,
	ENDED,
	DISCONNECTED,
};
const char* GameStateToString(Game_State& s) {
	switch (s) {
	case Game_State::WAITING: return "WAITING";
	case Game_State::PLAYING: return "PLAYING";
	case Game_State::ENDED: return "ENDED";
	default: return "UNKNOWN";
	}
}
Game_State gameState = WAITING;

void game_state_tick() {
	for (auto& player : players) {
		if (player.y < 0) {
			player.y = 0;
		}

		if (player.y + PLAYER_HEIGHT > HEIGHT) {
			player.y = HEIGHT - PLAYER_HEIGHT;
		}

		ball_x += ball_vx;
		ball_y += ball_vy;

		if (ball_y < 0) {
			ball_vy *= -1;
		}

		if (ball_y + BALL_WIDTH > HEIGHT) {
			ball_vy *= -1;
		}

		if (ball_x < 0 || ball_x + BALL_WIDTH > WIDTH) {
			if (ball_x < 0) {
				players[1].score++;
			}
			else {
				players[0].score++;
			}
			ball_x = WIDTH / 2.0f;
			ball_y = HEIGHT / 2.0f;
			ball_vx = 1;
		}

		if (CheckCollisionRecs({ ball_x, ball_y, BALL_WIDTH, BALL_WIDTH }, { player.x, player.y, PLAYER_WIDTH, PLAYER_HEIGHT })) {
			ball_vx *= -1.08f;
		}
	}
}

int main() {
	auto logger = spdlog::stdout_color_mt("SERVER");

	auto server = Game_Host<bs::Host_Server>(logger, SAMPLES_HOST, SAMPLES_PORT);

	server.set_disconnect_callback([&] {
		gameState = DISCONNECTED;
		});

	server.set_tick_callback([&](const Game::Message* message, const bs::Packet* packet) {
		auto type = message->payload_type();

		switch (type) {
		case Game::Any_ClientConnectedRequest: {
			const auto* client_msg = message->payload_as_ClientConnectedRequest();

			for (auto& player : players) {
				if (player.id == -1) {
					player.id = packet->get_client_id();
					break;
				}
			}

			auto client = server.get_host_type()->get_client_manager().get_client(packet->get_peer());

			bs::Packet packet = server.create_client_connect_response(client->get_id(), TICK_RATE);
			packet.set_peer(client->get_peer());
			packet.send(true);
			break;
		}

		case Game::Any_ClientDisconnected: {
			const auto* client_msg = message->payload_as_ClientDisconnected();
		} break;

		case Game::Any_ClientReady: {
			const auto* client_msg = message->payload_as_ClientReady();
			for (size_t i = 0; i < std::size(players); ++i) {
				auto& player = players[i];

				if (player.id == packet->get_client_id()) {
					player.ready = client_msg->ready();
					server.get_logger()->trace("Client {} is ready: {}", packet->get_client_id(), player.ready);

					if (player.ready) {
						bs::Packet response = server.create_client_ready_response((int)i);
						response.set_peer(packet->get_peer());
						response.send(true);
					}

					break;
				}
			}

			// Once all players have readied up, start the game.
			if (players[0].ready && players[1].ready) {
				gameState = PLAYING;
				server.get_logger()->info("All players are ready starting game...");

				game_started = true;

				auto& player_1 = players[0];
				player_1.x = 5.0f;
				player_1.y = (HEIGHT / 2.0f) - PLAYER_HEIGHT / 2.0f;

				auto& player_2 = players[1];
				player_2.x = WIDTH - PLAYER_WIDTH - 5.0f;
				player_2.y = (HEIGHT / 2.0f) - PLAYER_HEIGHT / 2.0f;

				ball_x = WIDTH / 2.0f;
				ball_y = HEIGHT / 2.0f;

				ball_vx = BALL_INITIAL_SPEED;
				ball_vy = BALL_INITIAL_SPEED;

				bs::Packet start_packet = server.create_game_starting(player_1.x, player_1.y, player_2.x, player_2.y, ball_x, ball_y, ball_vx, ball_vy);
				server.get_host_type()->broadcast_to_clients(start_packet, true);
			}
		} break;

		case Game::Any_PlayerMoved: {
			const auto* player_msg = message->payload_as_PlayerMoved();
			const auto slot = player_msg->slot();

			server.get_logger()->trace("Player moved receieved for {}: Vel: {}", slot, player_msg->velocity());

			if (slot >= 0 && slot < std::size(players)) {
				auto& player = players[slot];
				player.y += player_msg->velocity();
			}
			else {
				server.get_logger()->error("Invalid player slot: {}", slot);
			}
			break;
		}

		default:
			server.get_logger()->error("Unknown message type");
		}
		});

	InitWindow(300, 300, "PongServer");

	// NOTE(DC): This will also control the server's tick rate.
	SetTargetFPS(60);

	while (!WindowShouldClose()) {
		server.tick(0);

		if (gameState == PLAYING) {
			// Send out the game state to all the clients.
			bs::Packet tick_packet = server.create_tick(players[0].x, players[0].y, players[1].x, players[1].y, ball_x, ball_y, ball_vx, ball_vy, players[0].score, players[1].score);
			server.get_host_type()->broadcast_to_clients(tick_packet, true);
		}

		BeginDrawing();

		int y = 0;
		int x = 10;
		for (auto& player : players) {
			DrawText(TextFormat("Player %d: %s", player.id, player.ready ? "Ready" : "Not Ready"), x, y += 20, 10, WHITE);
		}

		if (players[0].ready && players[1].ready) {
			DrawText("All players ready, starting game", x, y += 20, 10, WHITE);
		}

		DrawText("Connected Clients:", x, y += 20, 10, WHITE);

		auto connected_clients = server.get_host_type()->get_client_manager().get_connected_clients();
		for (auto& client : connected_clients) {
			DrawText(TextFormat("Client %d: %s", client->get_id(), client->get_state() == bs::Base_Client::CONNECTED ? "Connected" : "Disconnected"), x, y += 20, 10, WHITE);
		}

		DrawText(TextFormat("Game State %s", GameStateToString(gameState)), x, y += 20, 10, WHITE);
		switch (gameState) {
		case DISCONNECTED:
		case PLAYING: {
			for (auto& player : players) {
				DrawText(TextFormat("Player %d: Pos: %f, %f", player.id, player.x, player.y), x, y += 20, 10, WHITE);
			}
			DrawText(TextFormat("Ball: Pos: %.2f, %.2f", ball_x, ball_y), x, y += 20, 10, WHITE);
			DrawText(TextFormat("Ball: Vel: %.2f, %f Vel: %.2f, %f", ball_vx, ball_vy), x, y += 20, 10, WHITE);

			if (gameState != DISCONNECTED) {
				game_state_tick();
			}
			break;
		}
		}

		ClearBackground(BLACK);

		EndDrawing();

	}

	return EXIT_SUCCESS;
}
