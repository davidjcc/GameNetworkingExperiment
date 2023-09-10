#pragma once

#include <raylib.h>

#define SAMPLES_HOST "127.0.0.1"
#define SAMPLES_PORT 1222

// Game screen dimensions.
#define FACTOR 60
#define WIDTH FACTOR * 16
#define HEIGHT FACTOR * 9

#define PLAYER_WIDTH 15 
#define PLAYER_HEIGHT 100 
#define PLAYER_SPEED 8 
#define BALL_WIDTH 15 
#define BALL_INITIAL_SPEED 1.5f

template <typename Players>
inline void game_state_tick(Players& players, float& ball_x, float& ball_y, float& ball_vx, float& ball_vy) {
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

