#pragma once

#include <string>
#include <vector>

// 地图中的坐标点，x 为列索引，y 为行索引。
struct Position{
  int x;
  int y;
};

enum class Direction{
  Up,
  Down,
  Left,
  Right
};

// 游戏配置项，用于控制地图大小、速度、得分和颜色方案。
struct GameConfig{
  int mapWidth;
  int mapHeight;
  int baseDelayMillis;
  int scorePerFood;
  std::string snakeColor;
  std::string foodColor;
  std::string borderColor;
  std::string obstacleColor;
  std::string textColor;
};

class SnakeGame
{
  public:
    SnakeGame();
    void Run();

  private:
    void InitializeGame();               // 初始化游戏状态，包括蛇身、食物和障碍物。
    void ShowMainMenu();                 // 显示主菜单并处理用户选择。
    void ChooseDifficulty();             // 显示难度选择菜单并设置游戏参数。
    void ChooseColorScheme();            // 显示颜色选择菜单并设置显示方案。
    void SetupGame();                    // 未使用的备用初始化函数声明。
    void SpawnFood();                    // 生成一个新的食物坐标。
    void SpawnObstacles();               // 根据难度生成障碍物坐标。
    void DrawFrame() const;              // 绘制一帧游戏画面。
    void DrawBorder() const;             // 绘制地图边框。
    void DrawSnake() const;              // 绘制蛇身。
    void DrawFood() const;               // 绘制食物。
    void DrawObstacles() const;          // 绘制障碍物。
    void UpdateGame();                   // 更新游戏状态，包括蛇移动、吃食物和碰撞检测。
    void ProcessInput();                 // 处理玩家输入并更新移动方向。
    bool DetectCollision(const Position &nextHead) const; // 检查下一步是否碰撞。
    bool IsPositionAvailable(const Position &pos) const;  // 检查某个坐标是否可用。
    void ShowGameOver() const;           // 显示游戏结束界面。
    void WaitForMenuOption();            // 等待用户选择重新开始或退出。
    static void ClearScreen();           // 清屏。
    static void SetCursorVisible(bool visible); // 显示或隐藏光标。
    static void SetRawMode(bool enable); // 开启/关闭原始终端模式。
    static int ReadKey();                // 读取键盘输入。

  private:
    GameConfig config;                // 游戏参数配置
    std::vector<Position> snakeBody;  // 蛇身坐标列表，头部在第一个元素
    std::vector<Position> obstacles;  // 障碍物坐标列表
    Position food;                    // 当前食物坐标
    Direction moveDirection;          // 当前移动方向
    Direction lastDirection;          // 上一次确认的移动方向，用于避免掉头
    int score;                        // 当前得分
    bool gameOver;                    // 游戏是否结束
    bool isRunning;                   // 游戏主循环是否继续
    bool accelerate;                  // 当前是否进行加速移动
    int difficultyLevel;              // 难度等级
    bool hasColorScheme;              // 是否已选择颜色方案
};
