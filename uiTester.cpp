#include <raylib.h>
#include <raymath.h>

#include <iostream>
#include <string>
#include <vector>

#include "components.hpp"
#include "entt.hpp"
#include "UIHandler.hpp"


const int WINDOW_WIDTH(1280);
const int WINDOW_HEIGHT(720);
const char* WINDOW_TITLE("UI TESTER");

const int TARGET_FPS(60);

int main()
{
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
    SetTargetFPS(TARGET_FPS);
    MenuHandler menuHandler;

    menuHandler.initialize(WINDOW_WIDTH, WINDOW_HEIGHT);

    while (!WindowShouldClose()) 
    {
        menuHandler.Update();

        BeginDrawing();
        ClearBackground(RAYWHITE);
        menuHandler.Draw();
        EndDrawing();
    }
  CloseWindow();
  return 0;
}