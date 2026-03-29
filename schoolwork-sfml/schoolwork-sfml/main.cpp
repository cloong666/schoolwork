// 用SFML重写原graphics.h动画，加入 putimage 风格的车体复制渲染、往返轨迹和随机小球碰撞逻辑
#include <SFML/Graphics.hpp>
#include <cmath>
#include <type_traits>
#include <optional>
#include <random>
#include <algorithm>

// 颜色映射
const sf::Color RED = sf::Color::Red;
const sf::Color YELLOW = sf::Color::Yellow;
const sf::Color WHITE = sf::Color::White;
const sf::Color BROWN(139, 69, 19);
const sf::Color DARKGRAY(64, 64, 64);
const sf::Color SKY(20, 20, 30);
const sf::Color BUILDING(70, 70, 90);
const sf::Color GRASS(34, 139, 34);

int main() {
    // 窗口尺寸
    const unsigned int WIN_W = 1000u;
    const unsigned int WIN_H = 700u;

    // 创建窗口
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u{ WIN_W, WIN_H }), "shcoolwork");
    window.setFramerateLimit(60);

    bool running = true;

    // 随机数生成器
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_real_distribution<float> angDist(0.f, 2.f * 3.14159265f);
    std::uniform_real_distribution<float> smallAngle(-0.785398f, 0.785398f); // +/-45deg

    // ====== 预渲染小车到 RenderTexture（模拟 putimage：把车体绘制到一张图，然后每帧直接拷贝这张图） ======
    // 使用构造函数创建 RenderTexture（部分 SFML3 实现没有 create()）
    sf::RenderTexture carTex(sf::Vector2u{ 260u, 140u });
    carTex.clear(sf::Color::Transparent);

    // 在 carTex 上绘制车（使用相对于 carTex 的本地坐标）
    // 车身底层（更方正）
    sf::RectangleShape bodyBase(sf::Vector2f(200.f, 48.f));
    bodyBase.setFillColor(sf::Color(60, 30, 15));
    bodyBase.setPosition(sf::Vector2f(30.f, 58.f));
    carTex.draw(bodyBase);
    // 车身中层
    sf::RectangleShape bodyMid(sf::Vector2f(192.f, 40.f));
    bodyMid.setFillColor(BROWN);
    bodyMid.setPosition(sf::Vector2f(34.f, 62.f));
    carTex.draw(bodyMid);
    // 方正车厢（用矩形替代原来的凸多边形，使车体更方正）
    sf::RectangleShape cabin(sf::Vector2f(100.f, 44.f));
    cabin.setFillColor(sf::Color(200, 200, 200, 200));
    cabin.setPosition(sf::Vector2f(80.f, 20.f));
    cabin.setOutlineColor(WHITE);
    cabin.setOutlineThickness(2.f);
    carTex.draw(cabin);
    // 车头（方正前端）
    sf::RectangleShape nose(sf::Vector2f(30.f, 36.f));
    nose.setFillColor(BROWN);
    nose.setPosition(sf::Vector2f(170.f, 34.f));
    nose.setOutlineColor(WHITE);
    nose.setOutlineThickness(1.5f);
    carTex.draw(nose);
    // 窗户（方形）
    sf::RectangleShape winRect(sf::Vector2f(60.f, 28.f));
    winRect.setFillColor(sf::Color(160, 190, 240, 220));
    winRect.setOutlineColor(sf::Color(220, 220, 220));
    winRect.setOutlineThickness(1.5f);
    winRect.setPosition(sf::Vector2f(92.f, 28.f));
    carTex.draw(winRect);
    // 轮胎和轮毂（稍微内缩）
    sf::CircleShape wheelL(22.f), wheelRshape(22.f);
    wheelL.setOrigin(sf::Vector2f(22.f, 22.f));
    wheelRshape.setOrigin(sf::Vector2f(22.f, 22.f));
    wheelL.setFillColor(DARKGRAY);
    wheelRshape.setFillColor(DARKGRAY);
    wheelL.setPosition(sf::Vector2f(80.f, 116.f));
    wheelRshape.setPosition(sf::Vector2f(160.f, 116.f));
    carTex.draw(wheelL);
    carTex.draw(wheelRshape);
    sf::CircleShape rimL(12.f), rimR(12.f);
    rimL.setOrigin(sf::Vector2f(12.f, 12.f));
    rimR.setOrigin(sf::Vector2f(12.f, 12.f));
    rimL.setFillColor(sf::Color(200, 200, 200));
    rimR.setFillColor(sf::Color(200, 200, 200));
    rimL.setOutlineColor(sf::Color(30, 30, 30));
    rimR.setOutlineColor(sf::Color(30, 30, 30));
    rimL.setOutlineThickness(2.f);
    rimR.setOutlineThickness(2.f);
    rimL.setPosition(sf::Vector2f(80.f, 116.f));
    rimR.setPosition(sf::Vector2f(160.f, 116.f));
    carTex.draw(rimL);
    carTex.draw(rimR);
    // 车体轮廓
    sf::RectangleShape outline(sf::Vector2f(200.f, 48.f));
    outline.setFillColor(sf::Color::Transparent);
    outline.setOutlineColor(sf::Color(230, 230, 230));
    outline.setOutlineThickness(2.f);
    outline.setPosition(sf::Vector2f(30.f, 58.f));
    carTex.draw(outline);

    carTex.display();

    // 将 RenderTexture 的纹理做成 Sprite，每帧直接绘制这个 sprite（等同于 putimage）
    sf::Sprite carSprite(carTex.getTexture());
    // 设置原点为中心，便于按中心定位和碰撞检测
    carSprite.setOrigin(sf::Vector2f(carTex.getSize().x / 2.f, carTex.getSize().y / 2.f));

    // ====== 小车运动参数：沿水平轨道往返 ======
    const float carMinX = 120.f;
    const float carMaxX = float(WIN_W) - 240.f; // 让车在窗口内来回
    const float carY = 320.f; // 固定纵坐标（放低一些以配合地面）
    float carX = carMinX;
    float carSpeed = 150.f; // 像素/秒
    int carDir = 1; // 1 向右，-1 向左

    // ====== 小球参数：随机运动 ======
    const float ballR = 12.f;
    sf::CircleShape ball(ballR);
    ball.setFillColor(sf::Color::Green);
    // 初始位置随机
    std::uniform_real_distribution<float> posXDist(ballR, WIN_W - ballR);
    std::uniform_real_distribution<float> posYDist(ballR, WIN_H - ballR);
    sf::Vector2f ballPos(posXDist(rng), posYDist(rng));
    ball.setOrigin(sf::Vector2f(ballR, ballR));
    ball.setPosition(ballPos);
    // 初始速度随机方向
    float angle = angDist(rng);
    float ballSpeed = 200.f; // 像素/秒
    sf::Vector2f ballVel(std::cos(angle) * ballSpeed, std::sin(angle) * ballSpeed);

    // 时钟用于计算增量时间
    sf::Clock clock;

    // 预造一些背景建筑，以填充窗口避免过于空旷
    std::vector<sf::RectangleShape> buildings;
    for (int b = 0; b < 8; ++b) {
        float bw = 60.f + (b % 3) * 20.f;
        float bh = 80.f + (b % 4) * 40.f;
        sf::RectangleShape rect(sf::Vector2f(bw, bh));
        rect.setFillColor(BUILDING);
        rect.setPosition(sf::Vector2f(40.f + b * 110.f, 30.f + (b % 2) * 20.f));
        buildings.push_back(rect);
    }

    while (window.isOpen() && running) {
        // 处理事件
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            event->visit([&](const auto& ev) {
                using T = std::decay_t<decltype(ev)>;
                if constexpr (std::is_same_v<T, sf::Event::Closed>)
                    window.close();
                else if constexpr (std::is_same_v<T, sf::Event::KeyPressed>)
                    running = false;
                });
        }

        // 计算 delta
        float dt = clock.restart().asSeconds();
        if (dt <= 0.f) dt = 1.f / 60.f;

        // 更新小车位置（往返）
        carX += carDir * carSpeed * dt;
        if (carX > carMaxX) { carX = carMaxX; carDir = -1; }
        if (carX < carMinX) { carX = carMinX; carDir = 1; }
        carSprite.setPosition(sf::Vector2f(carX, carY));

        // 更新小球位置
        ballPos += ballVel * dt;

        // 检查屏幕边界：如果碰到边界，随机化方向（仍保证速度不为零）
        bool edgeHit = false;
        if (ballPos.x - ballR <= 0.f) { edgeHit = true; ballPos.x = ballR; }
        if (ballPos.x + ballR >= WIN_W) { edgeHit = true; ballPos.x = WIN_W - ballR; }
        if (ballPos.y - ballR <= 0.f) { edgeHit = true; ballPos.y = ballR; }
        if (ballPos.y + ballR >= WIN_H) { edgeHit = true; ballPos.y = WIN_H - ballR; }
        if (edgeHit) {
            float a = angDist(rng);
            ballVel = sf::Vector2f(std::cos(a) * ballSpeed, std::sin(a) * ballSpeed);
        }

        // 检测小球与小车碰撞（圆-矩形碰撞）
        // 计算 car 的包围盒（使用 carSprite 位置和 carTex 尺寸，避免依赖 FloatRect 字段差异）
        sf::Vector2u texSize = carTex.getSize();
        float carWf = float(texSize.x);
        float carHf = float(texSize.y);
        float carLeft = carX - carWf / 2.f;
        float carTop = carY - carHf / 2.f;
        float closestX = std::clamp(ballPos.x, carLeft, carLeft + carWf);
        float closestY = std::clamp(ballPos.y, carTop, carTop + carHf);
        float dx = ballPos.x - closestX;
        float dy = ballPos.y - closestY;
        float dist2 = dx * dx + dy * dy;
        if (dist2 <= ballR * ballR) {
            // 碰撞发生：将速度反向并施加小随机角度偏移
            float curA = std::atan2(ballVel.y, ballVel.x);
            float newA = curA + 3.14159265f + smallAngle(rng);
            ballVel = sf::Vector2f(std::cos(newA) * ballSpeed, std::sin(newA) * ballSpeed);
            // 将球推离碰撞体以防止持续碰撞
            float dist = std::sqrt(dist2);
            if (dist == 0.f) dist = 0.1f;
            float overlap = ballR - dist;
            ballPos.x += (dx / dist) * overlap;
            ballPos.y += (dy / dist) * overlap;
        }

        // 更新 ball 形状位置
        ball.setPosition(ballPos);

        // 绘制：先清屏 -> 背景元素 -> 小球 -> 小车（使用预渲染 sprite 模拟 putimage）
        window.clear(SKY);

        // 背景：建筑
        for (auto& b : buildings) window.draw(b);

        // 太阳（固定）
        sf::CircleShape sun(25.f);
        sun.setOrigin(sf::Vector2f(25.f, 25.f));
        sun.setPosition(sf::Vector2f(275.f, 60.f));
        sun.setFillColor(YELLOW);
        sun.setOutlineColor(WHITE);
        sun.setOutlineThickness(2.f);
        window.draw(sun);

        // 草地带（靠近地面，使画面不空旷）
        sf::RectangleShape grass(sf::Vector2f(float(WIN_W), 40.f));
        grass.setFillColor(GRASS);
        grass.setPosition(sf::Vector2f(0.f, 240.f));
        window.draw(grass);

        // 地面（道路）
        sf::RectangleShape ground(sf::Vector2f(float(WIN_W), 40.f));
        ground.setFillColor(DARKGRAY);
        ground.setPosition(sf::Vector2f(0.f, 280.f));
        window.draw(ground);
        // 道路虚线
        for (float x = 0.f; x < WIN_W; x += 40.f) {
            sf::RectangleShape dash(sf::Vector2f(20.f, 4.f));
            dash.setFillColor(WHITE);
            dash.setPosition(sf::Vector2f(x + 10.f, 298.f));
            window.draw(dash);
        }

        // 小球
        window.draw(ball);

        // 小车（putimage 风格：直接绘制预渲染的 sprite）
        window.draw(carSprite);

        window.display();
    }

    return 0;
}
