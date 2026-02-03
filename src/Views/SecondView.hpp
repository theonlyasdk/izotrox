#pragma once
#include "Graphics/View.hpp"
#include "Widgets/LinearLayout.hpp"
#include "Graphics/Label.hpp"
#include "Graphics/Button.hpp"
#include "Graphics/Font.hpp"
#include <memory>

namespace Izo {

class SecondView {
public:
    static std::shared_ptr<View> create(Font* font);
};

} 
