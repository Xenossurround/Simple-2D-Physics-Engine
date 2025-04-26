#include <graphics.h>
#include <conio.h>
#include <vector>
#include <ctime>
#include <cmath>
#include <Windows.h>
#include <shellapi.h>
#include <thread>
#include <mutex>
#define MAXSTAR 200	

template <typename T>
const T& clamp(const T& v, const T& lo, const T& hi) {
    return (v < lo) ? lo : (v > hi) ? hi : v;
}

const double M_PI = 3.14159265;
const int consoleWidth = 200;
const double GRAVITY = 3;
const double BOUNCE = 90;
const int RADIUS = 20;
const int SQUARE_SIZE = 50;
const int maxBalls = 10;
const int maxParticles = 300;
const double RATE = 90;
const double damping = 0.9;
bool running = false;

class Slider {
public:
    int x, y, width, height;
    double minValue, maxValue;
    double value;
    bool dragging;

    const char* label;

    Slider(int _x, int _y, int _width, int _height, double _minValue, double _maxValue, double _value, const char* _label)
        : x(_x), y(_y), width(_width), height(_height),
        minValue(_minValue), maxValue(_maxValue), value(_value),
        dragging(false), label(_label) {}


    void draw() const {
        // 滑动条背景
        setfillcolor(RGB(200, 200, 200));
        solidrectangle(x, y, x + width, y + height);

        // 滑块位置
        int sliderX = x + (value - minValue) / (maxValue - minValue) * width;

        // 滑块
        setfillcolor(RGB(100, 100, 250));
        solidrectangle(sliderX - 5, y - 5, sliderX + 5, y + height + 5);

        // 标签
        settextcolor(BLACK);
        outtextxy(x, y - 20, label);
    }




    void startDragging(int mouseX) {
        dragging = true;
        update(mouseX);
    }


    void stopDragging() {
        dragging = false;
    }


    void update(int mouseX) {
        if (dragging) {
            int newSliderX = clamp(mouseX, x, x + width);
            value = minValue + (newSliderX - x) / (double)width * (maxValue - minValue);
        }
    }


    bool isHovered(int mouseX, int mouseY) const {
        return (mouseX >= x && mouseX <= x + width && mouseY >= y && mouseY <= y + height);
    }
};

void Button(int weight, int height, int weight1, int height1, int transx, int transy, int linew, const char* texts, COLORREF colors, COLORREF colorl, COLORREF colort)
{
    setlinestyle(PS_SOLID | PS_ENDCAP_FLAT, linew);
    setlinecolor(colorl);
    setfillcolor(colors);
    fillrectangle(weight, height, weight1, height1);
    settextcolor(colort);
    settextstyle(16, 0, _T("宋体"));
    outtextxy(weight + transx, height + transy, _T(texts));

}

struct STAR
{
    double	x;
    int		y;
    double	step;
    int		color;
};

STAR star[MAXSTAR];

void InitStar(int i)
{
    star[i].x = 0;
    star[i].y = rand() % 480;
    star[i].step = (rand() % 5000) / 1000.0 + 1;
    star[i].color = (int)(star[i].step * 255 / 6.0 + 0.5);
    star[i].color = RGB(star[i].color, star[i].color, star[i].color);
}


void MoveStar(int i)
{
    // 擦掉原来的星星
    putpixel((int)star[i].x, star[i].y, 0);

    // 计算新位置
    star[i].x += star[i].step;
    if (star[i].x > 1600)	InitStar(i);

    // 画新星星
    putpixel((int)star[i].x, star[i].y, star[i].color);
}
// 小球结构体
struct Ball {
    double x, y;
    double vx, vy;
    int radius;
    COLORREF color;

    Ball(double _x, double _y, double _vx, double _vy, int _r, COLORREF _color)
        : x(_x), y(_y), vx(_vx), vy(_vy), radius(_r), color(_color) {}

    void draw() {
        setlinecolor(BLACK);
        setlinestyle(PS_SOLID | PS_JOIN_BEVEL, 2);
        setfillcolor(color);
        fillellipse(x - radius, y - radius, x + radius, y + radius);
    }

    void update(int windowWidth, int windowHeight) {
        vy += (GRAVITY/10);
        x += vx;

        y += vy;

        if (x - radius < 0) {
            x = radius;
            vx = -vx * BOUNCE/100;
        }
        if (x + radius > windowWidth) {
            x = windowWidth - radius;
            vx = -vx * BOUNCE/100;
        }
        if (y - radius < 0) {
            y = radius;
            vy = -vy * BOUNCE/100;
        }
        if (y + radius > windowHeight) {
            y = windowHeight - radius;
            vy = -vy * BOUNCE/100;
        }
    }
};

void mouseClicker() {
    while (running) {

        mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
// 方块结构体
struct Square {
    double x, y;
    int size;
    COLORREF color;
    int increaseWa = 0;
    Square(double _x, double _y, int _size, COLORREF _color)
        : x(_x), y(_y), size(_size), color(_color) {}

    void draw() {
        setfillcolor(color);
        solidrectangle(x, y, x + size, y + size);
    }

    bool checkCollision(Ball& ball) {
        double closestX = clamp(ball.x, x, x + size);
        double closestY = clamp(ball.y, y, y + size);

        double dx = ball.x - closestX;
        double dy = ball.y - closestY;
        double distance = sqrt(dx * dx + dy * dy);

        if (distance < ball.radius) {
            double nx = dx / distance;
            double ny = dy / distance;

            double dotProduct = ball.vx * nx + ball.vy * ny;

            ball.vx -= 2 * dotProduct * nx * BOUNCE/100;
            ball.vy -= 2 * dotProduct * ny * BOUNCE/100;

            double overlap = ball.radius - distance;
            ball.x += nx * overlap;
            ball.y += ny * overlap;

            if (std::abs(ball.vy) < 0.1) {
                ball.vy = 0;
            }

            return true;
        }
        return false;
    }
};
struct FluidParticle {
    double x, y;       // 位置
    double vx, vy;     // 速度
    double density;    // 密度
    double pressure;   // 压力
    int radius;        // 粒子半径
    COLORREF color;    // 颜色

    FluidParticle(double _x, double _y, double _vx, double _vy, int _r, COLORREF _color)
        : x(_x), y(_y), vx(_vx), vy(_vy), radius(_r), color(_color), density(0), pressure(0) {}

    void draw() {
        setfillcolor(color);
        fillellipse(x - radius, y - radius, x + radius, y + radius);
    }
};
double kernel(double r, double h) {
    double q = r / h;
    if (q <= 1) {
        return (15 / (M_PI * pow(h, 5))) * pow(1 - q, 2);
    }
    return 0;
}

double kernelGradient(double r, double h) {
    double q = r / h;
    if (q <= 1) {
        return (-45 / (M_PI * pow(h, 6))) * (1 - q);
    }
    return 0;
}

void computeDensityAndPressure(std::vector<FluidParticle>& particles, double h) {
    const double K = 1.0;  // 气体常数
    const double REST_DENSITY = 1000.0;  // 静止密度

    for (auto& pi : particles) {
        pi.density = 0;
        for (auto& pj : particles) {
            double dx = pi.x - pj.x;
            double dy = pi.y - pj.y;
            double r = sqrt(dx * dx + dy * dy);
            pi.density += kernel(r, h);
        }
        pi.pressure = K * (pi.density - REST_DENSITY);
    }
}

void computeForces(std::vector<FluidParticle>& particles, double h) {
    const double VISCOSITY = 0.1;  // 粘性系数

    for (auto& pi : particles) {
        double fx = 0, fy = 0;
        for (auto& pj : particles) {
            double dx = pi.x - pj.x;
            double dy = pi.y - pj.y;
            double r = sqrt(dx * dx + dy * dy);

            if (r > 0) {
                double gradient = kernelGradient(r, h);
                fx += (pi.pressure + pj.pressure) / (2 * pj.density) * gradient * (dx / r);
                fy += (pi.pressure + pj.pressure) / (2 * pj.density) * gradient * (dy / r);

                // 粘性力
                fx += VISCOSITY * (pj.vx - pi.vx) / pj.density * kernel(r, h);
                fy += VISCOSITY * (pj.vy - pi.vy) / pj.density * kernel(r, h);
            }
        }

        // 更新速度
        pi.vx += fx;
        pi.vy += fy;
    }
}

void updateParticles(std::vector<FluidParticle>& particles, int windowWidth, int windowHeight) {
    for (auto& p : particles) {
        // 重力
        p.vy += (GRAVITY/10);

        // 更新位置
        p.x += p.vx;
        p.y += p.vy;

        // 边界碰撞
        if (p.x - p.radius < 0) {
            p.x = p.radius;
            p.vx *= -BOUNCE/100;
        }
        if (p.x + p.radius > windowWidth) {
            p.x = windowWidth - p.radius;
            p.vx *= -BOUNCE/100;
        }
        if (p.y - p.radius < 0) {
            p.y = p.radius;
            p.vy *= -BOUNCE/100;
        }
        if (p.y + p.radius > windowHeight) {
            p.y = windowHeight - p.radius;
            p.vy *= -BOUNCE/100;
        }
    }
}

void drawFluidSurface(const std::vector<FluidParticle>& particles, double h) {
    const int GRID_SIZE = 10;  // 网格大小
    const double THRESHOLD = 500.0;  // 密度阈值

    for (int x = 0; x < 1000; x += GRID_SIZE) {
        for (int y = 0; y < 700; y += GRID_SIZE) {
            double density = 0;
            for (const auto& p : particles) {
                double dx = x - p.x;
                double dy = y - p.y;
                double r = sqrt(dx * dx + dy * dy);
                density += kernel(r, h);
            }

            if (density > THRESHOLD) {
                setfillcolor(RGB(0, 0, 255));  // 流体颜色
                solidrectangle(x, y, x + GRID_SIZE, y + GRID_SIZE);
            }
        }
    }
}

// 检测小球之间的碰撞
void checkBallCollision(Ball& b1, Ball& b2) {
    double dx = b1.x - b2.x;
    double dy = b1.y - b2.y;
    double dist = sqrt(dx * dx + dy * dy);

    if (dist < b1.radius + b2.radius) {
        double angle = atan2(dy, dx);
        double sinAngle = sin(angle);
        double cosAngle = cos(angle);

        double v1x = b1.vx * cosAngle + b1.vy * sinAngle;
        double v1y = b1.vy * cosAngle - b1.vx * sinAngle;
        double v2x = b2.vx * cosAngle + b2.vy * sinAngle;
        double v2y = b2.vy * cosAngle - b2.vx * sinAngle;

        std::swap(v1x, v2x);
        v1x *= RATE/100;
        v2x *= RATE/100;
        b1.vx = v1x * cosAngle - v1y * sinAngle;
        b1.vy = v1y * cosAngle + v1x * sinAngle;
        b2.vx = v2x * cosAngle - v2y * sinAngle;
        b2.vy = v2y * cosAngle + v2x * sinAngle;

        double overlap = (b1.radius + b2.radius - dist) / 2.0;
        b1.x += overlap * cosAngle;
        b1.y += overlap * sinAngle;
        b2.x -= overlap * cosAngle;
        b2.y -= overlap * sinAngle;
    }
}

// 获取当前帧率
int getFPS(clock_t& lastTime, int& frameCount, int& fps) {
    frameCount++;
    clock_t currentTime = clock();
    double elapsedTime = double(currentTime - lastTime) / CLOCKS_PER_SEC;

    if (elapsedTime >= 1.0) {
        fps = frameCount;
        frameCount = 0;
        lastTime = currentTime;
    }

    return fps;
}


int main() {


    Sleep(1000);

    srand((unsigned)time(NULL));
    bool inControlArea;
    bool inSpawnArea;
    bool inButtonFirstArea;
    bool inButtonSecondArea;
    bool inButtonThirdArea;
    bool showTutorial = false;
    bool rightButtonHeld = false;
    bool drawStars = false;

    int rightButtonX = 0, rightButtonY = 0;
    int windowWidth = 1600, windowHeight = 900;

    HWND hwnd = initgraph(windowWidth, windowHeight);
    HWND hWn = GetHWnd();
    SetWindowText(hWn, "2DEngine-Xenos(version1.1)");
    srand((unsigned)time(NULL));
    std::vector<Ball> balls;
    std::vector<Square> squares;
    clock_t lastTime = clock();  // FPS计算
    int frameCount = 0;
    int fps = 0;
    setbkcolor(WHITE);
    BeginBatchDraw();

    //setbkcolor(RGB(99, 99, 99));

    Slider gravitySlider(10, 50, consoleWidth - 20, 10, 0, 10, GRAVITY, "质量");
    Slider radiusSlider(10, 100, consoleWidth - 20, 10, 5, 50, RADIUS, "圆球半径");
    Slider squareSizeSlider(10, 150, consoleWidth - 20, 10, 10, 100, SQUARE_SIZE, "方形边长");
    Slider bounceSlider(10, 200, consoleWidth - 20, 10, 0, 100, BOUNCE, "弹性");
    Slider decayrate(10, 250, consoleWidth - 20, 10, 0, 100, RATE, "势能衰减");

    Slider* activeSlider = nullptr;
    settextcolor(WHITE);
    for (int i = 0; i < MAXSTAR; i++)
    {
        InitStar(i);
        star[i].x = rand() % 640;
    }
    std::thread clickerThread;
    std::vector<FluidParticle> fluidParticles;
    while (true) {
        
        if (MouseHit()) {
            MOUSEMSG msg = GetMouseMsg();


            inControlArea = (msg.x >= 0 && msg.x <= consoleWidth && msg.y >= 0 && msg.y <= 270); // 控制台区域
            inSpawnArea = (msg.x >= 0 && msg.x <= 1000 && msg.y >= 0 && msg.y <= 700);
            inButtonFirstArea = (msg.x >= 950 && msg.x <= 1000 && msg.y >= 0 && msg.y <= 50);
            inButtonSecondArea = (msg.x >= 950 && msg.x <= 1000 && msg.y >= 50 && msg.y <= 100);
            inButtonThirdArea = (msg.x >= 950 && msg.x <= 1000 && msg.y >= 100 && msg.y <= 150);
            
            if (msg.uMsg == WM_LBUTTONDOWN) {
                if (inControlArea) {

                    if (gravitySlider.isHovered(msg.x, msg.y)) {
                        activeSlider = &gravitySlider;
                    }
                    else if (radiusSlider.isHovered(msg.x, msg.y)) {
                        activeSlider = &radiusSlider;
                    }
                    else if (squareSizeSlider.isHovered(msg.x, msg.y)) {
                        activeSlider = &squareSizeSlider;
                    }
                    else if (bounceSlider.isHovered(msg.x, msg.y)) {
                        activeSlider = &bounceSlider;
                    }
                    else if (decayrate.isHovered(msg.x, msg.y)) {
                        activeSlider = &decayrate;
                    }


                    if (activeSlider) {
                        activeSlider->startDragging(msg.x);
                    }
                }
                else if (!inButtonFirstArea && !inButtonSecondArea && !inButtonThirdArea && inSpawnArea) {

                    COLORREF color = RGB(rand() % 256, rand() % 256, rand() % 256);  // 随机颜色
                    balls.emplace_back(msg.x, msg.y, 0, 0, radiusSlider.value, color);  // 创建小球
                }
                else if (inButtonFirstArea)
                {


                    balls.clear();    // 清空小球
                    squares.clear();  // 清空方块

                    cleardevice();
                }
                else if (inButtonSecondArea)
                {
                    if (drawStars == false)
                        drawStars = true;
                    else
                        drawStars = false;

                }
                else if (inButtonThirdArea)
                {

                }


            }
            else if (msg.uMsg == WM_RBUTTONDOWN && inSpawnArea) {

                rightButtonHeld = true;
                rightButtonX = msg.x;
                rightButtonY = msg.y;
            }
            else if (msg.uMsg == WM_RBUTTONUP && inSpawnArea) {

                rightButtonHeld = false;
            }
            else if (msg.uMsg == WM_MBUTTONDOWN && inSpawnArea) {
                
            }



            else if (msg.uMsg == WM_LBUTTONUP) {

                if (activeSlider) {
                    activeSlider->stopDragging();
                    activeSlider = nullptr;
                }
            }




            if (activeSlider) {
                activeSlider->update(msg.x);
            }
        }
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            //  ESC 清屏
            balls.clear();
            squares.clear();
            cleardevice();
        }

        if (GetAsyncKeyState('G') & 0x8000) {

            showTutorial = !showTutorial;
            Sleep(200);
        }
        if (GetAsyncKeyState(VK_F3) & 0x8000) {
            if (!running) {
                running = true;
                clickerThread = std::thread(mouseClicker);

            }
        }


        if (GetAsyncKeyState(VK_F2) & 0x8000) {
            if (running) {
                running = false;
                if (clickerThread.joinable()) {
                    clickerThread.join();
                }

            }
        }
        if (rightButtonHeld && !inControlArea && inSpawnArea) {

            bool ctrlPressed = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;

            if (ctrlPressed) {

                for (auto it = squares.begin(); it != squares.end(); ++it) {
                    if (rightButtonX >= it->x && rightButtonX <= it->x + it->size && rightButtonY >= it->y && rightButtonY <= it->y + it->size) {
                        squares.erase(it);
                        break;
                    }
                }
            }
            else {

                bool blockExists = false;

                for (auto& square : squares) {
                    if (rightButtonX >= square.x && rightButtonX <= square.x + square.size && rightButtonY >= square.y && rightButtonY <= square.y + square.size) {
                        blockExists = true;
                        break;
                    }
                }

                if (!blockExists) {

                    squares.emplace_back(rightButtonX - squareSizeSlider.value / 2, rightButtonY - squareSizeSlider.value / 2, squareSizeSlider.value, BLACK);
                }
            }

            POINT cursorPos;
            GetCursorPos(&cursorPos);
            ScreenToClient(GetHWnd(), &cursorPos);
            rightButtonX = cursorPos.x;
            rightButtonY = cursorPos.y;
        }



        cleardevice();
        for (auto& ball : balls) {
            ball.update(windowWidth - 600, windowHeight - 200);
            ball.draw();
        }


        for (auto& square : squares) {
            square.draw();
            for (auto& ball : balls) {
                square.checkCollision(ball);
            }
        }


        for (size_t i = 0; i < balls.size(); i++) {
            for (size_t j = i + 1; j < balls.size(); j++) {
                checkBallCollision(balls[i], balls[j]);
            }
        }
        

        gravitySlider.draw();
        radiusSlider.draw();
        squareSizeSlider.draw();
        bounceSlider.draw();
        decayrate.draw();
        if (drawStars == true)
        {
            for (int i = 0; i < MAXSTAR; i++)
                MoveStar(i);
            Sleep(20);
        }

        Button(949, 0, 999, 50, 10, 15, 3, "清屏", BLUE, BLACK, WHITE);
        Button(949, 50, 999, 100, 10, 15, 3, "慢放", BLUE, BLACK, WHITE);
        Button(949, 100, 999, 150, 10, 15, 3, "画图", BLUE, BLACK, WHITE);
        Button(1000, 700, 0, 1600, 0, 0, 3, "", RGB(99, 99, 99), RED, RED);
        Button(1000, 0, 1300, 900, 0, 0, 3, "", RGB(99, 99, 99), BLUE, BLUE);
        Button(1300, 0, 1600, 900, 0, 0, 3, "", RGB(99, 99, 99), YELLOW, BLUE);
        if (showTutorial) {
            setfillcolor(RGB(50, 50, 50));
            solidrectangle(250, 100, 750, 500);
            settextcolor(WHITE);
            outtextxy(270, 120, "2D物理引擎 by XenosMC,bili_XenosMC");
            outtextxy(270, 160, "1.左键生成小球");
            outtextxy(270, 200, "2.右键生成方块");
            outtextxy(270, 240, "3.按住右键可连续生成方块");
            outtextxy(270, 280, "4.按住ctrl+右键可连续消除方块");
            outtextxy(270, 360, "5.G键呼出教程、关闭教程");
            outtextxy(270, 320, "F3连点，F2停止");
        }

        // 显示 FPS
        fps = getFPS(lastTime, frameCount, fps);
        settextcolor(BLACK);
        setbkmode(TRANSPARENT);
        TCHAR fpsText[20];
        _stprintf_s(fpsText, _T("FPS: %d"), fps);
        outtextxy(10, 10, fpsText);


        FlushBatchDraw();
        Sleep(5);
    }


    EndBatchDraw();

    closegraph();
    return 0;
}
