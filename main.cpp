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
const int SWORD_REACH(80);
const int SWORD_SWING_INTERVAL(1.0f);
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

static bool checkWeaponCollision(
  const meleeWeaponComponent& a, const CharacterComponent& b
) {
  float sumOfRadii(pow(a.hitboxRadius + b.hitboxRadius, 2));
  float distanceBetweenCenters(Vector2DistanceSqr(a.position, b.position));

  return (sumOfRadii >= distanceBetweenCenters);
}


int main() {
  srand(GetTime());
  bool canSwing = false;
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

  entt::entity weaponEntity;
  weaponEntity = registry.create();
  meleeWeaponComponent& wc = registry.emplace<meleeWeaponComponent>(weaponEntity);
  TimerComponent& weaponTc = registry.emplace<TimerComponent>(weaponEntity);
  wc.hitboxRadius = 50.0f;
  weaponTc.maxTime = SWORD_SWING_INTERVAL;
  weaponTc.timeLeft = weaponTc.maxTime;
  
  
  float accumulator(0.0f);
  float deltaTime(0.0f);
  float lastSwordSwingTime(0.0f);
  float aimAngle;
  Vector2 attackHitboxPosition;

  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
  SetTargetFPS(TARGET_FPS);
  while (!WindowShouldClose()) {
    deltaTime = GetFrameTime();
    Texture playerTexture = LoadTexture("Assets/rsword.png");

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

    // Attack
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      if (canSwing) {
        std::cout << "SUCCESSFUL SWING! " << weaponTc.timeLeft << std::endl;
        auto characters = registry.view<CharacterComponent>();
        for (auto e: characters) {
          CharacterComponent& cc = registry.get<CharacterComponent>(e);

          MobComponent* mc = registry.try_get<MobComponent>(e);
          if (mc) {
            if (checkWeaponCollision(wc, cc)) {
              registry.destroy(e);
            }
          }
          canSwing = false;
          weaponTc.timeLeft = weaponTc.maxTime;
        }
      }
      else{
        std::cout<<"CANNOT SWING YET, TIME LEFT: " << weaponTc.timeLeft << std::endl; 
      }
       
    }
  


    // Physics Process
    accumulator += deltaTime;
    while (accumulator >= TIMESTEP) {
      CharacterComponent& playerCc =
        registry.get<CharacterComponent>(playerEntity);

      // Move player character
      playerCc.position = Vector2Add(
        playerCc.position,
        Vector2Scale(playerMoveDirection, PLAYER_MOVESPEED * TIMESTEP)
      );

      //Weapon Hitbox Tracking
      wc.position = Vector2Add(playerCc.position, Vector2Scale(Vector2Normalize(Vector2Subtract(GetMousePosition(), playerCc.position)), SWORD_REACH));
      if (weaponTc.timeLeft <= 0.0f){
        canSwing = true;
      }
      else{
        weaponTc.timeLeft -= TIMESTEP;
      }
      
      //std::cout << wc.position.x << " | " << wc.position.y << std::endl;

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
              moveTowardsWithLimit(
                cc, playerCc.position, ENEMY_RANGE_SAFE_DISTANCE, TIMESTEP
              );
              break;
            default:
              break;
          }

          // Prevent enemies from going offscreen

          // Destroy character if it collides with player
          if (checkCharacterCollision(playerCc, cc)) {
            registry.destroy(e);
          }
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
        Vector2 texOffset;
        texOffset.x = 100;
        texOffset.y = 100;
        DrawCircleV(cc.position, cc.hitboxRadius, BLUE);
        DrawTextureEx(playerTexture, Vector2Subtract(cc.position, texOffset), 1.0, 2.0, WHITE);
      }
  
    }
    auto weap = registry.view<meleeWeaponComponent>();
    for (auto e : weap) {
      meleeWeaponComponent* wc = registry.try_get<meleeWeaponComponent>(e);
      //DrawCircleV(wc->position, wc->hitboxRadius, GREEN);
    }
    

    EndDrawing();
  }
  CloseWindow();
  return 0;
}
