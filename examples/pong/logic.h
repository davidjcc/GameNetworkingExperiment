#pragma once

#include "base_game_host.h"

namespace Game {
	struct Message;
}

class GameState {
public:
	enum State {
		NONE = 0,
		IN_GAME,
		WAITING,
		ENDED
	};

	GameState();

	void tick(float dt);
	void draw();

	void update(const Game::Message* message);

	State set_state(const GameState::State& state) { m_state = state; }
	State get_state() const { return m_state; }

private:
	struct Player {
		float x = 0.0f;
		float y = 0.0f;
		bool ready = false;

		bool isLocal = false;
	} m_players[2];

	float m_ball_x = 0.0f;
	float m_ball_y = 0.0f;
	float m_ball_vx = 0.0f;
	float m_ball_vy = 0.0f;

	State m_state = WAITING;
	bool m_ready = false;

	Game_Host<Host_Client> m_client;

	// Server tick rate is set once the server replies with a 
	// client ready response.
	int m_server_tick_rate = 0;
};

