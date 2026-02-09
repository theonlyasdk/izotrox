#pragma once
#include "UI/View/View.hpp"
#include "UI/Layout/LinearLayout.hpp"
#include "UI/Widgets/Label.hpp"
#include "UI/Widgets/Button.hpp"
#include "UI/Widgets/TextBox.hpp"
#include "UI/Widgets/ListBox.hpp"
#include "UI/Widgets/ProgressBar.hpp"
#include "UI/Widgets/Slider.hpp"
#include "UI/Widgets/OptionBox.hpp"
#include "Graphics/Font.hpp"
#include <memory>

namespace Izo {

class Image;

class ThemePreviewView {
public:
    static std::unique_ptr<View> create();
};

}
