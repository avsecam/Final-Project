#ifndef UNIGRID
#define UNIGRID

#include <raylib.h>
#include <raymath.h>

#include <cstdio>
#include <cmath>
#include <vector>

#include "entt.hpp"
#include "components.hpp"

static Vector2 convertToUnigridPosition(const Vector2 position, const float gridCellSize);

struct Cell {
  Vector2 topLeft;
  int size;
  std::vector<entt::entity> objects;

  Cell() {}

  Cell(const int _size, const Vector2 _topLeft) {
    size = _size;
    topLeft = _topLeft;
  }

  // Show grid position if x and y are greater than -1
  void draw(const int x = -1, const int y = -1) {
    DrawRectangleLines(topLeft.x, topLeft.y, size, size, RED);
    if (x >= 0 && y >= 0) {
      char buffer[10];
      sprintf(buffer, "%d,%d", x, y);
      DrawText(buffer, topLeft.x, topLeft.y, 12, BLACK);

      sprintf(buffer, "%d", objects.size());
      DrawText(
        buffer, topLeft.x + (size / 2), topLeft.y + (size / 2), 15, GREEN
      );
    }
  }
};

struct UniformGrid {
  std::vector<std::vector<Cell>> cells;  // [row][column], [y][x]
  int gridCellSize;

  UniformGrid(
    const int _windowHeight, const int _windowWidth, const float _gridCellSize
  ) {
    gridCellSize = _gridCellSize;

    for (size_t i = 0; i < _windowHeight; i += gridCellSize) {
      std::vector<Cell> row;
      for (size_t j = 0; j < _windowWidth; j += gridCellSize) {
        Vector2 cellTopRight = {static_cast<float>(j), static_cast<float>(i)};
        row.push_back(Cell(gridCellSize, cellTopRight));
      }
      cells.push_back(row);
    }
  }

  void refreshPosition(entt::registry& r, entt::entity e) {
		CharacterComponent* cc = r.try_get<CharacterComponent>(e);
		if (!cc) return;

    for (size_t j = 0; j < cc->unigridPositions.size(); j++) {
      int gridX = cc->unigridPositions[j].x;
      int gridY = cc->unigridPositions[j].y;
      // Only add if object is inside cells within screen borders
      bool isInsideValidCell = gridY < cells.size() && gridX < cells[0].size();
      if (isInsideValidCell) {
        cells[gridY][gridX].objects.push_back(e);
      }
    }
  }

  void draw() {
    for (size_t i = 0; i < cells.size(); i++) {
      for (size_t j = 0; j < cells[i].size(); j++) {
        cells[i][j].draw(j, i);
      }
    }
  }

  void clearCells() {
    for (size_t i = 0; i < cells.size(); i++) {
      for (size_t j = 0; j < cells[i].size(); j++) {
        cells[i][j].objects.clear();
      }
    }
  }
};

static void refreshUnigridPositions(CharacterComponent* character, const float gridCellSize) {
  // Get the min(bottom-left) and max(top-right) extents of the Circle (just
  // like an AABB)
  Vector2 min = {
    character->position.x - character->hitboxRadius, character->position.y + character->hitboxRadius};
  Vector2 max = {
    character->position.x + character->hitboxRadius, character->position.y - character->hitboxRadius};

  Vector2 minGridPosition = convertToUnigridPosition(min, gridCellSize);
  Vector2 maxGridPosition = convertToUnigridPosition(max, gridCellSize);

  character->unigridPositions.clear();

  if (Vector2Equals(minGridPosition, maxGridPosition)) {
    // If the circle is in only one cell:
    character->unigridPositions.push_back(minGridPosition);
  } else {
    // Occupies spaces that are in between min and max
    for (int i = minGridPosition.x; i <= maxGridPosition.x; i++) {
      for (int j = minGridPosition.y; j >= maxGridPosition.y; j--) {
        Vector2 newGridPosition = {
          static_cast<float>(i), static_cast<float>(j)};
        character->unigridPositions.push_back(newGridPosition);
      }
    }
  }
}

static Vector2 convertToUnigridPosition(const Vector2 position, const float gridCellSize) {
  Vector2 gridPosition = {
    floor(position.x / gridCellSize), floor(position.y / gridCellSize)};
  return gridPosition;
}

#endif