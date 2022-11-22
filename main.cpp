#include <raylib.h>
#include <raymath.h>

#include "components.hpp"
#include "entt.hpp"

const int WINDOW_WIDTH(1280);
const int WINDOW_HEIGHT(720);
const char* WINDOW_TITLE("⚔ Hack and Slash ⚔");

const int TARGET_FPS(60);
const float TIMESTEP(1.0f / TARGET_FPS);

const float PLAYER_MOVESPEED(180.0f);

static bool checkCharacterCollision(
  const CharacterComponent& a, const CharacterComponent& b
) {
  float sumOfRadii(pow(a.hitboxRadius + b.hitboxRadius, 2));
  float distanceBetweenCenters(Vector2DistanceSqr(a.position, b.position));

  return (sumOfRadii >= distanceBetweenCenters);
}

int main() {
  srand(GetTime());

  entt::registry registry;

  // Create player
  entt::entity playerEntity;
  if (registry.view<PlayerComponent>().size() <= 0) {
    playerEntity = registry.create();
    CharacterComponent& cc = registry.emplace<CharacterComponent>(playerEntity);
    PlayerComponent& pc = registry.emplace<PlayerComponent>(playerEntity);
    cc.hitboxRadius = 25.0f;
    cc.position = {WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f};
    pc.hp = 10;
  }

  float accumulator(0.0f);
  float deltaTime(0.0f);
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
  SetTargetFPS(TARGET_FPS);
  while (!WindowShouldClose()) {
    deltaTime = GetFrameTime();

    Vector2 playerMoveDirection = Vector2Zero();
    if (IsKeyDown(KEY_W)) {
      playerMoveDirection.y -= 1.0f;
    }
    if (IsKeyDown(KEY_A)) {
      playerMoveDirection.x -= 1.0f;
    }
    if (IsKeyDown(KEY_S)) {
      playerMoveDirection.y += 1.0f;
    }
    if (IsKeyDown(KEY_D)) {
      playerMoveDirection.x += 1.0f;
    }
    playerMoveDirection = Vector2Normalize(playerMoveDirection);

    if (IsKeyPressed(KEY_SPACE)) {
      entt::entity e = registry.create();

      CharacterComponent& cc = registry.emplace<CharacterComponent>(e);
      MobComponent& mc = registry.emplace<MobComponent>(e);
      mc.type = (rng(50)) ? MELEE : RANGE;
      mc.spawnPosition =
        chooseSpawnPosition(WINDOW_WIDTH, WINDOW_HEIGHT, SPAWN_OFFSET);
      cc.hitboxRadius = 20.0f;
      cc.position = mc.spawnPosition;
      if (mc.type == MELEE) {
        cc.velocity = {
          randf(ENEMY_MELEE_VELOCITY_MIN, ENEMY_MELEE_VELOCITY_MAX),
          randf(ENEMY_MELEE_VELOCITY_MIN, ENEMY_MELEE_VELOCITY_MAX)};
      } else if (mc.type == RANGE) {
        cc.velocity = {
          randf(ENEMY_RANGE_VELOCITY_MIN, ENEMY_RANGE_VELOCITY_MAX),
          randf(ENEMY_RANGE_VELOCITY_MIN, ENEMY_RANGE_VELOCITY_MAX)};
        TimerComponent& tc = registry.emplace<TimerComponent>(e);
        tc.maxTime = ENEMY_SHOOT_INTERVAL;
        tc.timeLeft = tc.maxTime;
      }
    }

    // Physics Process
    accumulator += deltaTime;
    while (accumulator >= TIMESTEP) {
      auto characters = registry.view<CharacterComponent>();
      for (auto e : characters) {
        CharacterComponent& cc = registry.get<CharacterComponent>(e);

        // Check if range enemy should shoot
        if (registry.try_get<TimerComponent>(e)) {
          TimerComponent& tc = registry.get<TimerComponent>(e);
          tc.timeLeft -= TIMESTEP;
          if (tc.timeLeft <= 0.0f) {
            // Create and shoot bullet
            entt::entity e = registry.create();
            CharacterComponent& bulletCc =
              registry.emplace<CharacterComponent>(e);
            MobComponent& mc = registry.emplace<MobComponent>(e);
            StraightMovementComponent& smc =
              registry.emplace<StraightMovementComponent>(e);
            mc.type = BULLET;
            mc.spawnPosition = cc.position;
            bulletCc.hitboxRadius = 5.0f;
            bulletCc.position = cc.position;
            bulletCc.velocity = {BULLET_SPEED, BULLET_SPEED};

            CharacterComponent& playerCc =
              registry.get<CharacterComponent>(playerEntity);
            smc.direction =
              Vector2Normalize(Vector2Subtract(playerCc.position, cc.position));

            tc.timeLeft = tc.maxTime;
          }
        }

        if (!registry.try_get<PlayerComponent>(e)) {
          StraightMovementComponent* smc =
            registry.try_get<StraightMovementComponent>(e);
          CharacterComponent& playerCc =
            registry.get<CharacterComponent>(playerEntity);

          if (smc) {
            moveDirectional(cc, smc->direction, TIMESTEP);
            // Destroy SMC if it's not visible anymore
            if (cc.position.x < 0.0f
						|| cc.position.y < 0.0f
						|| cc.position.x > WINDOW_WIDTH
						|| cc.position.y > WINDOW_HEIGHT ) {
              registry.destroy(e);
            }
          } else {
            moveTowards(cc, playerCc.position, TIMESTEP);
          }
          // Destroy character if it collides with player
          if (checkCharacterCollision(playerCc, cc)) {
            registry.destroy(e);
          }
        } else {
          // Move player character
          CharacterComponent& playerCc = registry.get<CharacterComponent>(e);
          playerCc.position = Vector2Add(
            playerCc.position,
            Vector2Scale(playerMoveDirection, PLAYER_MOVESPEED * TIMESTEP)
          );
        }
      }
      accumulator -= TIMESTEP;
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);

    auto characters = registry.view<CharacterComponent>();
    for (auto e : characters) {
      CharacterComponent& cc = registry.get<CharacterComponent>(e);
      Color color;
      MobComponent* mc = registry.try_get<MobComponent>(e);
      PlayerComponent* pc = registry.try_get<PlayerComponent>(e);
      if (mc) {
        switch (mc->type) {
          case MELEE:
            color = RED;
            break;
          case RANGE:
            color = YELLOW;
            break;
          case BULLET:
            color = GREEN;
            break;
          default:
            color = BLACK;
        }
        DrawCircleV(cc.position, cc.hitboxRadius, color);
      }
      if (pc) {
        DrawCircleV(cc.position, cc.hitboxRadius, BLUE);
      }
    }

    EndDrawing();
  }
  CloseWindow();
  return 0;
}