#ifndef HELPER
#define HELPER

#include <raylib.h>

#include <climits>
#include <cstdlib>

#include "components.hpp"

// https://cplusplus.com/forum/beginner/81180/
// Returns a random float within min and max
static float randf(const float min, const float max) {
  float result = (rand() / static_cast<float>(RAND_MAX) * (max + 1)) + min;
  return result;
}

// RNG 0-100
static bool rng(const int chance) {
  if (chance >= 100) return true;
  int randomNumber = rand() % 100;
  if (randomNumber < chance) {
    return true;
  }
  return false;
}

// Choose spawn position outside of window
static Vector2 chooseSpawnPosition(
  const int windowWidth, const int windowHeight, const float offset
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
      (randomPosition.x < windowWidth / 2) ? -offset : windowWidth + offset,
      randomPosition.y};
  } else {
    outsidePosition = {
      randomPosition.x,
      (randomPosition.y < windowHeight / 2) ? -offset : windowHeight + offset,
    };
  }
  return outsidePosition;
}

#endif