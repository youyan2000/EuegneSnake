#include "SnakeGame.hpp"
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>
#include <vector>

static struct termios originalTerminalState;

SnakeGame::SnakeGame()
    : score(0), gameOver(false), isRunning(true), accelerate(false), difficultyLevel(2), hasColorScheme(false)
{
  // 默认配置
  config.mapWidth = 40;
  config.mapHeight = 20;
  config.baseDelayMillis = 200;
  config.scorePerFood = 10;
  config.snakeColor = "\033[32m";    // 绿色蛇身
  config.foodColor = "\033[31m";     // 红色食物
  config.borderColor = "\033[34m";   // 蓝色边框
  config.obstacleColor = "\033[35m"; // 品红色障碍物
  config.textColor = "\033[37m";     // 白色文字
}

// 运行游戏主循环，负责显示菜单、初始化游戏、处理游戏结束和退出逻辑。
void SnakeGame::Run(){
  ShowMainMenu();
  SetCursorVisible(false);
  SetRawMode(true);
  while (isRunning){
    InitializeGame();
    while (!gameOver){
      DrawFrame();
      ProcessInput();
      UpdateGame();
      int delay = config.baseDelayMillis;
      if (accelerate){
        delay = std::max(50, delay / 2);
      }
      usleep(delay * 1000);
    }
    SetRawMode(false);
    SetCursorVisible(true);
    ShowGameOver();
    WaitForMenuOption();
    if (isRunning){
      SetRawMode(true);
      SetCursorVisible(false);
    }
  }
  SetRawMode(false);
  SetCursorVisible(true);
  ClearScreen();
  std::cout << config.textColor << "谢谢游玩！再见。\n"
            << "\033[0m";
}

// 主菜单显示与输入处理。
void SnakeGame::ShowMainMenu()
{
  ClearScreen();
  std::cout << config.textColor;
  std::cout << "===== 欢迎进入贪吃蛇游戏 =====\n";
  std::cout << "1. 开始游戏\n";
  std::cout << "2. 选择难度\n";
  std::cout << "3. 选择颜色\n";
  std::cout << "4. 退出游戏\n";
  std::cout << "请输入选项编号并回车：";
  std::cout << "\033[0m";
  int option = 0;
  while (true){
    if (!(std::cin >> option)){
      std::cin.clear();
      std::cin.ignore(1024, '\n');
      std::cout << "输入无效，请输入 1-4 之间的数字：";
      continue;
    }
    if (option < 1 || option > 4){
      std::cout << "输入超出范围，请重新输入：";
      continue;
    }
    break;
  }
  std::cin.ignore(1024, '\n');
  switch (option){
    case 1:
      break;
    case 2:
      ChooseDifficulty();
      ShowMainMenu();
      return;
    case 3:
      ChooseColorScheme();
      ShowMainMenu();
      return;
    case 4:
      isRunning = false;
      return;
  }
}

// 难度选择菜单，调整速度与得分。
void SnakeGame::ChooseDifficulty(){
  ClearScreen();
  std::cout << config.textColor;
  std::cout << "===== 难度选择 =====\n";
  std::cout << "1. 简单 (慢速, 每个食物 10 分)\n";
  std::cout << "2. 普通 (中速, 每个食物 20 分)\n";
  std::cout << "3. 困难 (快速, 每个食物 30 分)\n";
  std::cout << "请输入选项编号并回车：";
  std::cout << "\033[0m";
  int option = 0;
  while (true){
    if (!(std::cin >> option)){
      std::cin.clear();
      std::cin.ignore(1024, '\n');
      std::cout << "输入无效，请输入 1-3 之间的数字：";
      continue;
    }
    if (option < 1 || option > 3){
      std::cout << "输入超出范围，请重新输入：";
      continue;
    }
    break;
  }
  std::cin.ignore(1024, '\n');
  difficultyLevel = option;
  if (option == 1){
    config.baseDelayMillis = 250;
    config.scorePerFood = 10;
  }else if (option == 2){
    config.baseDelayMillis = 170;
    config.scorePerFood = 20;
  }else{
    config.baseDelayMillis = 120;
    config.scorePerFood = 30;
  }
  std::cout << "难度设置完成。按回车返回主界面。";
  std::cin.get();
}

// 颜色选择菜单，设置蛇、食物、边框和障碍物的显示颜色。
void SnakeGame::ChooseColorScheme(){
  ClearScreen();
  std::cout << config.textColor;
  std::cout << "===== 颜色选择 =====\n";
  std::cout << "1. 经典绿蛇\n";
  std::cout << "2. 夕阳黄\n";
  std::cout << "3. 冰霜蓝\n";
  std::cout << "请输入选项编号并回车：";
  std::cout << "\033[0m";
  int option = 0;
  while (true){
    if (!(std::cin >> option)){
      std::cin.clear();
      std::cin.ignore(1024, '\n');
      std::cout << "输入无效，请输入 1-3 之间的数字：";
      continue;
    }
    if (option < 1 || option > 3){
      std::cout << "输入超出范围，请重新输入：";
      continue;
    }
    break;
  }
  std::cin.ignore(1024, '\n');
  if (option == 1){
    config.snakeColor = "\033[32m";
    config.foodColor = "\033[31m";
    config.borderColor = "\033[34m";
    config.obstacleColor = "\033[35m";
  }else if (option == 2){
    config.snakeColor = "\033[33m";
    config.foodColor = "\033[91m";
    config.borderColor = "\033[93m";
    config.obstacleColor = "\033[95m";
  }else{
    config.snakeColor = "\033[36m";
    config.foodColor = "\033[92m";
    config.borderColor = "\033[94m";
    config.obstacleColor = "\033[96m";
  }
  hasColorScheme = true;
  std::cout << "颜色设置完成。按回车返回主界面。";
  std::cin.get();
}

// 初始化游戏状态：蛇身、食物、障碍物、分数和方向。
void SnakeGame::InitializeGame(){
  snakeBody.clear();
  obstacles.clear();
  gameOver = false;
  accelerate = false;
  score = 0;
  moveDirection = Direction::Right;
  lastDirection = Direction::Right;
  int centerX = config.mapWidth / 2;
  int centerY = config.mapHeight / 2;
  int initialLength = 3;
  for (int i = 0; i < initialLength; ++i){
    snakeBody.push_back({centerX - i, centerY});
  }
  std::srand(static_cast<unsigned int>(std::time(nullptr)));
  SpawnFood();
  SpawnObstacles();
}

// 随机生成不与蛇身或障碍物重叠的食物位置。
void SnakeGame::SpawnFood(){
  Position candidate;
  do{
    candidate.x = std::rand() % (config.mapWidth - 2) + 1;
    candidate.y = std::rand() % (config.mapHeight - 2) + 1;
  } while (!IsPositionAvailable(candidate));
  food = candidate;
}

// 根据难度生成若干障碍物。
void SnakeGame::SpawnObstacles(){
  int obstacleCount = 4 + difficultyLevel;
  Position candidate;
  for (int i = 0; i < obstacleCount; ++i){
    do{
      candidate.x = std::rand() % (config.mapWidth - 2) + 1;
      candidate.y = std::rand() % (config.mapHeight - 2) + 1;
    } while (!IsPositionAvailable(candidate) || (candidate.x == food.x && candidate.y == food.y));
    obstacles.push_back(candidate);
  }
}

// 判断某个坐标是否可以放置蛇身、食物或障碍物。
bool SnakeGame::IsPositionAvailable(const Position &pos) const{
  if (pos.x <= 0 || pos.x >= config.mapWidth - 1 || pos.y <= 0 || pos.y >= config.mapHeight - 1){
    return false;
  }
  for (const auto &part : snakeBody){
    if (part.x == pos.x && part.y == pos.y){
      return false;
    }
  }
  for (const auto &mine : obstacles){
    if (mine.x == pos.x && mine.y == pos.y){
      return false;
    }
  }
  return true;
}

// 绘制当前画面：边框、障碍物、食物、蛇身和提示文字。
void SnakeGame::DrawFrame() const{
  ClearScreen();
  DrawBorder();
  DrawObstacles();
  DrawFood();
  DrawSnake();
  std::cout << "\033[" << (config.mapHeight + 2) << ";1H"; // 定位到地图下方第一列
  std::cout << config.textColor;
  std::cout << "得分: " << score << "    难度: " << difficultyLevel << "    当前速度: ";
  std::cout << (accelerate ? "加速" : "正常") << "    按键：WASD 或 方向键，空格加速\n";
  std::cout << "按 Q 退出本局" << "\033[0m\n";
}

// 绘制地图边框。
void SnakeGame::DrawBorder() const{
  std::cout << config.borderColor;
  for (int x = 0; x < config.mapWidth; ++x){
    std::cout << "#";
  }
  std::cout << "\n";
  for (int y = 1; y < config.mapHeight - 1; ++y){
    std::cout << "#";
    for (int x = 1; x < config.mapWidth - 1; ++x){
      std::cout << " ";
    }
    std::cout << "#\n";
  }
  for (int x = 0; x < config.mapWidth; ++x){
    std::cout << "#";
  }
  std::cout << "\033[0m\n";
}

// 绘制蛇身。
void SnakeGame::DrawSnake() const{
  std::cout << config.snakeColor;
  for (const auto &part : snakeBody)
  {
    std::cout << "\033[" << (part.y + 1) << ";" << (part.x + 1) << "H" << "O";
  }
  std::cout << "\033[0m";
}

// 绘制食物。
void SnakeGame::DrawFood() const
{
  std::cout << config.foodColor;
  std::cout << "\033[" << (food.y + 1) << ";" << (food.x + 1) << "H" << "@";
  std::cout << "\033[0m";
}

// 绘制障碍物。
void SnakeGame::DrawObstacles() const
{
  std::cout << config.obstacleColor;
  for (const auto &mine : obstacles)
  {
    std::cout << "\033[" << (mine.y + 1) << ";" << (mine.x + 1) << "H" << "*";
  }
  std::cout << "\033[0m";
}

// 读取玩家输入，将按键转换为移动方向或加速行为。
void SnakeGame::ProcessInput()
{
  int key = ReadKey();
  if (key == 0){
    accelerate = false;
    return;
  }
  accelerate = false;
  switch (key){
    case 'w':
    case 'W':
    case 1000 + 65: // up arrow
      if (lastDirection != Direction::Down){
        if (lastDirection == Direction::Up)
          accelerate = true;
        moveDirection = Direction::Up;
      }
      break;
    case 's':
    case 'S':
    case 1000 + 66: // down arrow
      if (lastDirection != Direction::Up){
        if (lastDirection == Direction::Down)
          accelerate = true;
        moveDirection = Direction::Down;
      }
      break;
    case 'a':
    case 'A':
    case 1000 + 68: // left arrow
      if (lastDirection != Direction::Right){
        if (lastDirection == Direction::Left)
          accelerate = true;
        moveDirection = Direction::Left;
      }
      break;
    case 'd':
    case 'D':
    case 1000 + 67: // right arrow
      if (lastDirection != Direction::Left){
        if (lastDirection == Direction::Right)
          accelerate = true;
        moveDirection = Direction::Right;
      }
      break;
    case ' ':
      accelerate = true;
      break;
    case 'q':
    case 'Q':
      gameOver = true;
      break;
    default:
      break;
  }
}

// 更新蛇的位置、检测碰撞并处理食物逻辑。
void SnakeGame::UpdateGame(){
  if (gameOver){
    return;
  }
  Position head = snakeBody.front();
  Position nextHead = head;
  switch (moveDirection){
    case Direction::Up:
      nextHead.y -= 1;
      break;
    case Direction::Down:
      nextHead.y += 1;
      break;
    case Direction::Left:
      nextHead.x -= 1;
      break;
    case Direction::Right:
      nextHead.x += 1;
      break;
  }
  if (DetectCollision(nextHead)){
    gameOver = true;
    return;
  }
  snakeBody.insert(snakeBody.begin(), nextHead);
  lastDirection = moveDirection;
  if (nextHead.x == food.x && nextHead.y == food.y){
    score += config.scorePerFood;
    SpawnFood();
  }else{
    snakeBody.pop_back();
  }
}

// 检查下一个蛇头位置是否会与边界、蛇身或障碍物相撞。
bool SnakeGame::DetectCollision(const Position &nextHead) const{
  if (nextHead.x <= 0 || nextHead.x >= config.mapWidth - 1 || nextHead.y <= 0 || nextHead.y >= config.mapHeight - 1){
    return true;
  }
  for (const auto &part : snakeBody){
    if (part.x == nextHead.x && part.y == nextHead.y){
      return true;
    }
  }
  for (const auto &mine : obstacles){
    if (mine.x == nextHead.x && mine.y == nextHead.y){
      return true;
    }
  }
  return false;
}

// 游戏结束时显示总结界面。
void SnakeGame::ShowGameOver() const{
  ClearScreen();
  std::cout << config.textColor;
  std::cout << "===== 游戏失败 =====\n";
  std::cout << "最终得分: " << score << "\n";
  std::cout << "撞到身体、障碍物或边框都会结束游戏。\n";
  std::cout << "1. 再次尝试\n";
  std::cout << "2. 退出游戏\n";
  std::cout << "请输入选项编号并回车：";
  std::cout << "\033[0m";
}

// 等待用户选择是否重新开始或退出。
void SnakeGame::WaitForMenuOption(){
  int option = 0;
  while (true){
    if (!(std::cin >> option)){
      std::cin.clear();
      std::cin.ignore(1024, '\n');
      std::cout << "输入无效，请输入 1-2 之间的数字：";
      continue;
    }
    if (option == 1){
      std::cin.ignore(1024, '\n');
      gameOver = false;
      break;
    }else if (option == 2){
      std::cin.ignore(1024, '\n');
      isRunning = false;
      break;
    }else{
      std::cout << "输入超出范围，请重新输入：";
    }
  }
}

// 清除终端屏幕。
void SnakeGame::ClearScreen(){
  std::cout << "\033[2J\033[H";
}

// 控制光标显示或隐藏。
void SnakeGame::SetCursorVisible(bool visible){
  if (visible){
    std::cout << "\033[?25h";
  }else{
    std::cout << "\033[?25l";
  }
}

// 切换终端原始输入模式，以便读取单字符输入。
void SnakeGame::SetRawMode(bool enable){
  if (enable){
    tcgetattr(STDIN_FILENO, &originalTerminalState);
    termios raw = originalTerminalState;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
  }else{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &originalTerminalState);
  }
}

// 从终端读取一个按键，如果是方向键则返回特殊编码。
int SnakeGame::ReadKey()
{
  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(STDIN_FILENO, &readfds);
  timeval timeout{0, 0};
  int result = select(STDIN_FILENO + 1, &readfds, nullptr, nullptr, &timeout);
  if (result <= 0)
  {
    return 0;
  }
  char buffer[3] = {0};
  ssize_t count = read(STDIN_FILENO, buffer, sizeof(buffer));
  if (count <= 0)
  {
    return 0;
  }
  if (buffer[0] == '\033' && count >= 3 && buffer[1] == '[')
  {
    return 1000 + static_cast<int>(buffer[2]);
  }
  return static_cast<int>(buffer[0]);
}
