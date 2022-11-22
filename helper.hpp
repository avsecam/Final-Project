#ifndef HELPER
#define HELPER

#include <cstdlib>
#include <climits>

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

#endif