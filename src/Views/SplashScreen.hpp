#pragma once
#include "Graphics/Painter.hpp"
#include "Graphics/Canvas.hpp"
#include "Graphics/Font.hpp"
#include "Core/Application.hpp"
#include <string>

namespace Izo {

class SplashScreen {
public:
    SplashScreen(Application& app, Painter& painter, Canvas& canvas, Font& font);

    void set_total_steps(int steps);
    void next_step(const std::string& status);

private:
    void render();

    Application& app;
    Painter& painter;
    Canvas& canvas;
    Font& font;

    int totalSteps = 1;
    int currentStep = 0;
    std::string currentStatus;
    bool windowShown = false;
};

} 
