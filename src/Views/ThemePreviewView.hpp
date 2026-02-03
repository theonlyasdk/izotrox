#pragma once
#include "Graphics/View.hpp"
#include "Widgets/LinearLayout.hpp"
#include "Graphics/Label.hpp"
#include "Graphics/Button.hpp"
#include "Graphics/TextBox.hpp"
#include "Widgets/ListBox.hpp"
#include "Graphics/ProgressBar.hpp"
#include "Graphics/Slider.hpp"
#include "Graphics/OptionBox.hpp"
#include "Graphics/Font.hpp"
#include <memory>

namespace Izo {

class Image;

class ThemePreviewView {
public:
    static std::shared_ptr<View> create(Font* font, Image* sliderHandle, Image* sliderHandleFocus);
};

}
