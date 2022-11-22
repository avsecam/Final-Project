#include <raylib.h>
#include <raymath.h>

#include <iostream>
#include <string>
#include <vector>

#include "entt.hpp"
#include "UIHandler.h"

const int WINDOW_WIDTH(1280);
const int WINDOW_HEIGHT(720);
const char* WINDOW_TITLE("⚔ Hack and Slash ⚔");

int main() {
	InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
	SetTargetFPS(60);
	
	MainMenu menu;

	menu.createUI(WINDOW_WIDTH, WINDOW_HEIGHT);


	while (!WindowShouldClose()) {
		// GAME
		menu.Update();

		ClearBackground(WHITE);
        BeginDrawing();
		menu.Draw();
		EndDrawing();
	}

	CloseWindow();

    return 0;
}

