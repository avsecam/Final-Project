#ifndef UI_HANDLER
#define UI_HANDLER

#include <raylib.h>
#include <raymath.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

const int BUTTON_WIDTH_1(260);
const int BUTTON_HEIGHT_1(60);
const int FONT_SIZE_1(20);
const int FONT_SIZE_2(30);

int currentMenu = 0;

// Base UI Component Struct that all other UI Components will be based off of
struct UIComponent {
  Rectangle bounds;

  bool isHovered = false;

  virtual void Draw() = 0;

  virtual bool HandleHover(Vector2 mousePosition) = 0;

  virtual bool HandleClick(Vector2 clickPosition) = 0;
};

// A Container for other UIComponents 
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

// Base Button Struct
struct Button : public UIComponent {
  std::string text;

  void(*buttonAction)();

  void Draw() override {
    if (isHovered) {
      DrawRectangleRec(bounds, RED);
    } else {
      DrawRectangleRec(bounds, GRAY);
    }
    // Centering the Text to the Button
    Vector2 textDimensions = MeasureTextEx(GetFontDefault(), text.c_str(), FONT_SIZE_1, 1);
    int textX = (bounds.x + (bounds.width / 2.1)) - (textDimensions.x / 2);
    int textY = (bounds.y + (bounds.height / 2)) - (textDimensions.y / 2);
    DrawText(text.c_str(), textX, textY, FONT_SIZE_1, WHITE);
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
      buttonAction();
      return true;
    }
    return false;
  }
};

// Base Label Struct
struct Label : public UIComponent {
  std::string text;
  int fontSize;
  Color textColor;
  bool centerAlign = true, leftAlign = false, rightAlign = false;

  void Draw() override {
    if (centerAlign) {
      Vector2 textDimensions = MeasureTextEx(GetFontDefault(), text.c_str(), fontSize, 1);
      int textX = bounds.x - (textDimensions.x / 2);
      int textY = (bounds.y + (bounds.height / 2)) - (textDimensions.y / 2);
      DrawText(text.c_str(), textX, textY, fontSize, textColor);
    }
    else if (leftAlign) {
      Vector2 textDimensions = MeasureTextEx(GetFontDefault(), text.c_str(), fontSize, 1);
      int textX = bounds.x;
      int textY = (bounds.y + (bounds.height / 2)) - (textDimensions.y / 2);
      DrawText(text.c_str(), textX, textY, fontSize, textColor);
    }
    else if (rightAlign) {
      Vector2 textDimensions = MeasureTextEx(GetFontDefault(), text.c_str(), fontSize, 1);
      int textX = bounds.x - textDimensions.x;
      int textY = (bounds.y + (bounds.height / 2)) - (textDimensions.y / 2);
      DrawText(text.c_str(), textX, textY, fontSize, textColor);
    }
  }

  void setCenterAlign() {
    leftAlign = false;
    rightAlign = false;
    centerAlign = true;
  }

  void setLeftAlign() {
    rightAlign = false;
    centerAlign = false;
    leftAlign = true;
  }

  void setRightAlign() {
    leftAlign = false;
    centerAlign = false;
    rightAlign = true;
  }

  bool HandleHover(Vector2 mousePosition) override {
    return false;
  }

  bool HandleClick(Vector2 clickPosition) override {
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

// --------------------------------------------------
//                     MENU STUFF
// --------------------------------------------------

void startGame() { std::cout << "I AM STARTING GAME" << std::endl; };
void goToMainMenu() { currentMenu = 0; };
void goToScoreScreen() { currentMenu = 1; };

struct Menu {
  UILibrary uiLibrary;

  virtual void createUI(float windowWidth, float windowHeight) = 0;

  void Update() { uiLibrary.Update(); }

  void Draw() { uiLibrary.Draw(); }
};


struct MainMenu : public Menu {
  Button startGameButton;
  Button checkHighScoresButton;

  void createUI(float windowWidth, float windowHeight) override {
    uiLibrary.rootContainer.bounds = {0, 0, windowWidth, windowHeight};
  
    startGameButton.text = "START GAME";
    startGameButton.bounds = {
      windowWidth / 2 - BUTTON_WIDTH_1 / 2,
      windowHeight / 2 - BUTTON_HEIGHT_1 / 2, BUTTON_WIDTH_1, BUTTON_HEIGHT_1};
    startGameButton.buttonAction = startGame;

    uiLibrary.rootContainer.AddChild(&startGameButton);

    checkHighScoresButton.text = "CHECK HIGHSCORES";
    checkHighScoresButton.bounds = {
      windowWidth / 2 - BUTTON_WIDTH_1 / 2,
      windowHeight / 2 + BUTTON_HEIGHT_1, BUTTON_WIDTH_1, BUTTON_HEIGHT_1};
    checkHighScoresButton.buttonAction = goToScoreScreen;

    uiLibrary.rootContainer.AddChild(&checkHighScoresButton);
  }
};


struct ScoreScreen : public Menu {
  Label highScoreLabel;
  Label* scoreLabel; 
  Label* nameLabel; 
  Label* separator;
  Button returnToMainMenuButton;
  std::fstream highScoreFile;

  void createUI(float windowWidth, float windowHeight) override {
    uiLibrary.rootContainer.bounds = {0, 0, windowWidth, windowHeight};

    highScoreLabel.text = "HIGH SCORES";
    highScoreLabel.bounds = {
      windowWidth / 2, FONT_SIZE_2 * 2, 
      0, FONT_SIZE_2};
    highScoreLabel.fontSize = FONT_SIZE_2;
    highScoreLabel.setCenterAlign();
    highScoreLabel.textColor = BLACK;

    uiLibrary.rootContainer.AddChild(&highScoreLabel);


    highScoreFile.open("high_scores.txt");
    std::string line;
    float scoreNumber = 1;
    while(getline(highScoreFile, line)){
      std::cout << "ADDING SCORE" << std::endl;
      int end = line.find(" ");
      std::string score = line.substr(0, end - 0);
      std::string name = line.substr(end, line.length());

      scoreLabel = new Label;
      scoreLabel->text = score;
      scoreLabel->bounds = {
        windowWidth / 2 - 10, 
        (FONT_SIZE_2 * 3) + (FONT_SIZE_1 * scoreNumber),
        0, FONT_SIZE_1};
      scoreLabel->fontSize = FONT_SIZE_1;
      scoreLabel->setRightAlign();
      scoreLabel->textColor = BLACK;

      nameLabel = new Label;
      nameLabel->text = name;
      nameLabel->bounds = {
        windowWidth / 2 + 10, 
        (FONT_SIZE_2 * 3) + (FONT_SIZE_1 * scoreNumber),
        0, FONT_SIZE_1};
      nameLabel->fontSize = FONT_SIZE_1;
      nameLabel->setLeftAlign();
      nameLabel->textColor = BLACK;

      std::cout << &scoreLabel << std::endl;
      uiLibrary.rootContainer.AddChild(scoreLabel);
      uiLibrary.rootContainer.AddChild(nameLabel);

      scoreNumber += 1;
    }
    highScoreFile.close();

    returnToMainMenuButton.text = "MAIN MENU";
    returnToMainMenuButton.bounds = {
      windowWidth / 2 - BUTTON_WIDTH_1 / 2,
      windowHeight - BUTTON_HEIGHT_1 * 2, BUTTON_WIDTH_1, BUTTON_HEIGHT_1};
    returnToMainMenuButton.buttonAction = goToMainMenu;

    uiLibrary.rootContainer.AddChild(&returnToMainMenuButton);
  }
};


struct MenuHandler {
  std::vector<Menu*> menuList;
  MainMenu mainMenu;
  ScoreScreen scoreScreen;

  void initialize(float windowWidth, float windowHeight) {
    mainMenu.createUI(windowWidth, windowHeight);
    scoreScreen.createUI(windowWidth, windowHeight);

    menuList.push_back(&mainMenu);
    menuList.push_back(&scoreScreen);
  }

  void Update() { menuList[currentMenu]->Update(); }

  void Draw() { menuList[currentMenu]->Draw(); }
};


#endif