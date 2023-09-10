#include "client_state.h"
#include "game_messages_generated.h"
#include "config.h"

#include <utils.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <raylib.h>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

const float BUTTON_SZ_W = 200;
const float BUTTON_SZ_H = 75;
const float BUTTON_Y_PADDING = 20;

Pong_Client_State::Pong_Client_State()
	: m_client(spdlog::stdout_color_mt("CLIENT"), SAMPLES_HOST, SAMPLES_PORT)
{
	m_client.set_connect_callback([&] {
		// When we connect send over a connection request so we
		// can get our client id and store it. We shouldn't 
		// continue until we do so.
		m_client.create_client_connect_request().send(true);
		});

	m_client.set_disconnect_callback([&] {
		});

	m_client.set_tick_callback([&](const Game::Message* message, const bs::Packet* packet) {
		ASSERT_PANIC(m_client->get_state() == bs::Base_Client::CONNECTED, "Not currently connected");

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
			m_players[client_msg->slot()].is_local = true;
			break;
		}

		case Game::Any_ClientDisconnected: {
			const auto* client_msg = message->payload_as_ClientDisconnected();
		} break;

		case Game::Any_GameStarting: {
			const auto* client_msg = message->payload_as_GameStarting();
			m_state = MULTIPLAYER_IN_GAME;
			const auto* player_1 = client_msg->player_1();
			const auto* player_2 = client_msg->player_2();

			auto update_player = [](Player& player, const Game::Player* player_msg) {
				player.x = player_msg->position()->x();
				player.y = player_msg->position()->y();
				player.score = player_msg->score();
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
				player.score = player_msg->score();
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
			server_update(message);
			break;
		}

		});
}

void Pong_Client_State::tick(float dt) {
	m_client.tick(m_server_tick_rate);

	switch (m_state) {
	case SINGLE_PLAYER:
	case MULTIPLAYER_IN_GAME: {
		bool is_single_player = m_state == SINGLE_PLAYER;

		if (is_single_player)
			game_state_tick(m_players, m_ball_x, m_ball_y, m_ball_vx, m_ball_vy);

		for (int i = 0; i < std::size(m_players); ++i) {
			auto& player = m_players[i];

			const float SPEED = is_single_player ? PLAYER_SPEED * 2000.0f * dt : PLAYER_SPEED;

			if (!player.is_local) {
				// AI Player controls.
				if (is_single_player) {
					if (m_ball_x > WIDTH / 2.0f && m_ball_vx > 0.0f) {
						if (player.y + PLAYER_HEIGHT / 2.0f > m_ball_y) {
							player.y -= SPEED * dt;
						}
						else if (player.y + PLAYER_HEIGHT / 2.0f < m_ball_y) {
							player.y += SPEED * dt;
						}
					}
				}
				continue;
			}

			int velocity = 0;
			bool interacted = false;


			if (IsKeyDown(KEY_W)) {
				velocity = -SPEED;
				interacted = true;
			}
			else if (IsKeyDown(KEY_S)) {
				velocity = SPEED;
				interacted = true;
			}

			// Send a player moved message.
			if (interacted) {
				if (!is_single_player && (m_client.get_host_type()->get_state() == bs::Base_Client::CONNECTED)) {
					m_client.create_player_moved_message(i, velocity).send(true);
				}

				player.y += velocity * dt;
			}
		}

		break;
	}
	}
}

void Pong_Client_State::draw() {
	BeginDrawing();
	ClearBackground(BLACK);

	DrawFPS(WIDTH * 0.91f, 0.0f);

	switch (m_state) {
	case NONE: {
		float y = 10;
		DrawText("PONG!", WIDTH / 2 - MeasureText("PONG!", 60) / 2, y += 60, 60, RAYWHITE);

		if (GuiButton({ WIDTH / 2 - (BUTTON_SZ_W / 2), y += BUTTON_SZ_H + BUTTON_Y_PADDING, BUTTON_SZ_W, BUTTON_SZ_H }, "Single Player")) {
			m_state = SINGLE_PLAYER;

			auto& player_1 = m_players[0];
			player_1.x = 5.0f;
			player_1.y = (HEIGHT / 2.0f) - PLAYER_HEIGHT / 2.0f;

			// The left player will be the local player in single player.
			player_1.is_local = true;

			auto& player_2 = m_players[1];
			player_2.x = WIDTH - PLAYER_WIDTH - 5.0f;
			player_2.y = (HEIGHT / 2.0f) - PLAYER_HEIGHT / 2.0f;

			m_ball_x = WIDTH / 2.0f;
			m_ball_y = HEIGHT / 2.0f;

			m_ball_vx = BALL_INITIAL_SPEED;
			m_ball_vy = BALL_INITIAL_SPEED;
		}

		if (GuiButton({ WIDTH / 2 - (BUTTON_SZ_W / 2), y += BUTTON_SZ_H + BUTTON_Y_PADDING, BUTTON_SZ_W, BUTTON_SZ_H }, "Multiplayer Player")) {
			m_state = MULTIPLAYER_WAITING;

			m_connecting_timer = 10.0f;
		}

		break;
	}

	case MULTIPLAYER_WAITING: {
		if (is_connected()) {
			float y = 10.0f;
			DrawText(TextFormat("Connected to server (%s:%d)", m_client->get_host_address(), m_client->get_port()), 10, y, 20, WHITE);
			DrawText("Waiting for game, press space to ready up", 10, y += 40, 20, WHITE);
			DrawText("Ready: ", 10, y += 40, 20, WHITE);
			DrawText(TextFormat("%s", m_ready ? "YES" : "NO"), 10 + MeasureText("Ready: :", 20), y, 20, m_ready ? GREEN : RED);
			if (IsKeyPressed(KEY_SPACE)) {
				m_ready = !m_ready;
				m_client.create_client_ready(m_ready).send(m_ready);
			}

			m_connecting_timer = 0.0f;
		}
		else {
			if (m_connecting_timer <= 0.0f) {
				m_connecting_timer = 0.0f;
				DrawText("Timed out", 10, 10, 20, WHITE);
				if (GuiButton({ 10, 20, 100, 50 }, "Retry")) {
					m_connecting_timer = 10.0f;
				}
			}
			else {
				m_connecting_timer -= GetFrameTime();
				DrawText(TextFormat("Connecting to server... (%.1fs)", m_connecting_timer), 10, 10, 20, WHITE);
			}
		}

		if (GuiButton({ WIDTH - (BUTTON_SZ_W / 2), 0 + BUTTON_Y_PADDING, (BUTTON_SZ_W / 2), (BUTTON_SZ_H / 2) }, "Menu")) {
			m_state = NONE;

			// Unready the player before we go back to the menu.
			if (is_connected()) {
				m_ready = false;
				m_client.create_client_ready(m_ready).send(m_ready);
			}
		}
		break;
	}

	case SINGLE_PLAYER:
	case MULTIPLAYER_IN_GAME:
		for (int i = 0; i < std::size(m_players); ++i) {
			auto& player = m_players[i];

			if (player.is_local && m_state == MULTIPLAYER_IN_GAME) {
				DrawText(TextFormat("Slot ID: %d", i), 10, 10, 10, WHITE);
			}
			DrawRectangle((int)player.x, (int)player.y, PLAYER_WIDTH, PLAYER_HEIGHT, RAYWHITE);
		}
		DrawRectangle((int)m_ball_x, (int)m_ball_y, BALL_WIDTH, BALL_WIDTH, WHITE);
		DrawText(TextFormat("%d", m_players[0].score), 50, HEIGHT - 50, 50, WHITE);
		DrawText(TextFormat("%d", m_players[1].score), WIDTH - 100, HEIGHT - 50, 50, WHITE);

		break;

	case MULTIPLAYER_ENDED: break;

	case DISCONNECTED: {
		DrawText("Disconnected!", 10, 10, 20, WHITE);
		break;
	}
	}

	EndDrawing();
}

void Pong_Client_State::server_update(const Game::Message* message) {
	ASSERT_PANIC(m_state == MULTIPLAYER_IN_GAME, "Trying to update the game state while not in game");
	ASSERT_PANIC(message->payload_type() == Game::Any_Tick, "Game state receieved message. Expected Game::Any_Tick but got: {}", Game::EnumNameAny(message->payload_type()));

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

