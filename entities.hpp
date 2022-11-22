#ifndef ENTITIES
#define ENTITIES

#include <raylib.h>

#include "helper.hpp"

struct CharacterComponent {
  Vector2 position;
  Vector2 velocity;

  float hitboxRadius;
};

const float SPAWN_OFFSET(20.0f);
const float ENEMY_VELOCITY_MIN(30.0f);
const float ENEMY_VELOCITY_MAX(100.0f);
enum EnemyType { MELEE, RANGE };
struct EnemyComponent {
  EnemyType type;
  Vector2 spawnPosition;
};
// Choose spawn position outside of window
static Vector2 chooseSpawnPosition(
  const int windowWidth, const int windowHeight
) {
  // Select random Vector2 inside window
  Vector2 randomPosition = {
    static_cast<float>(rand() % windowWidth),
    static_cast<float>(rand() % windowHeight)};

  bool spawnOnLeftOrRight = rng(50);

  // Clamp it outside window
  Vector2 outsidePosition;

  if (spawnOnLeftOrRight) {
    outsidePosition = {
      (randomPosition.x < windowWidth / 2) ? -SPAWN_OFFSET
                                           : windowWidth + SPAWN_OFFSET,
      randomPosition.y};
  } else {
    outsidePosition = {
      randomPosition.x,
      (randomPosition.y < windowHeight / 2) ? -SPAWN_OFFSET
                                            : windowHeight + SPAWN_OFFSET,
    };
  }
  return outsidePosition;
}
static Vector2 chooseEnemyVelocity(const float min, const float max) {
  return {randf(min, max), randf(min, max)};
}

struct PlayerComponent {
  float hp;
};

static void moveTowards(
  CharacterComponent& c, const Vector2 targetPosition, const float timestep
) {
  Vector2 directionToTargetPosition =
    Vector2Normalize(Vector2Subtract(targetPosition, c.position));
  Vector2 directionScaledToVelocity =
    Vector2Multiply(directionToTargetPosition, c.velocity);

  c.position =
    Vector2Add(c.position, Vector2Scale(directionScaledToVelocity, timestep));
}

static void drawCharacter(CharacterComponent& c) {
  DrawCircleV(c.position, c.hitboxRadius, RED);
}

#endif