
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

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 450
#define PLAYER_WIDTH 20.0f
#define PLAYER_HEIGHT 100.0f
#define PLAYER_SPEED 0.05f
#define BALL_SIZE 10.0f
#define INITIAL_BALL_SPEED 0.04f
#define BALL_SPEED_INCREASE_PER_HIT 0.001f
#define NUM_PLAYERS 2

#define UPDATE_TIMER(TIMER) if (TIMER > 0.0f) {TIMER -= GetFrameTime();} else { TIMER = 0.0f; }

#define RESET_TIME 2.0f

static std::string s_assetsDir = "assets";

static int GetRandomDirection() {
	return GetRandomValue(0, 100) > 50 ? -1 : 1;
}

void ResetBall(State& state) {
	state.ball.x = WINDOW_WIDTH / 2 - BALL_SIZE / 2;
	state.ball.y = WINDOW_HEIGHT / 2 - BALL_SIZE / 2;

	state.ball.width = BALL_SIZE;
	state.ball.height = BALL_SIZE;
	state.ball.velX = (float)GetRandomDirection();
	state.ball.velY = -1.0f;
	state.ball.speed = INITIAL_BALL_SPEED;
}

void Init(State& state, bool resetScores = false, bool leftAi = false, bool rightAi = true, bool isDemo = false) {
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

	state.isDemo = isDemo;
	ResetBall(state);
	LoadAssets(s_assetsDir);
}

bool IsRectOutOfBounds(const Rectangle& rect) {
	return (rect.y <= 0) || (rect.y + PLAYER_HEIGHT >= WINDOW_HEIGHT);
}

Color GetColour(const State& state) {
	auto colour = Color{ 255, 255, 255, 255 };
	if (state.isDemo) {
		colour.a = 100;
	}

	return colour;
}

void DrawCentreLine(const State& state) {
	if (state.resetTimer <= 0.0f && !state.isDemo) {
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
		state.resetTimer = RESET_TIME;
		ResetBall(state);

		if (!state.isDemo) {
			PlaySound(LoseSound);
		}
	}

	// Check left paddle win.
	if (state.ball.x - BALL_SIZE > WINDOW_WIDTH) {
		state.resetTimer = RESET_TIME;
		state.left.score++;
		ResetBall(state);

		if (!state.isDemo) {
			PlaySound(WinSound);
		}
	}

	// Ball-Player collision
	Player* players[] = { &state.left, &state.right };
	for (size_t i = 0; i < std::size(players); ++i) {
		Player* player = players[i];

		if (CheckCollisionRecs(rect(*player), rect(state.ball))) {
			state.ball.velX *= -1.0f;
			state.ball.velY = (float)GetRandomDirection();

			// Don't increase the ball speed if in demo mode.
			if (!state.isDemo) {
				state.ball.speed += BALL_SPEED_INCREASE_PER_HIT;
			}
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

	if (!state.isDemo) {
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
}

enum GameState {
	GameState_Menu = 0,
	GameState_PlayingSinglePlayer,

	GameState_MenuMultiPlayer,
	GameState_PlayingMultiPlayer,

	GameState_CreateGameInput,

	GameState_JoinGame,
};

bool IsInPlayingState(const GameState& state) {
	switch (state) {
	case GameState_Menu:
	case GameState_PlayingSinglePlayer:
	case GameState_CreateGameInput:
	case GameState_JoinGame:
		return true;

	default:
		return false;

	}
}

static char CreateGameLobbyName[128];
static char JoinGameServerAddress[128];

static void DrawServerStatus(GameState& gameState) {
	if (!IsInPlayingState(gameState)) {
		DrawText("Server status: ", 10, WINDOW_HEIGHT - 20, 20, WHITE);

		std::string status = Net::IsConnected() ? "Connected" : "Disconnected";
		if (Net::IsConnected()) {
			float xPos = 10 + MeasureText("Server status: ", 20);
			DrawText(status.c_str(), xPos, WINDOW_HEIGHT - 20, 20, GREEN);

			std::string clientName = fmt::format("Client Name: {}", Net::GetClientName());
			xPos += MeasureText(clientName.c_str(), 20);
			DrawText(clientName.c_str(), xPos, WINDOW_HEIGHT - 20, 20, WHITE);
		}
		else {
			DrawText(status.c_str(), 10 + MeasureText("Server status: ", 20), WINDOW_HEIGHT - 20, 20, RED);
		}
	}
}

static void DrawMenu(GameState& gameState, State& state) {
	DrawServerStatus(gameState);

	switch (gameState) {
	case GameState_Menu: {
		DrawText("PONG", ((WINDOW_WIDTH / 2) - MeasureText("PONG", 50) / 2), 10, 50, WHITE);

		float yCursor = 100.0f;
		float buttonW = 200.0f;
		float buttonH = 50.0f;

#if 0
		if (GuiButton(Rectangle{ WINDOW_WIDTH / 2 - buttonW / 2.0f, yCursor, buttonW, buttonH }, "Single Player")) {
			gameState = GameState_PlayingSinglePlayer;
			Init(state, true, false, true, false);
			state.resetTimer = RESET_TIME;
		}

		yCursor += buttonH + 10;
		if (GuiButton(Rectangle{ WINDOW_WIDTH / 2 - 100, yCursor, buttonW, buttonH }, "Multi-Player")) {
			gameState = GameState_MenuMultiPlayer;
		}

		yCursor += buttonH + 10;
		if (GuiButton(Rectangle{ WINDOW_WIDTH / 2 - 100, yCursor, buttonW, buttonH }, "Test Client Ping")) {
			Net::Request::Ping();
		}
#endif

	} break;

	case GameState_MenuMultiPlayer: {
		DrawText("MULTIPLAYER", ((WINDOW_WIDTH / 2) - MeasureText("MULTIPLAYER", 50) / 2), 10, 50, WHITE);
#if 0
		if (GuiButton(Rectangle{ WINDOW_WIDTH - 100, 10, 50, 50 }, "CLOSE")) {
			gameState = GameState_Menu;
		}
#endif

		float yCursor = 100.0f;
		float buttonW = 200.0f;
		float buttonH = 50.0f;

#if 0
		if (GuiButton(Rectangle{ WINDOW_WIDTH / 2 - buttonW / 2.0f, yCursor, buttonW, buttonH }, "Create Game")) {
			gameState = GameState_CreateGameInput;
		}

		yCursor += buttonH + 10;
		if (GuiButton(Rectangle{ WINDOW_WIDTH / 2 - 100, yCursor, buttonW, buttonH }, "Join Game")) {
			gameState = GameState_JoinGame;

		}
#endif
	} break;

	case GameState_CreateGameInput: {
		DrawText("CREATE GAME", ((WINDOW_WIDTH / 2) - MeasureText("CREATE GAME", 50) / 2), 10, 50, WHITE);

#if 0
		int result = GuiTextInputBox(Rectangle{ WINDOW_WIDTH / 2 - 120, WINDOW_HEIGHT / 2 - 60, 240, 140 }, "", "Enter the Lobby name", "Ok;Cancel", CreateGameLobbyName, 128, NULL);
		if (result == 1) {
			// Transition to new online game.
		}
		else if (result > 1) {
			strcpy_s(CreateGameLobbyName, sizeof(CreateGameLobbyName), "\0");
			gameState = GameState_MenuMultiPlayer;
		}
#endif
	} break;

	case GameState_JoinGame: {
		DrawText("JOIN GAME", ((WINDOW_WIDTH / 2) - MeasureText("JOIN GAME", 50) / 2), 10, 50, WHITE);

#if 0
		int result = GuiTextInputBox(Rectangle{ WINDOW_WIDTH / 2 - 120, WINDOW_HEIGHT / 2 - 60, 240, 140 }, "", "Enter the server address", "Ok;Cancel", JoinGameServerAddress, 128, NULL);
		if (result == 1) {
			// Transition to joined game.
		}
		else if (result > 1) {
			strcpy_s(JoinGameServerAddress, sizeof(JoinGameServerAddress), "\0");
			gameState = GameState_MenuMultiPlayer;
		}
#endif
	} break;
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

	GameState gameState = GameState_Menu;

	//GuiLoadStyleDefault();
	rlImGuiSetup(true);

	Init(state, true, true, true, true);
	if (!Net::InitClient()) {
		spdlog::error("Error initialising Net::Client.");
		return EXIT_FAILURE;
	}

	ServerState serverState{};
	Net::Request::ListLobbies();

	bool running = true;
	while (running)
	{
		if (WindowShouldClose()) {
			running = false;
		}
		BeginDrawing();
		{
			ClearBackground(BLACK);

			auto colour = GetColour(state);
			if (!state.isDemo) {
				DrawText(TextFormat("%i", state.left.score), 100, WINDOW_HEIGHT - 50, 50, colour);
				DrawText(TextFormat("%i", state.right.score), WINDOW_WIDTH - 100, WINDOW_HEIGHT - 50, 50, colour);
			}

			if (Message message = Net::PollServer(); message.type != MessageType_None) {
				switch (message.type) {
				case MessageType_Ping: {
				} break;

				case MessageType_ListLobbies: {
					serverState.FromJson(nlohmann::json::parse(message.data));
				} break;

				default:
					fmt::print("Unknown message type: {}\n", MessageTypeString[message.type]);
					break;
				}
			}

			Update(state);
			Draw(state);

			rlImGuiBegin();
			{
				ImGui::Begin("Multi-player");
				ImGui::Text("Server status: %s", Net::IsConnected() ? "Connected" : "Disconnected");

				if (Net::IsConnected()) {
					ImGui::Text("Client name: %s", Net::GetClientName().c_str());
					if (ImGui::CollapsingHeader("Lobbies", ImGuiTreeNodeFlags_DefaultOpen)) {
						if (ImGui::Button("Update")) {
							Net::Request::ListLobbies();
						}

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
								for (auto& client : lobby.clients) {
									ImGui::Text("Client: %s", client ? client->name.c_str() : "(empty)");

									if (client == nullptr) {
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
				}

				if (ImGui::Begin("Server state")) {
					std::string json = serverState.ToJsonString(2);
					ImGui::Text(json.c_str());
					ImGui::End();
				}

				ImGui::End();
			}
			rlImGuiEnd();


			DrawMenu(gameState, state);

			if (IsKeyPressed(KEY_R)) {
				state.resetTimer = RESET_TIME;
				Init(state, true, false, true, false);
			}

			if (state.resetTimer > 0.0f) {
				char buf[10];
				sprintf_s(buf, "%.01f", state.resetTimer);
				DrawText(buf, ((WINDOW_WIDTH / 2) - MeasureText(buf, 50) / 2), 10, 50, WHITE);

				const char* help = "Press Up and Down arrows to move Paddle";
				DrawText(help, ((WINDOW_WIDTH / 2) - MeasureText(help, 30) / 2), 50, 30, WHITE);
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
