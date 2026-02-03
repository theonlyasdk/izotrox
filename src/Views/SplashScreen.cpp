#include "SplashScreen.hpp"
#include "Debug/Logger.hpp"
#include <format>

namespace Izo {

SplashScreen::SplashScreen(Application& app, Painter& painter, Canvas& canvas, Font& font)
    : app(app), painter(painter), canvas(canvas), font(font) 
{
}

void SplashScreen::set_total_steps(int steps) {
    totalSteps = steps;
    currentStep = 0;
}

void SplashScreen::next_step(const std::string& status) {
    currentStatus = status;
    currentStep++;
    if (currentStep > totalSteps) currentStep = totalSteps;
    LogDebug("SplashScreen::next_step(): {}", status);
    render();
}

void SplashScreen::render() {
    int width = canvas.width();
    int height = canvas.height();

    canvas.clear(Color::White);

    std::string title = "Izotrox";
    int tw = font.width(title);
    int th = font.height();
    font.draw_text(painter, {(width - tw) / 2, (height - th) / 2}, title, Color::Black);

    float completed_steps = (float)currentStep / (float)totalSteps;
    int completed_percent = (int)(completed_steps * 100);
    std::string pctStr = std::to_string(completed_percent) + "%";

    int barh = 10;
    int pw = font.width(pctStr);

    int pctY = height - barh - th - 5; 
    font.draw_text(painter, {(width - pw) / 2, pctY}, pctStr, Color(200, 200, 200));

    if (!currentStatus.empty()) {
        int sw = font.width(currentStatus);
        font.draw_text(painter, {(width - sw) / 2, pctY - th - 5}, currentStatus, Color(150, 150, 150));
    }

    painter.fill_rect({0, height - barh, width, barh}, Color(50, 50, 50));
    int fillW = (int)(width * completed_steps);
    painter.fill_rect({0, height - barh, fillW, barh}, Color(0, 120, 215));

    // Show the window before presenting the first frame
    if (!windowShown) {
        app.show();
        windowShown = true;
    }

    app.present(canvas);
}

} 