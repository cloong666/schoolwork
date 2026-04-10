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

    // 小球参数：半径、初始位置、初始速度（随机方向与大小）
    const int r = 20;                    // 小球半径
    double bx = 400.0, by = 200.0;       // 小球初始中心位置
    double bvx = randVel(2, 5) * randSign(); // 随机水平速度
    double bvy = randVel(2, 5) * randSign(); // 随机垂直速度

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

        // 更新小球位置：按当前速度移动
        bx += bvx;
        by += bvy;

        // 边界处理：当小球碰到画面边缘时，随机化反弹方向和速度
        // 左边界碰撞处理
        if (bx - r <= 0)
        {
            bx = r; // 修正位置，避免出界
            bvx = randVel(2, 5); // 水平向右随机速度
            bvy = randVel(-5, 5); // 垂直分量随机
            if (bvy == 0) bvy = 1; // 避免垂直速度为0
        }
        // 右边界碰撞处理
        else if (bx + r >= width)
        {
            bx = width - r; // 修正位置
            bvx = -randVel(2, 5); // 水平向左随机速度
            bvy = randVel(-5, 5); // 垂直分量随机
            if (bvy == 0) bvy = -1; // 避免垂直速度为0
        }

        // 上边界碰撞处理
        if (by - r <= 0)
        {
            by = r; // 修正位置
            bvy = randVel(2, 5); // 垂直向下随机速度
            bvx = randVel(-5, 5); // 水平分量随机
            if (bvx == 0) bvx = 1; // 避免水平速度为0
        }
        // 下边界碰撞处理
        else if (by + r >= height)
        {
            by = height - r; // 修正位置
            bvy = -randVel(2, 5); // 垂直向上随机速度
            bvx = randVel(-5, 5); // 水平分量随机
            if (bvx == 0) bvx = -1; // 避免水平速度为0
        }

        // 碰撞检测：小球与小车矩形碰撞的最近点距离法
        // 为了让碰撞箱尽量贴合小车图像，这里使用比 carW/carH 更紧凑的矩形
        int carLeft = carX + 10;                // 小车矩形左边界（贴合车身）
        int carTop = carY + 18;                 // 小车矩形上边界（贴合车顶）
        int carRight = carX + 190;              // 小车矩形右边界
        int carBottom = carY + 104;             // 小车矩形下边界（轮胎底部约为 104）

        // 计算小球中心到矩形的最近点坐标 nearestX, nearestY
        double nearestX = bx;
        if (nearestX < carLeft) nearestX = carLeft;
        if (nearestX > carRight) nearestX = carRight;
        double nearestY = by;
        if (nearestY < carTop) nearestY = carTop;
        if (nearestY > carBottom) nearestY = carBottom;

        // 最近点向量 nx, ny（从矩形到小球中心）
        double nx = bx - nearestX;
        double ny = by - nearestY;
        double dist2 = nx * nx + ny * ny; // 最近点到中心的平方距离

        // 如果距离小于等于半径平方，说明发生了碰撞或接触
        if (dist2 <= (r * r))
        {
            // 计算碰撞法线（从矩形表面指向小球）
            double len = sqrt(dist2);
            double nnx, nny; // 单位法线
            if (len == 0.0)
            {
                // 如果恰好重合，退回使用向上法线作为备用
                nnx = 0.0; nny = -1.0;
            }
            else
            {
                // 归一化最近点向量得到单位法线
                nnx = nx / len;
                nny = ny / len;
            }

            // 保存碰撞前小球速度大小（碰撞后保持不变）
            double speedBefore = sqrt(bvx * bvx + bvy * bvy);
            if (speedBefore < 0.001) speedBefore = 0.001; // 避免为零

            // 对小球速度进行反射（只改变方向，不改变速度大小）
            // 使用绝对速度在法线上的反射：v' = v - 2*(v·n)*n
            double dot = bvx * nnx + bvy * nny;
            double rvx = bvx - 2.0 * dot * nnx;
            double rvy = bvy - 2.0 * dot * nny;

            // 将反射向量归一化后按原速度大小恢复
            double rvSpeed = sqrt(rvx * rvx + rvy * rvy);
            if (rvSpeed < 0.0001)
            {
                // 若反射后速度近似为零，则随机给一个方向
                double ang = ((rand() % 360) * PI / 180.0);
                bvx = cos(ang) * speedBefore;
                bvy = sin(ang) * speedBefore;
            }
            else
            {
                bvx = rvx * (speedBefore / rvSpeed);
                bvy = rvy * (speedBefore / rvSpeed);
            }

            // 将小球沿法线推出表面以移除穿透（微小偏移）
            bx = nearestX + nnx * (r + 0.5);
            by = nearestY + nny * (r + 0.5);
        }

        // 绘制帧：先清屏
        cleardevice();

        // 绘制地面参考线（使用 groundY）
        setcolor(BLACK);
        line(0, groundY, width, groundY);

        // 使用 putimage 绘制小车图像到当前 carX, carY（保证车底贴地）
        putimage(carX, carY, &carImg);

        // 绘制小球（填充颜色）
        setfillcolor(LIGHTRED);
        setcolor(LIGHTRED);
        fillcircle((int)bx, (int)by, r);

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
