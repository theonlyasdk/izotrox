#pragma once
#include "UI/View/View.hpp"
#include "UI/Layout/LinearLayout.hpp"
#include "UI/Widgets/Label.hpp"
#include "UI/Widgets/Button.hpp"
#include "Graphics/Font.hpp"
#include <memory>

namespace Izo {

class SecondView {
public:
    static std::unique_ptr<View> create(Font* font);
};

} 
