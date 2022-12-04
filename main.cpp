#include <raylib.h>
#include <raymath.h>

#include <iostream>
#include <string>
#include <vector>

#include "components.hpp"
#include "entt.hpp"
#include "uiHandler.hpp"
#include "unigrid.hpp"

const int WINDOW_WIDTH(1280);
const int WINDOW_HEIGHT(720);
const char* WINDOW_TITLE("⚔ Hack and Slash ⚔");

const int TARGET_FPS(60);
const float TIMESTEP(1.0f / TARGET_FPS);

const float UNIGRID_CELL_SIZE(60.0f);

const float WAIT_TIME_BEFORE_FIRST_SPAWN(1.0f);
const int BASE_ENEMY_COUNT(5);
const int ADDITIONAL_ENEMY_COUNT(5
);  // How many more enemies to add after score threshold

const float PLAYER_MOVESPEED(180.0f);

static void spawnEnemies(entt::registry& registry, const int amount) {
  for (int i = 0; i < amount; i++) {
    entt::entity e = registry.create();

    CharacterComponent& cc = registry.emplace<CharacterComponent>(e);
    MobComponent& mc = registry.emplace<MobComponent>(e);
    ScoreOnKillComponent& sokc = registry.emplace<ScoreOnKillComponent>(e);
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
      tc.maxTime = randf(
        ENEMY_SHOOT_INTERVAL_BASE - ENEMY_SHOOT_INTERVAL_RAND,
        ENEMY_SHOOT_INTERVAL_BASE + ENEMY_SHOOT_INTERVAL_RAND
      );
      tc.timeLeft = tc.maxTime;
    }
  }
}

int main() {
  srand(GetTime());

  int score(0);
  int requiredEnemyCount(BASE_ENEMY_COUNT);

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

  UniformGrid unigrid =
    UniformGrid(WINDOW_HEIGHT, WINDOW_WIDTH, UNIGRID_CELL_SIZE);

  bool gameHasJustStarted(true);
  float startWaitTime(0.0f);

  float accumulator(0.0f);
  float deltaTime(0.0f);
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
  SetTargetFPS(TARGET_FPS);
  while (!WindowShouldClose()) {
    deltaTime = GetFrameTime();

    if (gameHasJustStarted) {
      // Wait a bit before spawning enemies
      if (startWaitTime < WAIT_TIME_BEFORE_FIRST_SPAWN) {
        startWaitTime += deltaTime;
      } else {
        gameHasJustStarted = false;
      }
    }

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

    // Enemy Spawning
    // Depends on score
    // Every 500 points, amount should increase by 10
    // Spawn when enemies are a quarter of enemyCount
    if (!gameHasJustStarted) {
      auto enemies = registry.view<MobComponent>();
      int currentEnemyCount = enemies.size();
      for (auto e : enemies) {
        MobComponent& mc = registry.get<MobComponent>(e);
        if (mc.type == BULLET) {
          currentEnemyCount--;
        }
      }
      if (currentEnemyCount <= floor(requiredEnemyCount / 4.0f)) {
        spawnEnemies(registry, requiredEnemyCount + ADDITIONAL_ENEMY_COUNT);
      }
    }

    // Physics Process
    unigrid.clearCells();
    accumulator += deltaTime;
    while (accumulator >= TIMESTEP) {
      CharacterComponent& playerCc =
        registry.get<CharacterComponent>(playerEntity);

      // Move player character
      playerCc.position = Vector2Add(
        playerCc.position,
        Vector2Scale(playerMoveDirection, PLAYER_MOVESPEED * TIMESTEP)
      );

      auto characters = registry.view<CharacterComponent>();
      for (auto e : characters) {
        CharacterComponent& cc = registry.get<CharacterComponent>(e);

        TimerComponent* tc = registry.try_get<TimerComponent>(e);
        StraightMovementComponent* smc =
          registry.try_get<StraightMovementComponent>(e);
        MobComponent* mc = registry.try_get<MobComponent>(e);

        // Check if range enemy should shoot
        if (tc) {
          tc->timeLeft -= TIMESTEP;
          if (tc->timeLeft <= 0.0f) {
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
            smc.direction =
              Vector2Normalize(Vector2Subtract(playerCc.position, cc.position));

            tc->timeLeft = tc->maxTime;
          }
        }

        if (smc) {
          moveDirectional(cc, smc->direction, TIMESTEP);
          // Destroy SMC if it's not visible anymore
          if (!isWithinRectangle(
                cc.position, {0.0f, 0.0f, WINDOW_WIDTH, WINDOW_HEIGHT}
              )) {
            registry.destroy(e);
          }
        }

        // Move mobs
        if (mc) {
          switch (mc->type) {
            case MELEE:
              moveTowards(cc, playerCc.position, TIMESTEP);
              break;
            case RANGE:
              moveTowardsWithSlowOnLimit(
                cc, playerCc.position, ENEMY_RANGE_SAFE_DISTANCE, TIMESTEP
              );
              break;
            default:
              break;
          }

          // Check collision with other mobs
          if (mc->type != BULLET) {
            for (auto otherMob : characters) {
							PlayerComponent* otherMobPc = registry.try_get<PlayerComponent>(otherMob);
							MobComponent* otherMobMc = registry.try_get<MobComponent>(otherMob);
							// Don't collide with player and bullets
              if (
								otherMobPc
								|| otherMobMc->type == BULLET
							) {
                continue;
              }
              CharacterComponent& otherMobCc =
                registry.get<CharacterComponent>(otherMob);
              if (charactersAreColliding(cc, otherMobCc)) {
                separateCharacters(cc, otherMobCc);
              }
            }
          }

          // Destroy character if it collides with player
          if (charactersAreColliding(playerCc, cc)) {
            ScoreOnKillComponent* sokc =
              registry.try_get<ScoreOnKillComponent>(e);
            if (sokc) {
              score += sokc->score;
              printf("%d\n", score);
            }
            registry.destroy(e);
          }
        }

        refreshUnigridPositions(&cc, UNIGRID_CELL_SIZE);
        unigrid.refreshPosition(&cc);
      }
      accumulator -= TIMESTEP;
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Uniform Grid
    unigrid.draw();

    // Entities
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
