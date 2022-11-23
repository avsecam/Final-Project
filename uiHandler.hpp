#ifndef UI_HANDLER
#define UI_HANDLER

#include <raylib.h>
#include <raymath.h>

#include <iostream>
#include <string>
#include <vector>

const int BUTTON_WIDTH_1(200);
const int BUTTON_HEIGHT_1(40);
const int FONT_SIZE_1(20);

// Function Defs
struct UIComponent {
  Rectangle bounds;

  bool isHovered = false;

  virtual void Draw() = 0;

  virtual bool HandleHover(Vector2 mousePosition) = 0;

  virtual bool HandleClick(Vector2 clickPosition) = 0;
};

struct UIContainer : public UIComponent {
  std::vector<UIComponent*> children;

  void AddChild(UIComponent* child) { children.push_back(child); }

  void Draw() override {
    for (size_t i = 0; i < children.size(); i++) {
      children[i]->Draw();
    }
  }

  bool HandleHover(Vector2 mousePosition) override {
    for (size_t i = children.size(); i > 0; --i) {
      if (children[i - 1]->HandleHover(mousePosition)) {
        return true;
      }
    }

    return false;
  }

  bool HandleClick(Vector2 clickPosition) override {
    for (size_t i = children.size(); i > 0; --i) {
      if (children[i - 1]->HandleClick(clickPosition)) {
        return true;
      }
    }

    return false;
  }
};

struct Button : public UIComponent {
  std::string text;

  std::vector<UIComponent*> observers;

  void Draw() override {
    if (isHovered) {
      DrawRectangleRec(bounds, RED);
    } else {
      DrawRectangleRec(bounds, GRAY);
    }
    DrawText(text.c_str(), bounds.x, bounds.y, FONT_SIZE_1, BLACK);
  }

  bool HandleHover(Vector2 mousePosition) override {
    isHovered = false;
    if (CheckCollisionPointRec(mousePosition, bounds)) {
      isHovered = true;
      return true;
    }
    return false;
  }

  bool HandleClick(Vector2 clickPosition) override {
    if (CheckCollisionPointRec(clickPosition, bounds)) {
      std::cout << "I Am Pressed" << std::endl;
      return true;
    }

    return false;
  }
};

struct UILibrary {
  UIContainer rootContainer;

  void Update() {
    rootContainer.HandleHover(GetMousePosition());

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
      std::cout << "CLICK" << std::endl;
      rootContainer.HandleClick(GetMousePosition());
    }
  }

  void Draw() { rootContainer.Draw(); }
};

struct MainMenu {
  UILibrary uiLibrary;

  Button startGameButton;

  void createUI(float windowWidth, float windowHeight) {
    uiLibrary.rootContainer.bounds = {0, 0, windowWidth, windowHeight};

    startGameButton.text = "START GAME";
    startGameButton.bounds = {
      windowWidth / 2 - BUTTON_WIDTH_1 / 2,
      windowHeight / 2 - BUTTON_HEIGHT_1 / 2, BUTTON_WIDTH_1, BUTTON_HEIGHT_1};

    uiLibrary.rootContainer.AddChild(&startGameButton);
  }

  void Update() { uiLibrary.Update(); }

  void Draw() { uiLibrary.Draw(); }
};


#endif