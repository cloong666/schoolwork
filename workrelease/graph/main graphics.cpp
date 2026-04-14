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

            // 碰撞检测：将小车拆分为多个基础碰撞体（车身/车顶/轮胎）
            // 从而比单一大矩形更贴合图像外形
            bool hitCar = false;
            double hitNx = 0.0, hitNy = -1.0;
            double bestPenetration = 0.0;

            auto considerHit = [&](double nx, double ny, double penetration)
            {
                if (penetration <= 0.0) return;
                if (!hitCar || penetration > bestPenetration)
                {
                    hitCar = true;
                    bestPenetration = penetration;
                    hitNx = nx;
                    hitNy = ny;
                }
            };

            auto testRect = [&](double left, double top, double right, double bottom)
            {
                double nearestX = balls[i].x;
                if (nearestX < left)  nearestX = left;
                if (nearestX > right) nearestX = right;

                double nearestY = balls[i].y;
                if (nearestY < top)    nearestY = top;
                if (nearestY > bottom) nearestY = bottom;

                double dx = balls[i].x - nearestX;
                double dy = balls[i].y - nearestY;
                double dist2 = dx * dx + dy * dy;
                if (dist2 > (double)(r * r)) return;

                if (dist2 < 1e-12)
                {
                    // 球心在矩形内部或极近边界时，按最近边给法线
                    double dl = balls[i].x - left;
                    double dr = right - balls[i].x;
                    double dt = balls[i].y - top;
                    double db = bottom - balls[i].y;

                    double minInside = dl;
                    double nx = -1.0, ny = 0.0;

                    if (dr < minInside) { minInside = dr; nx = 1.0; ny = 0.0; }
                    if (dt < minInside) { minInside = dt; nx = 0.0; ny = -1.0; }
                    if (db < minInside) { minInside = db; nx = 0.0; ny = 1.0; }

                    considerHit(nx, ny, r + minInside);
                }
                else
                {
                    double dist = sqrt(dist2);
                    considerHit(dx / dist, dy / dist, r - dist);
                }
            };

            auto testCircle = [&](double cx, double cy, double cr)
            {
                double dx = balls[i].x - cx;
                double dy = balls[i].y - cy;
                double dist2 = dx * dx + dy * dy;
                double minDist = r + cr;
                double minDist2 = minDist * minDist;
                if (dist2 > minDist2) return;

                if (dist2 < 1e-12)
                {
                    considerHit(0.0, -1.0, minDist);
                }
                else
                {
                    double dist = sqrt(dist2);
                    considerHit(dx / dist, dy / dist, minDist - dist);
                }
            };

            auto testSegment = [&](double x1, double y1, double x2, double y2)
            {
                double sx = x2 - x1;
                double sy = y2 - y1;
                double segLen2 = sx * sx + sy * sy;
                if (segLen2 < 1e-12) return;

                double bx = balls[i].x - x1;
                double by = balls[i].y - y1;
                double t = (bx * sx + by * sy) / segLen2;
                if (t < 0.0) t = 0.0;
                if (t > 1.0) t = 1.0;

                double px = x1 + t * sx;
                double py = y1 + t * sy;
                double dx = balls[i].x - px;
                double dy = balls[i].y - py;
                double dist2 = dx * dx + dy * dy;
                if (dist2 > (double)(r * r)) return;

                if (dist2 < 1e-12)
                {
                    considerHit(0.0, -1.0, (double)r);
                }
                else
                {
                    double dist = sqrt(dist2);
                    considerHit(dx / dist, dy / dist, (double)r - dist);
                }
            };

            // 车身主矩形
            testRect(carX + 10, carY + 30, carX + 190, carY + 80);
            // 车顶使用线段碰撞（更贴合斜边）
            testSegment(carX + 50,  carY + 30, carX + 90,  carY + 18);
            testSegment(carX + 90,  carY + 18, carX + 140, carY + 18);
            testSegment(carX + 140, carY + 18, carX + 160, carY + 30);
            // 轮胎圆形碰撞体
            testCircle(carX + 50,  carY + 88, 16.0);
            testCircle(carX + 140, carY + 88, 16.0);

            if (hitCar)
            {
                // 保存碰撞前小球速度大小
                double speedBefore = sqrt(balls[i].vx * balls[i].vx + balls[i].vy * balls[i].vy);
                if (speedBefore < 0.001) speedBefore = 0.001;

                // 对小球速度进行反射：v' = v - 2*(v.n)*n
                double dot = balls[i].vx * hitNx + balls[i].vy * hitNy;
                double rvx = balls[i].vx - 2.0 * dot * hitNx;
                double rvy = balls[i].vy - 2.0 * dot * hitNy;

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

                // 将小球沿法线推出小车表面以移除穿透
                balls[i].x += hitNx * (bestPenetration + 0.5);
                balls[i].y += hitNy * (bestPenetration + 0.5);
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
