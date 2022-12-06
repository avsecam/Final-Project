#include <raylib.h>
#include <raymath.h>
#include <string.h>

#include <iostream>
#include <string>
#include <vector>

#include "components.hpp"
#include "entt.hpp"
#include "uiHandler.hpp"
#include "unigrid.hpp"

const int WINDOW_WIDTH(1280);
const int WINDOW_HEIGHT(720);
const int SWORD_REACH(40);
const float SWORD_SWING_INTERVAL(0.5f);
const float ATTACK_ANIMATION_LENGTH(0.15f);
const int PLAYER_HEALTH(10);
const char* WINDOW_TITLE("⚔ HAKENSLASH ⚔");

const int TARGET_FPS(60);
const float TIMESTEP(1.0f / TARGET_FPS);

const KeyboardKey PAUSE_KEY(KEY_TAB);

const float UNIGRID_CELL_SIZE(60.0f);

const float WAIT_TIME_BEFORE_FIRST_SPAWN(1.0f);
const int BASE_ENEMY_COUNT(5);
const int ADDITIONAL_ENEMY_COUNT(1
);  // How many more enemies to add after score threshold
const int ENEMY_SPEEDUP_SPAWN_INTERVAL(2); // Speedup enemies after N times of spawning
const float ENEMY_SPEEDUP_ADDER(10.0f);

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

static void spawnEnemies(entt::registry& registry, const int amount, const int speedLevel) {
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
		cc.velocity = Vector2AddValue(cc.velocity, speedLevel * ENEMY_SPEEDUP_ADDER);
  }
}

int main() {
  srand(GetTime());

  State state;

  // MENUS
  MenuHandler menuHandler;
  menuHandler.initialize(WINDOW_WIDTH, WINDOW_HEIGHT);

  // VARIABLES FOR GAME
  int score(0);
  int requiredEnemyCount(BASE_ENEMY_COUNT);
	int timesEnemiesSpawned(0);
	int timesEnemiesSpedUp(0);

  bool isAttacking;



  entt::registry registry;

  // Create player
  entt::entity playerEntity;
  if (registry.view<PlayerComponent>().size() <= 0) {
    playerEntity = registry.create();
    CharacterComponent& cc = registry.emplace<CharacterComponent>(playerEntity);
    PlayerComponent& pc = registry.emplace<PlayerComponent>(playerEntity);
    cc.hitboxRadius = 25.0f;
    cc.position = {WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f};
    pc.hp = PLAYER_HEALTH;
  }
  CharacterComponent& playerCc = registry.get<CharacterComponent>(playerEntity);

  // WEAPON
  entt::entity weaponEntity;
  weaponEntity = registry.create();
  meleeWeaponComponent& wc =
    registry.emplace<meleeWeaponComponent>(weaponEntity);
  TimerComponent& weaponTc = registry.emplace<TimerComponent>(weaponEntity);
  wc.hitboxRadius = 60.0f;
  weaponTc.maxTime = SWORD_SWING_INTERVAL;
  weaponTc.timeLeft = weaponTc.maxTime;

  entt::entity weaponAnimationEntity;
  weaponAnimationEntity = registry.create();
  TimerComponent& animTimerTc =
    registry.emplace<TimerComponent>(weaponAnimationEntity);
  animTimerTc.maxTime = ATTACK_ANIMATION_LENGTH;
  animTimerTc.timeLeft = animTimerTc.maxTime;

  bool canSwing = false;

  UniformGrid unigrid =
    UniformGrid(WINDOW_HEIGHT, WINDOW_WIDTH, UNIGRID_CELL_SIZE);

  bool gameHasJustStarted(true);
  float startWaitTime(0.0f);

  float accumulator(0.0f);
  float deltaTime(0.0f);
  float lastSwordSwingTime(0.0f);
  float aimAngle;

  std::vector<entt::entity> entitiesToDelete;

	InitAudioDevice();
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
  SetTargetFPS(TARGET_FPS);

  menuHandler.inGameGUI.hpBar.InitBar(PLAYER_HEALTH);
  health = PLAYER_HEALTH;

  // TEXTURES
  Texture playerTexture = LoadTexture("./assets/knight.png");
  Texture playerAttackingTexture = LoadTexture("./assets/knightAttack.png");
  Texture mainMenuBackground = LoadTexture("./assets/Hakenslash.png");

	tick = LoadSound("./assets/tick.wav");

  while (!WindowShouldClose()) {
    deltaTime = GetFrameTime();

    state = menuHandler.getState();

    if (state == InGame) {
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

      if (IsKeyPressed(PAUSE_KEY)) {
        menuHandler.setState(InPauseScreen);
      }

      // Enemy Spawning
      // Spawn when enemies are a quarter of requiredEnemyCount
      if (!gameHasJustStarted) {
        int currentEnemyCount = 0;
        for (auto e : registry.view<MobComponent>()) {
          MobComponent& mc = registry.get<MobComponent>(e);
          if (mc.type == MELEE || mc.type == RANGE) {
            currentEnemyCount++;
          }
        }
        if (currentEnemyCount <= ceil(requiredEnemyCount / 4.0f)) {
					timesEnemiesSpawned++;

					if (timesEnemiesSpawned % ENEMY_SPEEDUP_SPAWN_INTERVAL == 0) {
						timesEnemiesSpedUp++;
					}

          spawnEnemies(registry, currentEnemyCount + requiredEnemyCount, timesEnemiesSpedUp + 1);
          requiredEnemyCount += ADDITIONAL_ENEMY_COUNT;

        }
      }

      // Attack
      if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (canSwing) {
          isAttacking = true;
          // Attack collision
          for (auto e : registry.view<CharacterComponent>()) {
            CharacterComponent& cc = registry.get<CharacterComponent>(e);

            MobComponent* mc = registry.try_get<MobComponent>(e);
            if (mc) {
              if (checkWeaponCollision(wc, cc)) {
                ScoreOnKillComponent* sokc =
                  registry.try_get<ScoreOnKillComponent>(e);
                if (sokc) {
                  score += sokc->score;
                }
                if (mc->type == MELEE || mc->type == RANGE) {
                  registry.destroy(e);
                } else {
                  StraightMovementComponent* smc =
                    registry.try_get<StraightMovementComponent>(e);
                  if (smc) {
                    // Deflect bullets
                    mc->type = FRIENDLY_BULLET;
                    cc.velocity = Vector2Scale(
                      cc.velocity, FRIENDLY_BULLET_SPEED_MULTIPLIER
                    );
                    // Get the average angle between player rotation and smc
                    // direction
                    float playerRotation =
                      findRotationAngle(playerCc.position, GetMousePosition());
                    float bulletAngle =
                      atan2f(-smc->direction.y, -smc->direction.x);
                    float newBulletAngle = (playerRotation + bulletAngle) / 2;
                    smc->direction = {cos(newBulletAngle), sin(newBulletAngle)};
                  }
                }
              }
            }
            canSwing = false;
            weaponTc.timeLeft = weaponTc.maxTime;
          }
        }
      }

      // Physics Process
      accumulator += deltaTime;
      while (accumulator >= TIMESTEP) {
      	unigrid.clearCells();
        // Move player character
        playerCc.position = Vector2Add(
          playerCc.position,
          Vector2Scale(playerMoveDirection, PLAYER_MOVESPEED * TIMESTEP)
        );

        // Weapon Hitbox Tracking and Swing Cooldown
        wc.position = Vector2Add(
          playerCc.position, Vector2Scale(
                               Vector2Normalize(Vector2Subtract(
                                 GetMousePosition(), playerCc.position
                               )),
                               SWORD_REACH
                             )
        );
        if (weaponTc.timeLeft <= 0.0f) {
          canSwing = true;
        } else {
          weaponTc.timeLeft -= TIMESTEP;
        }

        // Weapon Animation
        if (isAttacking) {
          animTimerTc.timeLeft -= TIMESTEP;
          if (animTimerTc.timeLeft <= 0) {
            isAttacking = false;
            animTimerTc.timeLeft = animTimerTc.maxTime;
          }
        }

        for (auto e : registry.view<CharacterComponent>()) {
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
                Vector2Normalize(Vector2Subtract(playerCc.position, cc.position)
                );

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

            // Destroy character if it collides with player
            if (charactersAreColliding(playerCc, cc)) {
              PlayerComponent& pc = registry.get<PlayerComponent>(playerEntity);
              registry.destroy(e);
              pc.hp -= 1;
              health = pc.hp;
              std::cout << health << std::endl;
              // GAME OVER?
              if (pc.hp <= 0) {
                menuHandler.gameOverScreen.scoreLabel.text =
                  "SCORE: " + std::to_string(score);
                memset(
                  menuHandler.gameOverScreen.playerName.text, '\0',
                  sizeof(menuHandler.gameOverScreen.playerName.text)
                );
                menuHandler.gameOverScreen.playerName.letterCount = 0;
                newScore = score;
                menuHandler.setState(InGameOverScreen);
              }
            }
          }

          // Mob collisions
          // Check collision with other mobs
          for (size_t i = 0; i < unigrid.cells.size(); i++) {
            for (size_t j = 0; j < unigrid.cells[i].size(); j++) {
							if (unigrid.cells[i][j].objects.empty()) continue;
              for (size_t obj1 = 0; obj1 < unigrid.cells[i][j].objects.size();
                   obj1++) {
                for (size_t obj2 = 0; obj2 < unigrid.cells[i][j].objects.size();
                     obj2++) {
                  if (obj1 == obj2) continue;

                  entt::entity eA = unigrid.cells[i][j].objects[obj1];
                  entt::entity eB = unigrid.cells[i][j].objects[obj2];

                  if (!registry.valid(eA) || !registry.valid(eB)) continue;

                  // Don't collide with player
                  if (registry.try_get<PlayerComponent>(eA) || registry.try_get<PlayerComponent>(eB))
                    continue;

                  MobComponent* aMc = registry.try_get<MobComponent>(eA);
                  MobComponent* bMc = registry.try_get<MobComponent>(eB);

                  // Don't collide with bullets
                  if (aMc->type == BULLET || bMc->type == BULLET) continue;

                  CharacterComponent* aCc =
                    registry.try_get<CharacterComponent>(eA);
                  CharacterComponent* bCc =
                    registry.try_get<CharacterComponent>(eB);

                  if (charactersAreColliding(*aCc, *bCc)) {
                    ScoreOnKillComponent* aSokc =
                      registry.try_get<ScoreOnKillComponent>(eA);
                    // Collide with friendly bullets
                    if (bMc->type == FRIENDLY_BULLET) {
                      if (aSokc) {
                        score += aSokc->score;
                      }
                      registry.destroy(eA);
                    } else {
                      separateCharacters(*aCc, *bCc);
                    }
                  }
                }
              }
            }
          }

          refreshUnigridPositions(&cc, UNIGRID_CELL_SIZE);
          unigrid.refreshPosition(registry, e);
        }
        accumulator -= TIMESTEP;
      }
    }

    else {
      if (state == InMainMenu) {  // Reset the game
        PlayerComponent& pc = registry.get<PlayerComponent>(playerEntity);
        menuHandler.inGameGUI.hpBar.InitBar(PLAYER_HEALTH);
        health = PLAYER_HEALTH;
        pc.hp = PLAYER_HEALTH;
        score = 0;
        requiredEnemyCount = BASE_ENEMY_COUNT;
        playerCc.position = {WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2};
				timesEnemiesSpawned = 0;
				timesEnemiesSpedUp = 0;
        gameHasJustStarted = true;
        startWaitTime = 0.0f;
        for (auto mob : registry.view<MobComponent>()) {
          registry.destroy(mob);
        }
      } else if (state == InPauseScreen) {
        if (IsKeyPressed(PAUSE_KEY)) {
          menuHandler.setState(InGame);
        }
      }
    }

    menuHandler.Update();

    BeginDrawing();
    ClearBackground(BROWN);

    if (state == InGame || state == InPauseScreen) {
      // Uniform Grid
      // unigrid.draw();

      // Entities
      for (auto e : registry.view<CharacterComponent>()) {
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
            case FRIENDLY_BULLET:
              color = BLUE;
              break;
            default:
              color = BLACK;
          }
          DrawCircleV(cc.position, cc.hitboxRadius, color);
        }
        if (pc) {
          Rectangle playerRec;  // Texture coords
          Rectangle windowRec;  //
          playerRec.x = 0;
          playerRec.y = 0;
          playerRec.width = 200;
          playerRec.height = 106;
          windowRec.x = cc.position.x;
          windowRec.y = cc.position.y;
          windowRec.width = 134;
          windowRec.height = 106;

          
          if (isAttacking == false) {
            DrawTexturePro(
              playerTexture, playerRec, windowRec, {67 / 2, 50},
              findRotationAngle(cc.position, GetMousePosition()) * RAD2DEG,
              WHITE
            );
          } else {
            DrawTexturePro(
              playerAttackingTexture, playerRec, windowRec, {67 / 2, 50},
              findRotationAngle(cc.position, GetMousePosition()) * RAD2DEG,
              WHITE
            );
          }
        }
      }

      // Weapon Hitbox Visual
      //auto weap = registry.view<meleeWeaponComponent>();
      //for (auto e : weap) {
        //meleeWeaponComponent* wc = registry.try_get<meleeWeaponComponent>(e);
        //DrawCircleV(wc->position, wc->hitboxRadius, GREEN);
      //}


      // score
      DrawText(std::to_string(score).c_str(), 10, 10, 20, PURPLE);
      newScore = score;
    }

    menuHandler.menuList[InMainMenu]->loadBackgroundTexture(mainMenuBackground);

    menuHandler.Draw();

    EndDrawing();
  }
  
	UnloadTexture(playerTexture);
	UnloadTexture(playerAttackingTexture);
  menuHandler.menuList[InMainMenu]->unloadBackgroundTexture();

	CloseAudioDevice();
	
	CloseWindow();
  return 0;
}
