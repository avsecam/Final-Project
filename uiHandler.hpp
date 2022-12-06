#ifndef UI_HANDLER
#define UI_HANDLER

#include <raylib.h>
#include <raymath.h>

#include <cctype>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "helper.hpp"

const int BUTTON_WIDTH_1(260);
const int BUTTON_HEIGHT_1(60);
const int FONT_SIZE_1(20);
const int FONT_SIZE_2(30);
const int FONT_SIZE_3(60);

State gameState = InMainMenu;

int num_of_scores = 0;
int max_score = 0;
int min_score = 0;
int newScore = 0;
int health = 0;

bool scoreUpdate = false;

std::string userName = "";

Sound tick;

// --------------------------------------------------
//                  UI COMPONENTS
// --------------------------------------------------

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
  bool transparent;
  Color containerColor;

  void AddChild(UIComponent* child) { children.push_back(child); }

  void ClearChildren() { children.clear(); }

  void Draw() override {
    if (!transparent) {
      DrawRectangle(bounds.x, bounds.y, bounds.width, bounds.height, containerColor);
    }
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

struct BackgroundImage: public UIComponent {
  Texture backgroundImageTexture;

  void Draw() override {
    DrawTexture(backgroundImageTexture, bounds.x, bounds.y, WHITE);
  }

  bool HandleHover(Vector2 mousePosition) override { return false; }

  bool HandleClick(Vector2 clickPosition) override { return false; }
};

// Base Button Struct
struct Button : public UIComponent {
  std::string text;
  bool active;

  void (*buttonAction)();

  void Draw() override {
    if (isHovered && active) {
      DrawRectangleRec(bounds, RED);
    } else if (active) {
      DrawRectangleRec(bounds, GRAY);
    } else {
      DrawRectangleRec(bounds, DARKGRAY);
    }
    // Centering the Text to the Button
    Vector2 textDimensions =
      MeasureTextEx(GetFontDefault(), text.c_str(), FONT_SIZE_1, 1);
    int textX = (bounds.x + (bounds.width / 2.1)) - (textDimensions.x / 2);
    int textY = (bounds.y + (bounds.height / 2)) - (textDimensions.y / 2);

    if (active) {
      DrawText(text.c_str(), textX, textY, FONT_SIZE_1, WHITE);
    } else {
      DrawText(text.c_str(), textX, textY, FONT_SIZE_1, LIGHTGRAY);
    }
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
      if (active) {
        buttonAction();
        PlaySoundMulti(tick);
        return true;
      } else {
        return false;
      }
    }
    return false;
  }
};

struct Bar : public UIComponent{
  float barFill;
  int maxValue;
  int currentValue;

  void Draw() override {
    DrawRectangle(bounds.x, bounds.y, bounds.width, bounds.height, GRAY);

    if (maxValue == currentValue) {
      barFill = bounds.width;
    }
    else {
      barFill = (bounds.width / maxValue) * currentValue;
    }

    DrawRectangle(bounds.x, bounds.y, barFill, bounds.height, GREEN);
  }

  void InitBar(int value) {
    maxValue = value;
    currentValue = maxValue;
  }

  void UpdateBar(int value) {
    if (value >= maxValue) {
      currentValue = maxValue;
    }
    else {
      currentValue = value;
    }
  }

  bool HandleHover(Vector2 mousePosition) override { return false; }

  bool HandleClick(Vector2 clickPosition) override { return false; }
};

// Base Label Struct
struct Label : public UIComponent {
  std::string text;
  int fontSize;
  Color textColor;
  bool centerAlign = true, leftAlign = false, rightAlign = false;

  void Draw() override {
    if (centerAlign) {
      Vector2 textDimensions =
        MeasureTextEx(GetFontDefault(), text.c_str(), fontSize, 1);
      int textX = bounds.x - (textDimensions.x / 2);
      int textY = (bounds.y + (bounds.height / 2)) - (textDimensions.y / 2);
      DrawText(text.c_str(), textX, textY, fontSize, textColor);
    } else if (leftAlign) {
      Vector2 textDimensions =
        MeasureTextEx(GetFontDefault(), text.c_str(), fontSize, 1);
      int textX = bounds.x;
      int textY = (bounds.y + (bounds.height / 2)) - (textDimensions.y / 2);
      DrawText(text.c_str(), textX, textY, fontSize, textColor);
    } else if (rightAlign) {
      Vector2 textDimensions =
        MeasureTextEx(GetFontDefault(), text.c_str(), fontSize, 1);
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

  bool HandleHover(Vector2 mousePosition) override { return false; }

  bool HandleClick(Vector2 clickPosition) override { return false; }
};

struct TextField : public UIComponent {
  char text[4];
  int letterCount;
  int fontSize;
  Color textColor;
  bool isMax;

  void Draw() override {
    if (letterCount == 0) {
      text[letterCount] = '_';
    }
    userName = text;
    DrawText(text, bounds.x, bounds.y, fontSize, textColor);
  }

  void AddLetter(char letter) {
    text[letterCount] = toupper(letter);
    if ((letterCount < 2) && (letterCount >= 0)) {
      text[letterCount + 1] = '_';
      text[letterCount + 2] = '\0';
    } else {
      text[letterCount + 1] = '\0';
    }

    if (letterCount >= 2) {
      isMax = true;
    }

    letterCount += 1;
  }

  void RemoveLetter() {
    letterCount--;
    if (letterCount < 0) letterCount = 0;
    text[letterCount] = '_';
    text[letterCount + 1] = '\0';
    isMax = false;
  }

  bool HandleHover(Vector2 mousePosition) override { return false; }

  bool HandleClick(Vector2 clickPosition) override { return false; }
};

struct UILibrary {
  UIContainer rootContainer;

  void Update() {
    rootContainer.HandleHover(GetMousePosition());

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
      rootContainer.HandleClick(GetMousePosition());
    }
  }

  void Draw() { rootContainer.Draw(); }
};

// --------------------------------------------------
//                     MENU STUFF
// --------------------------------------------------

void startGame() { gameState = InGame; };
void goToMainMenu() { gameState = InMainMenu; };
void goToScoreScreen() { gameState = InScoreScreen; };
void goToPauseScreen() { gameState = InPauseScreen; };
void goToGameOverScreen() { gameState = InGameOverScreen; };
void saveScore() {
  std::fstream highScoreFile;
  std::ofstream newHighScoreFile;
  int currentScore;
  bool addedNewScore = false;
  std::vector<std::string> scoreList;

  highScoreFile.open("high_scores.txt");
  std::string line;
  float scoreNumber = 1;

  std::string newScoreLine = std::to_string(newScore) + " " + userName;

  while (getline(highScoreFile, line) && scoreList.size() < 10) {
    int end = line.find(" ");
    std::string score = line.substr(0, end - 0);
    std::string name = line.substr(end, line.length());

    currentScore = stoi(score);

    if (!addedNewScore) {
      if (newScore <= currentScore) {
        scoreList.push_back(line);
      } else {
        scoreList.push_back(newScoreLine);
        if (scoreList.size() >= 10) break;
        scoreList.push_back(line);
        addedNewScore = true;
      }
    } else {
      scoreList.push_back(line);
    }
  }

  if (!addedNewScore && scoreList.size() < 10) {
    scoreList.push_back(newScoreLine);
  }

  highScoreFile.close();

  newHighScoreFile.open("high_scores.txt", std::ofstream::trunc);

  for (size_t i = 0; i < scoreList.size(); i++) {
    newHighScoreFile << scoreList[i] << std::endl;
  }

  newHighScoreFile.close();

  scoreUpdate = true;

  gameState = InScoreScreen;
};

struct Menu {
  UILibrary uiLibrary;

  virtual void createUI(float windowWidth, float windowHeight) = 0;

  virtual void Update() = 0;

  virtual void loadBackgroundTexture(Texture tex) = 0;

  virtual void unloadBackgroundTexture() = 0;

  void Draw() { uiLibrary.Draw(); }
};

struct MainMenu : public Menu {
  Button startGameButton;
  Button checkHighScoresButton;
  BackgroundImage startMenuBackground;

  void createUI(float windowWidth, float windowHeight) override {
    uiLibrary.rootContainer.bounds = {0, 0, windowWidth, windowHeight};
    uiLibrary.rootContainer.transparent = true;

    startMenuBackground.bounds = {0, 0, windowWidth, windowHeight};
    uiLibrary.rootContainer.AddChild(&startMenuBackground);

    startGameButton.text = "START GAME";
    startGameButton.bounds = {
      windowWidth / 2 - BUTTON_WIDTH_1 / 2,
      windowHeight / 2 - BUTTON_HEIGHT_1 / 2, BUTTON_WIDTH_1, BUTTON_HEIGHT_1};
    startGameButton.buttonAction = startGame;
    startGameButton.active = true;
    uiLibrary.rootContainer.AddChild(&startGameButton);

    checkHighScoresButton.text = "CHECK HIGHSCORES";
    checkHighScoresButton.bounds = {
      windowWidth / 2 - BUTTON_WIDTH_1 / 2, windowHeight / 2 + BUTTON_HEIGHT_1,
      BUTTON_WIDTH_1, BUTTON_HEIGHT_1};
    checkHighScoresButton.buttonAction = goToScoreScreen;
    checkHighScoresButton.active = true;
    uiLibrary.rootContainer.AddChild(&checkHighScoresButton);
  }

  void loadBackgroundTexture(Texture tex) override {
    startMenuBackground.backgroundImageTexture = tex;
  }

  void unloadBackgroundTexture() override {
    UnloadTexture(startMenuBackground.backgroundImageTexture);
  }

  void Update() override { uiLibrary.Update(); }
};

struct ScoreScreen : public Menu {
  Label highScoreLabel;
  Label* scoreLabel;
  Label* nameLabel;
  Button returnToMainMenuButton;
  std::fstream highScoreFile;
  int currentScore;

  void createUI(float windowWidth, float windowHeight) override {
    uiLibrary.rootContainer.ClearChildren();

    uiLibrary.rootContainer.bounds = {0, 0, windowWidth, windowHeight};
    uiLibrary.rootContainer.transparent = true;

    highScoreLabel.text = "HIGH SCORES";
    highScoreLabel.bounds = {windowWidth / 2, FONT_SIZE_3 * 2, 0, FONT_SIZE_3};
    highScoreLabel.fontSize = FONT_SIZE_3;
    highScoreLabel.setCenterAlign();
    highScoreLabel.textColor = BLACK;

    uiLibrary.rootContainer.AddChild(&highScoreLabel);

    highScoreFile.open("high_scores.txt");
    std::string line;
    float scoreNumber = 1;
    while (getline(highScoreFile, line)) {
      std::cout << line << std::endl;
      int end = line.find(" ");
      std::string score = line.substr(0, end - 0);
      std::string name = line.substr(end, line.length());

      currentScore = stoi(score);

      if (scoreNumber == 1) {
        max_score = currentScore;
      }

      scoreLabel = new Label;
      scoreLabel->text = score;
      scoreLabel->bounds = {
        windowWidth / 2 - 20, (FONT_SIZE_3 * 3) + (FONT_SIZE_2 * scoreNumber),
        0, FONT_SIZE_2};
      scoreLabel->fontSize = FONT_SIZE_2;
      scoreLabel->setRightAlign();
      scoreLabel->textColor = BLACK;

      nameLabel = new Label;
      nameLabel->text = name;
      nameLabel->bounds = {
        windowWidth / 2 + 10, (FONT_SIZE_3 * 3) + (FONT_SIZE_2 * scoreNumber),
        0, FONT_SIZE_2};
      nameLabel->fontSize = FONT_SIZE_2;
      nameLabel->setLeftAlign();
      nameLabel->textColor = BLACK;

      uiLibrary.rootContainer.AddChild(scoreLabel);
      uiLibrary.rootContainer.AddChild(nameLabel);

      num_of_scores += 1;
      scoreNumber += 1;
    }

    min_score = currentScore;
    highScoreFile.close();

    returnToMainMenuButton.text = "MAIN MENU";
    returnToMainMenuButton.bounds = {
      windowWidth / 2 - BUTTON_WIDTH_1 / 2, windowHeight - BUTTON_HEIGHT_1 * 2,
      BUTTON_WIDTH_1, BUTTON_HEIGHT_1};
    returnToMainMenuButton.buttonAction = goToMainMenu;
    returnToMainMenuButton.active = true;
    uiLibrary.rootContainer.AddChild(&returnToMainMenuButton);
  }

  void loadBackgroundTexture(Texture tex) override {}

  void unloadBackgroundTexture() override {}

  void Update() override { uiLibrary.Update(); }
};

struct PauseScreen : Menu {
  Label pauseScreenLabel;
  Button returnToMainMenuButton, returnToGameButton;
  void createUI(float windowWidth, float windowHeight) override {
    uiLibrary.rootContainer.bounds = {0, 0, windowWidth, windowHeight};
    uiLibrary.rootContainer.transparent = true;

    pauseScreenLabel.text = "PAUSE SCREEN";
    pauseScreenLabel.bounds = {
      windowWidth / 2, FONT_SIZE_3 * 2, 0, FONT_SIZE_3};
    pauseScreenLabel.fontSize = FONT_SIZE_3;
    pauseScreenLabel.setCenterAlign();
    pauseScreenLabel.textColor = BLACK;

    uiLibrary.rootContainer.AddChild(&pauseScreenLabel);

    returnToMainMenuButton.text = "MAIN MENU";
    returnToMainMenuButton.bounds = {
      (windowWidth / 2 - BUTTON_WIDTH_1 / 2) - (BUTTON_WIDTH_1 * float(0.75)),
      windowHeight / 2 - BUTTON_HEIGHT_1 / 2, BUTTON_WIDTH_1, BUTTON_HEIGHT_1};
    returnToMainMenuButton.buttonAction = goToMainMenu;
    returnToMainMenuButton.active = true;
    uiLibrary.rootContainer.AddChild(&returnToMainMenuButton);

    returnToGameButton.text = "BACK TO GAME";
    returnToGameButton.bounds = {
      (windowWidth / 2 - BUTTON_WIDTH_1 / 2) + (BUTTON_WIDTH_1 * float(0.75)),
      windowHeight / 2 - BUTTON_HEIGHT_1 / 2, BUTTON_WIDTH_1, BUTTON_HEIGHT_1};
    returnToGameButton.buttonAction = startGame;
    returnToGameButton.active = true;
    uiLibrary.rootContainer.AddChild(&returnToGameButton);
  }

  void loadBackgroundTexture(Texture tex) override {}

  void unloadBackgroundTexture() override {}

  void Update() override { uiLibrary.Update(); }
};

struct GameOverScreen : Menu {
  Label gameOverScreenLabel, scoreLabel, setNameLabel;
  Button returnToMainMenuButton, saveScoreButton;
  TextField playerName;
  BackgroundImage gameOverBackground;

  void createUI(float windowWidth, float windowHeight) override {
    uiLibrary.rootContainer.ClearChildren();

    uiLibrary.rootContainer.bounds = {0, 0, windowWidth, windowHeight};
    uiLibrary.rootContainer.transparent = true;

    gameOverBackground.bounds = {0, 0, windowWidth, windowHeight};
    uiLibrary.rootContainer.AddChild(&gameOverBackground);

    gameOverScreenLabel.text = "GAME OVER";
    gameOverScreenLabel.bounds = {
      windowWidth / 2, FONT_SIZE_3 * 2, 0, FONT_SIZE_3};
    gameOverScreenLabel.fontSize = FONT_SIZE_3;
    gameOverScreenLabel.setCenterAlign();
    gameOverScreenLabel.textColor = WHITE;

    uiLibrary.rootContainer.AddChild(&gameOverScreenLabel);

    scoreLabel.text = "SCORE: 0";
    scoreLabel.bounds = {
      windowWidth / 2, windowHeight / 2 - FONT_SIZE_3 * float(2.5), 0,
      FONT_SIZE_2};
    scoreLabel.fontSize = FONT_SIZE_2;
    scoreLabel.setCenterAlign();
    scoreLabel.textColor = WHITE;

    uiLibrary.rootContainer.AddChild(&scoreLabel);

    setNameLabel.text = "NAME:";
    setNameLabel.bounds = {
      windowWidth / 2 - 20, windowHeight / 2 - FONT_SIZE_3 * float(1.5), 0,
      FONT_SIZE_2};
    setNameLabel.fontSize = FONT_SIZE_2;
    setNameLabel.setRightAlign();
    setNameLabel.textColor = WHITE;

    uiLibrary.rootContainer.AddChild(&setNameLabel);

    playerName.bounds = {
      windowWidth / 2 + 20, windowHeight / 2 - FONT_SIZE_3 * float(1.5), 0,
      FONT_SIZE_2};
    playerName.fontSize = FONT_SIZE_2;
    playerName.textColor = WHITE;
    playerName.letterCount = 0;
    playerName.isMax = false;

    uiLibrary.rootContainer.AddChild(&playerName);

    saveScoreButton.text = "SAVE SCORE";
    saveScoreButton.bounds = {
      windowWidth / 2 - BUTTON_WIDTH_1 / 2,
      windowHeight / 2 - BUTTON_HEIGHT_1 / 2, BUTTON_WIDTH_1, BUTTON_HEIGHT_1};
    saveScoreButton.buttonAction = saveScore;
    saveScoreButton.active = false;
    uiLibrary.rootContainer.AddChild(&saveScoreButton);

    returnToMainMenuButton.text = "MAIN MENU";
    returnToMainMenuButton.bounds = {
      windowWidth / 2 - BUTTON_WIDTH_1 / 2, windowHeight / 2 + BUTTON_HEIGHT_1,
      BUTTON_WIDTH_1, BUTTON_HEIGHT_1};
    returnToMainMenuButton.buttonAction = goToMainMenu;
    returnToMainMenuButton.active = true;
    uiLibrary.rootContainer.AddChild(&returnToMainMenuButton);
  }

  void loadBackgroundTexture(Texture tex) override {
    gameOverBackground.backgroundImageTexture = tex;
  }

  void unloadBackgroundTexture() override {}

  void Update() {
    uiLibrary.Update();

    if (playerName.isMax) {
      saveScoreButton.active = true;
    } else {
      saveScoreButton.active = false;
    }

    int key = GetCharPressed();
    if ((key >= 32) && (key <= 125) && (playerName.letterCount < 3)) {
      playerName.AddLetter((char)key);
    }

    if (IsKeyPressed(KEY_BACKSPACE)) {
      playerName.RemoveLetter();
    }
  }
};


struct HPAndScoreGUI : Menu {
  Label healthLabel, scoreLabel, scoreOutput;
  Bar hpBar;
  
  void createUI(float windowWidth, float windowHeight) override {
    uiLibrary.rootContainer.bounds = {0, 0, windowWidth, 100};
    uiLibrary.rootContainer.transparent = false;
    uiLibrary.rootContainer.containerColor = BLACK;

    healthLabel.text = "HP:";
    healthLabel.bounds = {
      100, 25, 0, FONT_SIZE_3};
    healthLabel.fontSize = FONT_SIZE_3;
    healthLabel.setLeftAlign();
    healthLabel.textColor = WHITE;

    uiLibrary.rootContainer.AddChild(&healthLabel);

    hpBar.bounds = {
      200, 25, 500, FONT_SIZE_3};
    uiLibrary.rootContainer.AddChild(&hpBar);

    scoreLabel.text = "SCORE:";
    scoreLabel.bounds = {
      800, 25, 0, FONT_SIZE_3};
    scoreLabel.fontSize = FONT_SIZE_3;
    scoreLabel.setLeftAlign();
    scoreLabel.textColor = WHITE;
    uiLibrary.rootContainer.AddChild(&scoreLabel);

    scoreOutput.text = "0";
    scoreOutput.bounds = {
      1050, 25, 0, FONT_SIZE_3};
    scoreOutput.fontSize = FONT_SIZE_3;
    scoreOutput.setLeftAlign();
    scoreOutput.textColor = WHITE;
    uiLibrary.rootContainer.AddChild(&scoreOutput);
  }

  void loadBackgroundTexture(Texture tex) override {}

  void unloadBackgroundTexture() override {}

  void Update() override { 
    uiLibrary.Update(); 

    hpBar.UpdateBar(health);
    scoreOutput.text = std::to_string(newScore);
  }
};


struct MenuHandler {
  std::vector<Menu*> menuList;
  MainMenu mainMenu;
  ScoreScreen scoreScreen;
  PauseScreen pauseScreen;
  GameOverScreen gameOverScreen;
  HPAndScoreGUI inGameGUI;
  float menuWindowWidth, menuWindowHeight;

  void initialize(float windowWidth, float windowHeight) {
    menuWindowWidth = windowWidth;
    menuWindowHeight = windowHeight;

    mainMenu.createUI(windowWidth, windowHeight);
    scoreScreen.createUI(windowWidth, windowHeight);
    pauseScreen.createUI(windowWidth, windowHeight);
    gameOverScreen.createUI(windowWidth, windowHeight);
    inGameGUI.createUI(windowWidth, windowHeight);

    gameState = InMainMenu;

    menuList.push_back(&mainMenu);
    menuList.push_back(&scoreScreen);
    menuList.push_back(&pauseScreen);
    menuList.push_back(&gameOverScreen);
    menuList.push_back(&inGameGUI);
  }

  void Update() {
    //if (gameState == InGame) return;
    if (scoreUpdate) {
      scoreScreen.createUI(menuWindowWidth, menuWindowHeight);
      scoreUpdate = false;
    }

    menuList[gameState]->Update();
  }

  void Draw() {
    //if (gameState == InGame) return;
    menuList[gameState]->Draw();
  }

  void setState(State s) { gameState = s; }

  State getState() { return gameState; }
};

#endif