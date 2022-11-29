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
const int FONT_SIZE_3(60);

int currentMenu = 0;

int num_of_scores = 0;
int score = 0;
int max_score = 0;
int min_score = 0;

// --------------------------------------------------
//                 MISC COMPONENTS
// --------------------------------------------------

struct ScoreCard {
  std::string name;
  int score;
};

std::vector<ScoreCard*> score_cards;

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
void goToPauseScreen() { currentMenu = 3; };
void goToGameOverScreen() { currentMenu = 4; };
void saveScore() { std::cout << "SAVING SCORE" << std::endl; };

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
  ScoreCard* scoreCard;
  Button returnToMainMenuButton;
  std::fstream highScoreFile;
  int currentScore;

  void createUI(float windowWidth, float windowHeight) override {
    uiLibrary.rootContainer.bounds = {0, 0, windowWidth, windowHeight};

    highScoreLabel.text = "HIGH SCORES";
    highScoreLabel.bounds = {
      windowWidth / 2, FONT_SIZE_3 * 2, 
      0, FONT_SIZE_3};
    highScoreLabel.fontSize = FONT_SIZE_3;
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

      currentScore = stoi(score);

      if(scoreNumber == 1){
        max_score = currentScore;
      }

      scoreCard = new ScoreCard;
      scoreCard->name = name;
      scoreCard->score = currentScore;

      score_cards.push_back(scoreCard);

      scoreLabel = new Label;
      scoreLabel->text = score;
      scoreLabel->bounds = {
        windowWidth / 2 - 20, 
        (FONT_SIZE_3 * 3) + (FONT_SIZE_2 * scoreNumber),
        0, FONT_SIZE_2};
      scoreLabel->fontSize = FONT_SIZE_2;
      scoreLabel->setRightAlign();
      scoreLabel->textColor = BLACK;

      nameLabel = new Label;
      nameLabel->text = name;
      nameLabel->bounds = {
        windowWidth / 2 + 10, 
        (FONT_SIZE_3 * 3) + (FONT_SIZE_2 * scoreNumber),
        0, FONT_SIZE_2};
      nameLabel->fontSize = FONT_SIZE_2;
      nameLabel->setLeftAlign();
      nameLabel->textColor = BLACK;

      std::cout << &scoreLabel << std::endl;
      uiLibrary.rootContainer.AddChild(scoreLabel);
      uiLibrary.rootContainer.AddChild(nameLabel);

      num_of_scores += 1;
      scoreNumber += 1;
    }
    
    min_score = currentScore;
    highScoreFile.close();

    returnToMainMenuButton.text = "MAIN MENU";
    returnToMainMenuButton.bounds = {
      windowWidth / 2 - BUTTON_WIDTH_1 / 2,
      windowHeight - BUTTON_HEIGHT_1 * 2, BUTTON_WIDTH_1, BUTTON_HEIGHT_1};
    returnToMainMenuButton.buttonAction = goToMainMenu;

    uiLibrary.rootContainer.AddChild(&returnToMainMenuButton);
  }
};


struct PauseScreen : Menu {
  Label pauseScreenLabel;
  Button returnToMainMenuButton, returnToGameButton;
  void createUI(float windowWidth, float windowHeight) override {
    uiLibrary.rootContainer.bounds = {0, 0, windowWidth, windowHeight};

    pauseScreenLabel.text = "PAUSE SCREEN";
    pauseScreenLabel.bounds = {
      windowWidth / 2, FONT_SIZE_3 * 2, 
      0, FONT_SIZE_3};
    pauseScreenLabel.fontSize = FONT_SIZE_3;
    pauseScreenLabel.setCenterAlign();
    pauseScreenLabel.textColor = BLACK;

    uiLibrary.rootContainer.AddChild(&pauseScreenLabel);

    returnToMainMenuButton.text = "MAIN MENU";
    returnToMainMenuButton.bounds = {
      ( windowWidth / 2 - BUTTON_WIDTH_1 / 2 ) - (BUTTON_WIDTH_1 * float(0.75)),
      windowHeight / 2 - BUTTON_HEIGHT_1 / 2, BUTTON_WIDTH_1, BUTTON_HEIGHT_1};
    returnToMainMenuButton.buttonAction = goToMainMenu;

    uiLibrary.rootContainer.AddChild(&returnToMainMenuButton);

    returnToGameButton.text = "BACK TO GAME";
    returnToGameButton.bounds = {
      ( windowWidth / 2 - BUTTON_WIDTH_1 / 2 ) + (BUTTON_WIDTH_1 * float(0.75)),
      windowHeight / 2 - BUTTON_HEIGHT_1 / 2, BUTTON_WIDTH_1, BUTTON_HEIGHT_1};
    returnToGameButton.buttonAction = startGame;

    uiLibrary.rootContainer.AddChild(&returnToGameButton);
  }
};


struct GameOverScreen : Menu {
  Label gameOverScreenLabel, scoreLabel;
  Button returnToMainMenuButton, saveScoreButton;
  void createUI(float windowWidth, float windowHeight) override {
    uiLibrary.rootContainer.bounds = {0, 0, windowWidth, windowHeight};

    gameOverScreenLabel.text = "GAME OVER";
    gameOverScreenLabel.bounds = {
      windowWidth / 2, FONT_SIZE_3 * 2, 
      0, FONT_SIZE_3};
    gameOverScreenLabel.fontSize = FONT_SIZE_3;
    gameOverScreenLabel.setCenterAlign();
    gameOverScreenLabel.textColor = BLACK;

    uiLibrary.rootContainer.AddChild(&gameOverScreenLabel);

    scoreLabel.text = "SCORE: " + std::to_string(score);
    scoreLabel.bounds = {
      windowWidth / 2, windowHeight / 2 - FONT_SIZE_3 * 2, 
      0, FONT_SIZE_2};
    scoreLabel.fontSize = FONT_SIZE_2;
    scoreLabel.setCenterAlign();
    scoreLabel.textColor = BLACK;

    uiLibrary.rootContainer.AddChild(&scoreLabel);

    saveScoreButton.text = "SAVE SCORE";
    saveScoreButton.bounds = {
      windowWidth / 2 - BUTTON_WIDTH_1 / 2,
      windowHeight / 2 - BUTTON_HEIGHT_1 / 2, BUTTON_WIDTH_1, BUTTON_HEIGHT_1};
    saveScoreButton.buttonAction = saveScore;

    uiLibrary.rootContainer.AddChild(&saveScoreButton);

    returnToMainMenuButton.text = "MAIN MENU";
    returnToMainMenuButton.bounds = {
      windowWidth / 2 - BUTTON_WIDTH_1 / 2,
      windowHeight / 2 + BUTTON_HEIGHT_1, BUTTON_WIDTH_1, BUTTON_HEIGHT_1};
    returnToMainMenuButton.buttonAction = goToMainMenu;

    uiLibrary.rootContainer.AddChild(&returnToMainMenuButton);
  }
};


struct MenuHandler {
  std::vector<Menu*> menuList;
  MainMenu mainMenu;
  ScoreScreen scoreScreen;
  PauseScreen pauseScreen;
  GameOverScreen gameOverScreen;

  void initialize(float windowWidth, float windowHeight) {
    mainMenu.createUI(windowWidth, windowHeight);
    scoreScreen.createUI(windowWidth, windowHeight);
    pauseScreen.createUI(windowWidth, windowHeight);
    gameOverScreen.createUI(windowWidth, windowHeight);

    currentMenu = 3;

    menuList.push_back(&mainMenu);
    menuList.push_back(&scoreScreen);
    menuList.push_back(&pauseScreen);
    menuList.push_back(&gameOverScreen);
  }

  void Update() { menuList[currentMenu]->Update(); }

  void Draw() { menuList[currentMenu]->Draw(); }
};


#endif