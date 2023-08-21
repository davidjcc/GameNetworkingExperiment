
#include <iostream>
#include <algorithm>
#include <filesystem>
namespace fs = std::filesystem;

#include <spdlog/spdlog.h>
#include <raylib.h>
#include <nlohmann/json.hpp>

#include "rlImGui.h"

#define JSON_SERIALISE_DEFINE
#include "state.h"

#include "assets.h"
#include "client.h"

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT WINDOW_WIDTH / 16 * 9
#define PLAYER_WIDTH 20.0f
#define PLAYER_HEIGHT 100.0f
#define PLAYER_SPEED 0.05f
#define BALL_SIZE 10.0f
#define INITIAL_BALL_SPEED 0.04f
#define BALL_SPEED_INCREASE_PER_HIT 0.001f
#define NUM_PLAYERS 2

#define UPDATE_TIMER(TIMER) if (TIMER > 0.0f) {TIMER -= GetFrameTime();} else { TIMER = 0.0f; }
#define RESET_TIMER(TIMER, RESET_TIME) TIMER = RESET_TIME;

#define DEFAULT_RESET_TIME 2.0f

static std::string s_assetsDir = "assets";
static uint32_t latency = 0;
static ServerState serverState{};

static int GetRandomDirection() {
	return GetRandomValue(0, 100) > 50 ? -1 : 1;
}

struct Player {
	float x, y, width, height;
	bool isAi = false;
	float score;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Player, x, y, width, height, isAi, score);

struct Ball {
	float x, y, width, height;
	float velX, velY;
	float speed;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Ball, x, y, width, height, velX, velY, speed);

struct State {
	Player left;
	Player right;
	Ball ball = {};
	float resetTimer = 0.0f;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(State, left, right, ball, resetTimer);

void ResetBall(State& state) {
	state.ball.x = WINDOW_WIDTH / 2 - BALL_SIZE / 2;
	state.ball.y = WINDOW_HEIGHT / 2 - BALL_SIZE / 2;

	state.ball.width = BALL_SIZE;
	state.ball.height = BALL_SIZE;
	state.ball.velX = (float)GetRandomDirection();
	state.ball.velY = -1.0f;
	state.ball.speed = INITIAL_BALL_SPEED;
}

void Init(State& state, bool resetScores = false, bool leftAi = false, bool rightAi = true) {
	float playerInitialY = (float)WINDOW_HEIGHT * .5f - PLAYER_HEIGHT * .5f;
	float xBuffer = 10.0f;

	state.left = { xBuffer, playerInitialY, PLAYER_WIDTH, PLAYER_HEIGHT };
	state.left.isAi = leftAi;

	state.right = { (WINDOW_WIDTH - PLAYER_WIDTH) - xBuffer, playerInitialY, PLAYER_WIDTH, PLAYER_HEIGHT };
	state.right.isAi = rightAi;

	if (resetScores) {
		state.left.score = 0.0f;
		state.right.score = 0.0f;
	}

	ResetBall(state);
	LoadAssets(s_assetsDir);
}

bool IsRectOutOfBounds(const Rectangle& rect) {
	return (rect.y <= 0) || (rect.y + PLAYER_HEIGHT >= WINDOW_HEIGHT);
}

Color GetColour(const State& state) {
	return WHITE;
}

void DrawCentreLine(const State& state) {
	if (state.resetTimer <= 0.0f) {
		float width = 10.0f;
		float height = 30.0f;
		for (float y = 0.0f; y < WINDOW_HEIGHT; y += (height + 5.0f)) {
			DrawRectangle((int)(WINDOW_WIDTH / 2 - width / 2), (int)y, (int)width, (int)height, GetColour(state));
		}
	}
}

void Draw(const State& state) {
	DrawCentreLine(state);

	auto colour = GetColour(state);
	DrawRectangle((int)state.left.x, (int)state.left.y, (int)state.left.width, (int)state.left.height, colour);
	DrawRectangle((int)state.right.x, (int)state.right.y, (int)state.right.width, (int)state.right.height, colour);
	DrawRectangle((int)state.ball.x, (int)state.ball.y, (int)state.ball.width, (int)state.ball.height, colour);
}

Rectangle rect(Player& player) { return Rectangle{ player.x, player.y, player.width, player.height }; }
Rectangle rect(Ball& ball) { return Rectangle{ ball.x, ball.y, ball.width, ball.height }; }

void Update(State& state) {
	UPDATE_TIMER(state.resetTimer);

	// If the reset timer has been set then,
	// stop the ball moving temporarily.
	if (state.resetTimer <= 0.0f) {
		state.ball.x += (state.ball.velX * state.ball.speed);
		state.ball.y += (state.ball.velY * state.ball.speed);
	}

	// Ball roof/floor collision
	if (state.ball.y <= 0) {
		state.ball.velY *= -1.0f;
	}
	else if (state.ball.y >= WINDOW_HEIGHT - state.ball.width) {
		state.ball.velY *= -1;
	}

	// Check right paddle win.
	if (state.ball.x < 0) {
		state.right.score++;
		state.resetTimer = DEFAULT_RESET_TIME;
		ResetBall(state);
		PlaySound(LoseSound);
	}

	// Check left paddle win.
	if (state.ball.x - BALL_SIZE > WINDOW_WIDTH) {
		state.resetTimer = DEFAULT_RESET_TIME;
		state.left.score++;
		ResetBall(state);
		PlaySound(WinSound);
	}

	// Ball-Player collision
	Player* players[] = { &state.left, &state.right };
	for (size_t i = 0; i < std::size(players); ++i) {
		Player* player = players[i];

		if (CheckCollisionRecs(rect(*player), rect(state.ball))) {
			state.ball.velX *= -1.0f;
			state.ball.velY = (float)GetRandomDirection();

			state.ball.speed += BALL_SPEED_INCREASE_PER_HIT;
		}

		// Handle any AI players.
		if (!player->isAi) {
			continue;
		}

		// If the ball isn't in the AI players side then don't do anything.
		float ballCentre = state.ball.x - state.ball.width / 2.0f;
		bool isMyTurn = state.ball.velX > 0.0f ?
			player->x > WINDOW_WIDTH / 2 && ballCentre > WINDOW_WIDTH / 2:
		player->x < WINDOW_WIDTH / 2 && ballCentre < WINDOW_WIDTH / 2;

		if (!isMyTurn) {
			continue;
		}

		// Automatically shift the y position to the same as the ball's
		// TODO(DC): Add some randomness to this.
		float targetY = state.ball.y;
		float centreY = (player->y + player->height / 2.0f);

		if (targetY < centreY) {
			player->y -= PLAYER_SPEED;
		}
		else {
			player->y += PLAYER_SPEED;
		}

		if (IsRectOutOfBounds(rect(*player))) {
			if (player->y < 0) {
				player->y = 1;
			}
			else {
				player->y = WINDOW_HEIGHT - player->height - 1;
			}
		}
	}

	// Player input
	if (IsKeyDown(KEY_UP)) {
		if (!IsRectOutOfBounds(rect(state.left))) {
			state.left.y -= PLAYER_SPEED;
		}
		else {
			state.left.y = 1;
		}
	}
	else if (IsKeyDown(KEY_DOWN)) {
		if (!IsRectOutOfBounds(rect(state.left))) {
			state.left.y += PLAYER_SPEED;
		}
		else {
			state.left.y = WINDOW_HEIGHT - PLAYER_HEIGHT - 1;
		}
	}
}

static Lobby lobby{};
std::array<std::string, MAX_CLIENTS_PER_LOBBY> clientNames{};
bool isInGame = false;

static void DrawUI() {
	if (isInGame) {
		return;
	}

	rlImGuiBegin();
	{
		ImGui::Begin("Multi-player");
		ImGui::Text("%s", fmt::format("Frame time: {}s", GetFrameTime()).c_str());
		ImGui::Text("Server status: %s", Net::IsConnected() ? "Connected" : "Disconnected");
		ImGui::Text("%s", fmt::format("Ping: {}ms", latency).c_str());
		if (ImGui::Button("Ping")) {
			Net::Request::Ping((uint32_t)(GetTime() * 1000.0f));
		}

		if (Net::IsConnected()) {
			if (ImGui::Button("Refresh")) {
				Net::Request::GetState();
			}
			ImGui::Text("Client name: %s", Net::GetClientName().c_str());
			if (ImGui::CollapsingHeader("Lobbies", ImGuiTreeNodeFlags_DefaultOpen)) {
				if (!Net::IsInLobby()) {
					static char lobbyName[64];
					ImGui::Text("Enter Lobby Name: "); ImGui::SameLine();
					ImGui::SetNextItemWidth(100.0f);
					ImGui::InputText("##LOBBY_NAME", lobbyName, 64);
					if (ImGui::Button("Create")) {
						Net::Request::CreateLobby(lobbyName);
						memset(lobbyName, 0, sizeof(lobbyName));
					}

					if (serverState.activeLobbies > 0) {
						for (auto& lobby : serverState.lobbies) {
							ImGui::Text("ID: %i", lobby.id);
							ImGui::Text("Name: %s", lobby.name.c_str());

							int ctr = 0;
							for (size_t i = 0; i < lobby.clients.size(); ++i) {
								auto& client = lobby.clients[i];

								ImGui::Text("%s", fmt::format("Client: {}", client == CLIENT_STATE_EMPTY ? "(empty)" : clientNames[i]).c_str());

								if (client == CLIENT_STATE_EMPTY) {
									ImGui::SameLine();
									if (ImGui::Button(fmt::format("Join##{}", ctr++).c_str())) {
										Net::Request::JoinLobby(lobby.id);
									}
								}
							}
							ImGui::Text("");
						}
					}
					else {
						ImGui::Text("No lobbies");
					}
				}
				else {
					static bool ready = false;
					ImGui::Text("In lobby: %i", Net::GetLobbyID());

					if (ImGui::Checkbox("Ready", &ready)) {
						Net::Request::SetReady(ready);
					}
				}
			}
		}

		ImGui::End();
	}
	rlImGuiEnd();

}

static std::array<ClientState, MAX_CLIENTS_PER_LOBBY> clients;
void DrawGame() {
	for (auto& client : clients) {
		// Draw self and other players.
		if (client.status == CLIENT_STATUS_IN_GAME || client.name.empty()) {
			DrawRectangle((int32_t)client.x, (int32_t)client.y, 50, 50, RED);
		}

		// Handle local player input.
		if (client.isLocal) {
			Vector2 velocity = { 0.0f, 0.0f };
			float speed = 100.0f;
			if (IsKeyDown(KEY_W)) {
				velocity.y = -speed * GetFrameTime();
			}
			else if (IsKeyDown(KEY_S)) {
				velocity.y = speed * GetFrameTime();
			}
			else if (IsKeyDown(KEY_A)) {
				velocity.x = -speed * GetFrameTime();
			}
			else if (IsKeyDown(KEY_D)) {
				velocity.x = speed * GetFrameTime();
			}

			client.x += velocity.x;
			client.y += velocity.y;

			if ((velocity.x != 0.0f) || velocity.y != 0.0f) {
				Net::Request::PlayerMoved(client.x, client.y);
			}
		}
	}

}

int main(int argc, char* argv[])
{
	if (argc > 1) {
		s_assetsDir = argv[1];
	}

	InitAudioDevice();

	State state{};

	SetRandomSeed((unsigned int)time(NULL));
	InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Pong");
	rlImGuiSetup(true);

	Init(state, true, false, true);
	if (!Net::InitClient()) {
		spdlog::error("Error initialising Net::Client.");
		return EXIT_FAILURE;
	}

	Vector2 pos = { 10.0f, 10.0f };


	float pingTimer = 5.0f;
	float lobbyUpdateTimer = 5.0f;
	bool showUI = false;
	bool running = true;
	while (running)
	{
		if (WindowShouldClose()) {
			running = false;
		}
		UPDATE_TIMER(pingTimer);
		if (!isInGame && Net::IsInLobby()) {
			UPDATE_TIMER(lobbyUpdateTimer);

			if (lobbyUpdateTimer <= 0.0f) {
				Net::Request::UpdateLobby();
				RESET_TIMER(lobbyUpdateTimer, 5.0f);
			}
		}

		if (pingTimer <= 0.0f) {
			// Get the current time in milliseconds.
			const uint32_t timestamp = (uint32_t)(GetTime() * 1000.0f);
			Net::Request::Ping(timestamp);
			RESET_TIMER(pingTimer, 5.0f);
		}

		BeginDrawing();
		{
			ClearBackground(BLACK);

			Net::PollServer([](const Packet& packet) {
				// TODO(DC): It might be possible to get an error packet back and
				// we still to process it but for now to cover all bases just assert
				// if it isn't a success.
				//assert(packet.processedState == PACKET_STATE_SUCCESS);

				switch (packet.type) {
				case PACKET_TYPE_GET_STATE: {
					serverState = JsonToServerState(packet.data);
				} break;
				case PACKET_TYPE_PING:

				case PACKET_TYPE_PONG: {
					float current = (float)GetTime();
					auto currentMs = (uint32_t)(current * 1000.0f);
					latency = currentMs - packet.timestampMs;
				} break;

				case PACKET_TYPE_CLIENT_UPDATE_LOBBY: {
					if (strlen(packet.data) > 0) {
						nlohmann::json json = nlohmann::json::parse(packet.data);
						from_json(json, lobby);
					}
				} break;

				case PACKET_TYPE_CLIENT_JOINED_LOBBY: {
					auto json = nlohmann::json(nlohmann::json::parse(packet.data));

					client_id clientId = json["client"].get<client_id>();
					auto clientName = json["clientName"].get<std::string>();
					lobby_id lobbyId = json["lobby"].get<lobby_id>();
					int32_t slot = json["slot"].get<int32_t>();

					clientNames[slot] = clientName;

					spdlog::info("Player {} has joined lobby {}", clientId, lobbyId);
				} break;

				case PACKET_TYPE_SET_READY: {
					nlohmann::json json = nlohmann::json::parse(packet.data);
					size_t idx = json["lobbyIdx"];
					clients[idx].isLocal = true;
				} break;

				case PACKET_TYPE_GAME_START: {
					isInGame = true;
					nlohmann::json json = nlohmann::json::parse(packet.data);
					for (size_t i = 0; i < clients.size(); ++i) {
						clients[i].x = json[i]["x"];
						clients[i].y = json[i]["y"];
					}

				} break;

				case PACKET_TYPE_PLAYER_MOVED: {
					if (!isInGame) {
						spdlog::error("Received a player move packet but we're not currently in a game");
						assert(isInGame);
					}

					for (auto& client : clients) {
						if (client.isLocal) {
							continue;
						}

						nlohmann::json json = nlohmann::json::parse(packet.data);
						client.x = json["x"];
						client.y = json["y"];
					}
				}

											 // Net::Client handle's these.
				case PACKET_TYPE_CLIENT_RECEIVE_NAME:
				case PACKET_TYPE_JOIN_LOBBY:
					break;

				default:
					fmt::print("Unknown message type: {}\n", PacketTypeToString(packet.type));
					break;
				}
				});

			DrawGame();

			if (showUI) {
				DrawUI();
			}

			if (IsKeyPressed(KEY_F1)) {
				showUI = !showUI;
			}
		}
		EndDrawing();
	}
	rlImGuiShutdown();

	Net::Shutdown();
	UnloadAssets();

	CloseWindow();

	return 0;
}
