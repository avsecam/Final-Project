#ifndef COMPONENTS
#define COMPONENTS

#include <raylib.h>

#include "helper.hpp"

struct CharacterComponent {
  Vector2 position;
  Vector2 velocity;

  float hitboxRadius;
  std::vector<Vector2> unigridPositions;
};

const float SPAWN_OFFSET(20.0f);
const float ENEMY_MELEE_VELOCITY_MIN(30.0f);
const float ENEMY_MELEE_VELOCITY_MAX(100.0f);
const float ENEMY_RANGE_VELOCITY_MIN(15.0f);
const float ENEMY_RANGE_VELOCITY_MAX(30.0f);
const float ENEMY_RANGE_SAFE_DISTANCE(300.0f);
const float ENEMY_SHOOT_INTERVAL_BASE(5.0f);
const float ENEMY_SHOOT_INTERVAL_RAND(4.0f);
const float BULLET_SPEED(300.0f);
const int SCORE_PER_KILL(10);
enum MobType { MELEE, RANGE, BULLET };
struct MobComponent {
  MobType type;
  Vector2 spawnPosition;
};

struct TimerComponent {
  float maxTime = randf(
    ENEMY_SHOOT_INTERVAL_BASE - ENEMY_SHOOT_INTERVAL_RAND,
    ENEMY_SHOOT_INTERVAL_BASE + ENEMY_SHOOT_INTERVAL_RAND
  );
  float timeLeft;
};

struct StraightMovementComponent {
  Vector2 direction;
};

struct ScoreOnKillComponent {  // Give score when entity dies
  int score = SCORE_PER_KILL;
};

struct PlayerComponent {
  float hp;
};

// Move towards a point
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

// Move towards a point but stay at some distance
static void moveTowardsWithSlowOnLimit(
  CharacterComponent& c, const Vector2 targetPosition, const float distance,
  const float timestep
) {
  float distanceToTargetPosition = Vector2Distance(targetPosition, c.position);

  Vector2 directionToTargetPosition =
    Vector2Normalize(Vector2Subtract(targetPosition, c.position));
  Vector2 directionScaledToVelocity =
    Vector2Multiply(directionToTargetPosition, c.velocity);

  if (distanceToTargetPosition > distance) {
    c.position =
      Vector2Add(c.position, Vector2Scale(directionScaledToVelocity, timestep));
  } else {
    c.position = Vector2Add(
      c.position, Vector2Scale(directionScaledToVelocity, timestep / 2)
    );
  }
}

// Move with direction
static void moveDirectional(
  CharacterComponent& c, const Vector2 direction, const float timestep
) {
  Vector2 normalizedDirection = Vector2Normalize(direction);
  Vector2 directionScaledToVelocity =
    Vector2Multiply(normalizedDirection, c.velocity);

  c.position =
    Vector2Add(c.position, Vector2Scale(directionScaledToVelocity, timestep));
}

static bool isWithinRectangle(const Vector2 position, const Rectangle limits) {
  return (
    position.x > limits.x || position.y > limits.y ||
    position.x < limits.width || position.y < limits.height
  );
}

static Vector2 clampToRectangle(
  const Vector2 position, const Rectangle limits
) {
  Vector2 newPosition;
  if (position.x < limits.x || position.x > limits.width) {
    newPosition.x = (position.x < limits.x) ? limits.x : limits.width;
  }
  if (position.y < limits.y || position.y > limits.height) {
    newPosition.y = (position.y < limits.y) ? limits.y : limits.height;
  }
  return newPosition;
}

static void separateCharacters(CharacterComponent& a, CharacterComponent& b) {
  Vector2 normalizedCollisionNormal =
    Vector2Normalize(Vector2Subtract(b.position, a.position));

  a.position =
    Vector2Subtract(a.position, Vector2Scale(normalizedCollisionNormal, 0.5f));
  b.position =
    Vector2Add(b.position, Vector2Scale(normalizedCollisionNormal, 0.5f));
}

static bool charactersAreColliding(
  const CharacterComponent& a, const CharacterComponent& b
) {
  float sumOfRadii(pow(a.hitboxRadius + b.hitboxRadius, 2));
  float distanceBetweenCenters(Vector2DistanceSqr(a.position, b.position));

  return (sumOfRadii >= distanceBetweenCenters);
}

#endif