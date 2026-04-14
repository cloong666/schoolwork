// 合并的小车与小球仿真程序
// 小车使用 getimage/putimage 移动；小球遵循物理反射与随机化的边界行为

#include <graphics.h>
#include <conio.h>
#include <Windows.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// 定义 PI 常量，避免依赖 M_PI
static const double PI = 3.14159265358979323846;

// 处理窗口消息的辅助函数：如果接收到 WM_QUIT/WM_CLOSE 返回 false
static bool ProcessWindowMessages()
{
    MSG msg;
    // 循环处理消息队列
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        // 如果收到退出消息，则返回 false
        if (msg.message == WM_QUIT || msg.message == WM_CLOSE)
            return false;
        // 转换并分发消息
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return true; // 正常
}

// 生成随机正负符号（返回 1 或 -1）
static int randSign()
{
    return (rand() % 2) ? 1 : -1;
}

// 在闭区间 [minv, maxv] 返回随机整数速度
static int randVel(int minv, int maxv)
{
    return minv + rand() % (maxv - minv + 1);
}

// 小球结构体：包含位置、速度、半径、质量和颜色
struct Ball
{
    double x, y;    // 中心位置
    double vx, vy;  // 速度
    double r;       // 半径
    double m;       // 质量
    COLORREF color; // 颜色
};

int main()
{
    // 窗口宽高设置
    const int width = 800;
    const int height = 600;

    // 用时间作为随机种子
    srand((unsigned)time(NULL));

    // 初始化 EasyX 图形窗口
    initgraph(width, height);
    // 设置背景色为白色并清屏
    setbkcolor(WHITE);
    cleardevice();

    // 准备小车图像的尺寸（用于 getimage/putimage）
    const int carW = 200;
    const int carH = 140;
    IMAGE carImg; // 存放小车位图的对象

    // 在屏幕原点绘制小车，然后用 getimage 捕获为图像
    cleardevice();

    // 设置车身填充颜色并绘制车身矩形（实心）
    setfillcolor(RGB(200, 0, 0));
    setcolor(RGB(200, 0, 0));
    // 车身主矩形（左上 x=10,y=30 右下 x=190,y=80）
    solidrectangle(10, 30, 190, 80);

    // 绘制车顶多边形（更像车顶形状）
    POINT roof[4] = { {50,30}, {90,18}, {140,18}, {160,30} };
    setfillcolor(RGB(180,0,0));
    setcolor(RGB(180,0,0));
    fillpolygon(roof, 4);

    // 绘制车窗（填充为蓝色）
    setfillcolor(RGB(135,206,235)); // 天空蓝
    setcolor(RGB(135,206,235));
    fillrectangle(95, 35, 140, 65);

    // 画出车身轮廓与细节（黑色）
    setcolor(BLACK);
    rectangle(10, 30, 190, 80); // 车身轮廓
    line(50, 30, 90, 18);       // 车顶线
    line(90, 18, 140, 18);     // 车顶线
    line(140, 18, 160, 30);    // 车顶线

    // 车灯（填充黄色）
    setfillcolor(YELLOW);
    setcolor(YELLOW);
    fillcircle(185, 60, 8);

    // 排气口（黑色小矩形）
    setfillcolor(BLACK);
    setcolor(BLACK);
    solidrectangle(2, 70, 8, 74);

    // 轮胎绘制：调整轮胎中心位置与半径以减少轮胎与车身之间的缝隙
    // 将轮胎中心上移到 y=88，半径设为 16，使轮胎与车身更贴合
    setfillcolor(DARKGRAY);
    setcolor(DARKGRAY);
    // 左轮中心 (50,88)，右轮中心 (140,88)，半径 16
    fillcircle(50, 88, 16);
    fillcircle(140, 88, 16);
    // 车轮内圈（轮毂）使用较小的黑色圆形
    setfillcolor(BLACK);
    setcolor(BLACK);
    fillcircle(50, 88, 6);
    fillcircle(140, 88, 6);

    // 车辆底部的细节线（地面参考线）
    setcolor(BLACK);
    line(0, 110, 200, 110);

    // 捕获小车位图到 carImg 对象中（复制 0,0 到 carW,carH 区域）
    getimage(&carImg, 0, 0, carW, carH);

    // 清屏准备主循环
    cleardevice();

    // 开启批量绘制（减少闪烁）
    BeginBatchDraw();

    // 设定地面 y 坐标，并使小车在地面上行驶（车底贴地）
    const int groundY = 500;             // 地面在窗口中的 y 坐标
    int carX = 50;                       // 小车当前 x 坐标
    const int carY = groundY - carH;     // 使小车底部与地面对齐
    int carDir = 1;                      // 小车方向：1 向右，-1 向左
    const int carSpeed = 4;              // 小车每帧位移

    // 小球参数：半径与多球数组
    const int r = 20;                    // 小球半径（统一）
    const int NUM_BALLS = 5;             // 小球数量
    Ball balls[5];                       // 小球数组
    // 初始位置（分散排列，避免初始重叠）
    const double initX[5] = { 150.0, 400.0, 650.0, 200.0, 600.0 };
    const double initY[5] = { 150.0, 300.0, 150.0, 400.0, 350.0 };
    const COLORREF ballColors[5] = { LIGHTRED, LIGHTBLUE, LIGHTGREEN, YELLOW, MAGENTA };
    for (int i = 0; i < NUM_BALLS; i++)
    {
        balls[i].x = initX[i];
        balls[i].y = initY[i];
        balls[i].r = r;
        balls[i].m = 1.0;
        balls[i].color = ballColors[i];
        // 随机初速度：方向随机，大小在 [2, 5]
        double ang = (rand() % 360) * PI / 180.0;
        double spd = (double)randVel(2, 5);
        balls[i].vx = spd * cos(ang);
        balls[i].vy = spd * sin(ang);
    }

    // 主循环运行标志
    bool running = true;

    // 主循环：处理输入、更新物理、绘制帧
    while (running && !_kbhit())
    {
        // 处理窗口消息，若窗口关闭则退出
        running = ProcessWindowMessages();
        if (!running) break;

        // 更新小车位置：根据方向与速度移动
        carX += carDir * carSpeed;
        // 到达左边界则反向并修正位置
        if (carX <= 0) { carX = 0; carDir = 1; }
        // 到达右边界则反向并修正位置
        if (carX + carW >= width) { carX = width - carW; carDir = -1; }

        // 更新各小球位置并处理边界与小车碰撞
        for (int i = 0; i < NUM_BALLS; i++)
        {
            // 更新位置
            balls[i].x += balls[i].vx;
            balls[i].y += balls[i].vy;

            // 边界弹性碰撞（恢复系数 e=1）：反射速度并修正位置
            if (balls[i].x - r <= 0)
            {
                balls[i].x = r;
                balls[i].vx = fabs(balls[i].vx);
            }
            else if (balls[i].x + r >= width)
            {
                balls[i].x = width - r;
                balls[i].vx = -fabs(balls[i].vx);
            }
            if (balls[i].y - r <= 0)
            {
                balls[i].y = r;
                balls[i].vy = fabs(balls[i].vy);
            }
            else if (balls[i].y + r >= height)
            {
                balls[i].y = height - r;
                balls[i].vy = -fabs(balls[i].vy);
            }

            // 碰撞检测：小球与小车矩形碰撞的最近点距离法
            // 为了让碰撞箱尽量贴合小车图像，这里使用比 carW/carH 更紧凑的矩形
            int carLeft   = carX + 10;   // 小车矩形左边界
            int carTop    = carY + 18;   // 小车矩形上边界
            int carRight  = carX + 190;  // 小车矩形右边界
            int carBottom = carY + 104;  // 小车矩形下边界

            // 计算小球中心到矩形的最近点坐标
            double nearestX = balls[i].x;
            if (nearestX < carLeft)  nearestX = carLeft;
            if (nearestX > carRight) nearestX = carRight;
            double nearestY = balls[i].y;
            if (nearestY < carTop)    nearestY = carTop;
            if (nearestY > carBottom) nearestY = carBottom;

            // 最近点向量 cnx, cny（从矩形到小球中心）
            double cnx = balls[i].x - nearestX;
            double cny = balls[i].y - nearestY;
            double dist2 = cnx * cnx + cny * cny; // 最近点到中心的平方距离

            // 如果距离小于等于半径平方，说明发生了碰撞或接触
            if (dist2 <= (double)(r * r))
            {
                // 计算碰撞法线（从矩形表面指向小球）
                double len = sqrt(dist2);
                double nnx, nny; // 单位法线
                if (len < 0.0001)
                {
                    nnx = 0.0; nny = -1.0;
                }
                else
                {
                    nnx = cnx / len;
                    nny = cny / len;
                }

                // 保存碰撞前小球速度大小
                double speedBefore = sqrt(balls[i].vx * balls[i].vx + balls[i].vy * balls[i].vy);
                if (speedBefore < 0.001) speedBefore = 0.001;

                // 对小球速度进行反射：v' = v - 2*(v.n)*n
                double dot = balls[i].vx * nnx + balls[i].vy * nny;
                double rvx = balls[i].vx - 2.0 * dot * nnx;
                double rvy = balls[i].vy - 2.0 * dot * nny;

                // 将反射向量归一化后按原速度大小恢复
                double rvSpeed = sqrt(rvx * rvx + rvy * rvy);
                if (rvSpeed < 0.0001)
                {
                    double ang = ((rand() % 360) * PI / 180.0);
                    balls[i].vx = cos(ang) * speedBefore;
                    balls[i].vy = sin(ang) * speedBefore;
                }
                else
                {
                    balls[i].vx = rvx * (speedBefore / rvSpeed);
                    balls[i].vy = rvy * (speedBefore / rvSpeed);
                }

                // 将小球沿法线推出表面以移除穿透
                balls[i].x = nearestX + nnx * (r + 0.5);
                balls[i].y = nearestY + nny * (r + 0.5);
            }
        }

        // 小球间完全弹性碰撞（动量守恒 + 动能守恒，恢复系数 e=1）
        // 使用冲量法：沿碰撞法向量分解速度，交换法向分量
        for (int i = 0; i < NUM_BALLS; i++)
        {
            for (int j = i + 1; j < NUM_BALLS; j++)
            {
                double dx = balls[j].x - balls[i].x;
                double dy = balls[j].y - balls[i].y;
                double dist2 = dx * dx + dy * dy;
                double minDist = 2.0 * r; // 两球半径之和

                if (dist2 >= minDist * minDist || dist2 < 1e-12)
                    continue; // 未接触或重合则跳过

                double dist = sqrt(dist2);
                // 单位法向量（从球 i 指向球 j）
                double nx = dx / dist;
                double ny = dy / dist;

                // 相对速度沿法向分量（正值表示两球相向靠近）
                double rel = (balls[i].vx - balls[j].vx) * nx
                           + (balls[i].vy - balls[j].vy) * ny;

                if (rel > 0.0) // 仅处理靠近情形，避免分离时重复碰撞
                {
                    double mi = balls[i].m;
                    double mj = balls[j].m;
                    // 完全弹性冲量大小（e=1）： imp = 2*rel / (1/mi + 1/mj)
                    double imp = 2.0 * rel / (1.0 / mi + 1.0 / mj);
                    balls[i].vx -= (imp / mi) * nx;
                    balls[i].vy -= (imp / mi) * ny;
                    balls[j].vx += (imp / mj) * nx;
                    balls[j].vy += (imp / mj) * ny;
                }

                // 位置修正：将重叠的两球均匀分开，防止抖动
                double overlap = minDist - dist;
                if (overlap > 0.0)
                {
                    double corr = overlap * 0.5;
                    balls[i].x -= corr * nx;
                    balls[i].y -= corr * ny;
                    balls[j].x += corr * nx;
                    balls[j].y += corr * ny;
                }
            }
        }

        // 绘制帧：先清屏
        cleardevice();

        // 绘制地面参考线（使用 groundY）
        setcolor(BLACK);
        line(0, groundY, width, groundY);

        // 使用 putimage 绘制小车图像到当前 carX, carY（保证车底贴地）
        putimage(carX, carY, &carImg);

        // 绘制所有小球
        for (int i = 0; i < NUM_BALLS; i++)
        {
            setfillcolor(balls[i].color);
            setcolor(balls[i].color);
            fillcircle((int)balls[i].x, (int)balls[i].y, r);
        }

        // 刷新批量绘制的内容到屏幕
        FlushBatchDraw();

        // 控制帧率（睡眠 30 毫秒）
        Sleep(30);
    }

    // 结束批量绘制并关闭图形窗口
    EndBatchDraw();
    closegraph();
    return 0;
}
