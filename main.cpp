#include <raylib.h>
#include <raymath.h>

#include "entities.hpp"
#include "entt.hpp"

const int WINDOW_WIDTH(1280);
const int WINDOW_HEIGHT(720);
const char* WINDOW_TITLE("⚔ Hack and Slash ⚔");

const int TARGET_FPS(60);
const float TIMESTEP(1.0f / TARGET_FPS);

int main() {
  srand(GetTime());

  entt::registry registry;

  float accumulator(0.0f);
  float deltaTime(0.0f);
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
	SetTargetFPS(TARGET_FPS);
  while (!WindowShouldClose()) {
    deltaTime = GetFrameTime();

    if (IsKeyPressed(KEY_SPACE)) {
      entt::entity e = registry.create();

      CharacterComponent& cc = registry.emplace<CharacterComponent>(e);
      EnemyComponent& ec = registry.emplace<EnemyComponent>(e);
      ec.type = MELEE;
      ec.spawnPosition = chooseSpawnPosition(WINDOW_WIDTH, WINDOW_HEIGHT);
      cc.hitboxRadius = 10.0f;
      cc.position = ec.spawnPosition;
      cc.velocity = chooseEnemyVelocity(ENEMY_VELOCITY_MIN, ENEMY_VELOCITY_MAX);
    }

    // Physics Process
    accumulator += deltaTime;
    while (accumulator >= TIMESTEP) {
      auto characters = registry.view<CharacterComponent>();
      for (auto e : characters) {
        CharacterComponent& cc = registry.get<CharacterComponent>(e);
        moveTowards(cc, {WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f}, TIMESTEP);
        // Destroy character if inside some zone in the middle of the screen
        float deadzoneExtents = 30.0f;
        if (
					cc.position.x > (WINDOW_WIDTH / 2) - deadzoneExtents
					&& cc.position.x < (WINDOW_WIDTH / 2) + deadzoneExtents
					&& cc.position.y > (WINDOW_HEIGHT / 2) - deadzoneExtents
					&& cc.position.y < (WINDOW_HEIGHT / 2) + deadzoneExtents
				) {
					registry.destroy(e);
        }
      }
      accumulator -= TIMESTEP;
    }

    BeginDrawing();
		ClearBackground(RAYWHITE);

    auto characters = registry.view<CharacterComponent>();
    for (auto e : characters) {
      CharacterComponent& cc = registry.get<CharacterComponent>(e);
      drawCharacter(cc);
    }

    EndDrawing();
  }
  CloseWindow();
  return 0;
}