
#include <raylib.h>

#include "config.h"
#include "logic.h"

int main() {
	GameState state{};
	InitWindow(WIDTH, HEIGHT, "Pong");

	SetTargetFPS(60);

	while (!WindowShouldClose()) {
		state.tick(GetFrameTime());
		state.draw();
	}

	CloseWindow();

	return EXIT_SUCCESS;
}
