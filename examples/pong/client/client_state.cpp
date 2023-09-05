#include "client_state.h"
#include "game_messages_generated.h"
#include "config.h"

#include <utils.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <raylib.h>

Client_State::Client_State()
	: m_client(spdlog::stdout_color_mt("CLIENT"), SAMPLES_HOST, SAMPLES_PORT)
{
	m_client.set_connect_callback([&] {
		// When we connect send over a connection request so we
		// can get our client id and store it. We shouldn't 
		// continue until we do so.
		m_client.create_client_connect_request().send(true);
		});

	m_client.set_disconnect_callback([&] {
		m_state = DISCONNECTED;
		});

	m_client.set_tick_callback([&](const Game::Message* message, const Packet* packet) {
		ASSERT_PANIC(m_client->get_state() == Base_Client::CONNECTED, "Not currently connected");

		// We can now switch on the type returned and act accordingly.
		switch (message->payload_type()) {
		case Game::Any_ClientConnectedResponse: {
			const auto* client_msg = message->payload_as_ClientConnectedResponse();
			auto id = client_msg->client_id();


			m_client.get_logger()->info("Server responded to connect with id: {}. Setting client id.", id);
			m_client->set_id(id);

			m_server_tick_rate = client_msg->tick_rate();
			break;
		}

		case Game::Any_ClientReadyResponse: {
			const auto* client_msg = message->payload_as_ClientReadyResponse();
			m_players[client_msg->slot()].isLocal = true;
			break;
		}


		case Game::Any_ClientDisconnected: {
			const auto* client_msg = message->payload_as_ClientDisconnected();
		} break;

		case Game::Any_GameStarting: {
			const auto* client_msg = message->payload_as_GameStarting();
			m_state = IN_GAME;
			const auto* player_1 = client_msg->player_1();
			const auto* player_2 = client_msg->player_2();

			auto update_player = [](Player& player, const Game::Player* player_msg) {
				player.x = player_msg->position()->x();
				player.y = player_msg->position()->y();
				};

			update_player(m_players[0], player_1);
			update_player(m_players[1], player_2);

			m_ball_x = client_msg->ball_position()->x();
			m_ball_y = client_msg->ball_position()->y();
			m_ball_vx = client_msg->ball_velocity()->x();
			m_ball_vy = client_msg->ball_velocity()->y();

		} break;

		case Game::Any_PlayerMoved: {
			const auto* client_msg = message->payload_as_PlayerMoved();
			m_players[client_msg->slot()].y += client_msg->velocity();

			break;
		}

		case Game::Any_Tick: {
			const auto* client_msg = message->payload_as_Tick();
			const auto* player_1 = client_msg->player_1();
			const auto* player_2 = client_msg->player_2();

			auto update_player = [](Player& player, const Game::Player* player_msg) {
				player.x = player_msg->position()->x();
				player.y = player_msg->position()->y();
				};

			update_player(m_players[0], player_1);
			update_player(m_players[1], player_2);

			m_ball_x = client_msg->ball_position()->x();
			m_ball_y = client_msg->ball_position()->y();
			m_ball_vx = client_msg->ball_velocity()->x();
			m_ball_vy = client_msg->ball_velocity()->y();


			break;
		}

		default:
			update(message);
			break;
		}

		});
}



void Client_State::tick(float dt) {
	m_client.tick(m_server_tick_rate);

	switch (m_state) {
	case IN_GAME: {
		for (int i = 0; i < std::size(m_players); ++i) {
			auto& player = m_players[i];

			if (!player.isLocal) {
				continue;
			}

			int velocity = 0;
			bool interacted = false;

			if (IsKeyDown(KEY_W)) {
				velocity = -PLAYER_SPEED;
				interacted = true;
			}
			else if (IsKeyDown(KEY_S)) {
				velocity = PLAYER_SPEED;
				interacted = true;
			}

			// Send a player moved message.
			if (interacted) {
				if (m_client.get_host_type()->get_state() == Base_Client::CONNECTED) {
					m_client.create_player_moved_message(i, velocity).send(true);
				}

				player.y += velocity * dt;
			}
		}

		break;
	}
	}
}

void Client_State::draw() {
	BeginDrawing();
	ClearBackground(BLACK);

	switch (m_state) {
	case WAITING: {
		DrawText("Waiting for game, press space to ready up", 10, 10, 20, WHITE);
		DrawText(TextFormat("Ready: %s", m_ready ? "YES" : "NO"), 10, 40, 20, WHITE);
		if (IsKeyPressed(KEY_SPACE)) {
			m_ready = !m_ready;
			m_client.create_client_ready(m_ready).send(true);
		}
		break;
	}

	case IN_GAME:
		for (int i = 0; i < std::size(m_players); ++i) {
			auto& player = m_players[i];

			if (player.isLocal) {
				DrawText(TextFormat("Slot ID: %d", i), 10, 10, 10, WHITE);
			}
			DrawRectangle((int)player.x, (int)player.y, PLAYER_WIDTH, PLAYER_HEIGHT, RAYWHITE);
		}
		DrawRectangle((int)m_ball_x, (int)m_ball_y, BALL_WIDTH, BALL_WIDTH, WHITE);

		DrawText(TextFormat("%d", m_players[0].score), 10, HEIGHT - 30, 20, WHITE);
		DrawText(TextFormat("%d", m_players[1].score), WIDTH - 30, HEIGHT - 30, 20, WHITE);
		break;

	case ENDED: break;

	case DISCONNECTED: {
		DrawText("Disconnected!", 10, 10, 20, WHITE);
		break;
	}
	}


	EndDrawing();
}

void Client_State::update(const Game::Message* message) {
	//ASSERT_PANIC(m_state == IN_GAME, "Trying to update the game state while not in game");
	//ASSERT_PANIC(message->payload_type() == Game::Any_Tick, "Game state receieved message. Expected Game::Any_Tick but got: {}", Game::EnumNameAny(message->payload_type()));

	const auto* client_msg = message->payload_as_Tick();
	const auto* player_1 = client_msg->player_1();
	const auto* player_2 = client_msg->player_1();

	auto update_player = [](Player& player, const Game::Player* player_msg) {
		player.x = player_msg->position()->x();
		player.y = player_msg->position()->y();
		};

	update_player(m_players[0], player_1);
	update_player(m_players[1], player_2);

	m_ball_x = client_msg->ball_position()->x();
	m_ball_y = client_msg->ball_position()->y();
	m_ball_vx = client_msg->ball_velocity()->x();
	m_ball_vy = client_msg->ball_velocity()->y();
}

