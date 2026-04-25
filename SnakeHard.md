# 代码重难点

## 1. 终端输入与原始模式
- 项目没有使用第三方库，而是直接操作 `termios` 与 `select()`。
- `SetRawMode(true)` 关闭回显（ECHO）和行缓冲（ICANON），并把 `VMIN=0`、`VTIME=1` 设置为非阻塞读取。
- 输入读取通过 `ReadKey()` 实现：先用 `select()` 检测 stdin 是否可读，再用 `read()` 获取字节。
- 方向键生成的是多字节转义序列，例如 `ESC [ A`，代码把它转换为 `1000 + 65`，方便后续判断。
- `SetRawMode(false)` 必须在退出前恢复终端状态，否则终端会继续停留在原始模式，导致用户无法正常输入和显示。

## 2. ANSI 控制与渲染
- 渲染方式是“字符终端绘制”，通过 ANSI 控制序列实现：
  - `\033[2J\033[H`：清屏并移动光标到左上角。
  - `\033[<y>;<x>H`：将光标定位到第 y 行、第 x 列。
  - `\033[0m`：重置所有颜色与样式。
- 颜色配置直接保存在 `config` 中，例如：
  - `config.snakeColor = "\033[32m";` // 绿色蛇身
  - `config.foodColor = "\033[31m";` // 红色食物
- 绘制时先输出颜色控制串，再输出字符，最后输出 `\033[0m` 复位，保证只对当前元素着色。
- 坐标系需要注意：ANSI 光标坐标从 1 开始，而游戏地图坐标从 0 开始，因此输出时要加 `+1`。

## 3. 游戏状态管理与移动逻辑
- 蛇身使用 `std::vector<Position>` 保存，头部总是 `snakeBody.front()`，尾部是 `snakeBody.back()`。
- 运动逻辑为：计算 `nextHead`，插入到 `snakeBody.begin()`，如果没有吃到食物则 `pop_back()` 移除尾部。
- 吃到食物时不删除尾部，从而实现“身体增长”。
- `moveDirection` 和 `lastDirection` 分离：`moveDirection` 是当前输入方向，`lastDirection` 是实际移动方向。
- 通过 `lastDirection` 判断是否允许换向，禁止直接 180 度掉头；若按下与当前方向相同的方向键，则触发加速 `accelerate = true`。

## 4. 难度与随机生成
- 难度由 `ChooseDifficulty()` 设置，主要影响：
  - `config.baseDelayMillis`：基础帧延迟。
  - `config.scorePerFood`：每个食物的分值。
- 难度还影响障碍物数量：`obstacleCount = 4 + difficultyLevel`，难度越高障碍越多。
- `SpawnFood()` 和 `SpawnObstacles()` 都依赖 `IsPositionAvailable()`，避免生成的坐标与蛇身或障碍重叠。
- 随机数使用 `std::rand()`，在 `InitializeGame()` 中根据当前时间初始化种子。

## 5. 菜单与输入容错
- 所有菜单输入都使用 `std::cin >> option` 读取，并检测输入失败或范围错误。
- 读取失败时执行：
  - `std::cin.clear()` 清除错误状态
  - `std::cin.ignore(1024, '\n')` 丢弃错误输入
- 这保证了用户输入非数字或超出范围时，程序不会直接崩溃或进入无限循环。
- 颜色和难度菜单设置完成后，提示用户按回车返回主界面，增强交互体验。

## 6. 碰撞检测与失败判定
- `DetectCollision()` 判断三类碰撞：
  - 撞到边界
  - 撞到自身
  - 撞到障碍物
- 边界判断使用 `nextHead.x <= 0 || nextHead.x >= config.mapWidth - 1 || nextHead.y <= 0 || nextHead.y >= config.mapHeight - 1`。
- 由于障碍物和食物也在地图内部生成，检测逻辑必须严格区分这些对象，避免错误判定。

## 7. 退出与终端恢复
- 游戏结束或返回菜单时，都要调用 `SetRawMode(false)` 和 `SetCursorVisible(true)`。
- `ClearScreen()` 先清屏再定位光标，避免残留旧画面。
- 结束时输出 `config.textColor` 后再输出 `\033[0m`，保证退出提示也能正确恢复默认终端样式。
