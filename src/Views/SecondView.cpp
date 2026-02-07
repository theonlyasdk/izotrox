#include "SecondView.hpp"
#include "Core/ViewManager.hpp"
#include "Core/ThemeDB.hpp"
#include "UI/Widgets/OptionBox.hpp"

#include "Core/ResourceManager.hpp"
#include "ThemePreviewView.hpp"

namespace Izo {

std::shared_ptr<View> SecondView::create(Font* font) {
    auto root = std::make_shared<LinearLayout>(Orientation::Vertical);
    root->set_focusable(false);
    root->set_width(WidgetSizePolicy::MatchParent);
    root->set_height(WidgetSizePolicy::MatchParent);
    root->set_padding(20);

    auto title = std::make_shared<Label>("Second View", font);
    title->set_width(WidgetSizePolicy::MatchParent);
    root->add_child(title);

    auto description = std::make_shared<Label>("This is a second view pushed onto the view stack.", font);
    description->set_width(WidgetSizePolicy::MatchParent);
    description->set_wrap(true);
    root->add_child(description);

    // Demo OptionBox
    auto optionLabel = std::make_shared<Label>("Select a theme:", font);
    optionLabel->set_width(WidgetSizePolicy::MatchParent);
    root->add_child(optionLabel);

    auto optionBox = std::make_shared<OptionBox>(font);
    optionBox->set_width(WidgetSizePolicy::MatchParent);
    optionBox->add_option("default");
    optionBox->add_option("catppuccin-mocha");
    optionBox->add_option("dracula");
    optionBox->add_option("ios-light");
    optionBox->add_option("tokyo-night");
    optionBox->select(0);
    optionBox->set_on_change([](int index, const std::string& value) {
        ThemeDB::the().load(std::format("theme/{}.ini", value));
    });

    root->add_child(optionBox);

    auto backBtn = std::make_shared<Button>("Go Back", font);
    backBtn->set_width(WidgetSizePolicy::MatchParent);
    backBtn->set_on_click([]() {
        ViewManager::the().pop();
    });
    root->add_child(backBtn);
    
    auto btn_next_screen = std::make_shared<Button>("Go to next screen", font);
    btn_next_screen->set_width(WidgetSizePolicy::MatchParent);
    btn_next_screen->set_on_click([font]() {
        auto view = ThemePreviewView::create(font);
        ViewManager::the().push(view);
    });
    root->add_child(btn_next_screen);

    return std::make_shared<View>(root);
}

} 
