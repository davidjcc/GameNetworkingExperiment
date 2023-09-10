
#include <raylib.h>

#include "config.h"
#include "client_state.h"

int main() {
	Pong_Client_State state{};
	InitWindow(WIDTH, HEIGHT, "Pong");

	SetTargetFPS(60);

	while (!WindowShouldClose()) {
		state.tick(GetFrameTime());
		state.draw();
	}

	CloseWindow();

	return EXIT_SUCCESS;
}
