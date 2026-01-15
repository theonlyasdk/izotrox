#include "SplashScreen.hpp"
#include "../Debug/Logger.hpp"
#include <thread>

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
    
    render();
}

void SplashScreen::render() {
    int width = canvas.width();
    int height = canvas.height();
    
    canvas.clear(Color(20, 20, 20));
    
    std::string title = "Izotrox";
    int tw = font.width(title);
    int th = font.height();
    font.draw_text(painter, (width - tw) / 2, (height - th) / 2, title, Color::White);
    
    float pct = (float)currentStep / (float)totalSteps;
    int pctInt = (int)(pct * 100);
    std::string pctStr = std::to_string(pctInt) + "%";
    
    int barHeight = 10;
    int pw = font.width(pctStr);
    
    int pctY = height - barHeight - th - 5; 
    font.draw_text(painter, (width - pw) / 2, pctY, pctStr, Color(200, 200, 200));
    
    if (!currentStatus.empty()) {
        int sw = font.width(currentStatus);
        font.draw_text(painter, (width - sw) / 2, pctY - th - 5, currentStatus, Color(150, 150, 150));
    }

    painter.fill_rect(0, height - barHeight, width, barHeight, Color(50, 50, 50));
    int fillW = (int)(width * pct);
    painter.fill_rect(0, height - barHeight, fillW, barHeight, Color(0, 120, 215));
    
    app.present(canvas);
}

} // namespace Izo