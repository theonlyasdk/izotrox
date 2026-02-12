#include "SecondView.hpp"
#include "Core/ViewManager.hpp"
#include "Core/ThemeDB.hpp"
#include "UI/Widgets/OptionBox.hpp"

#include "Core/ResourceManager.hpp"
#include "ThemePreviewView.hpp"

namespace Izo {

std::unique_ptr<View> SecondView::create() {
    auto root = std::make_unique<LinearLayout>(Orientation::Vertical);
    root->set_focusable(false);
    root->set_width(WidgetSizePolicy::MatchParent);
    root->set_height(WidgetSizePolicy::MatchParent);
    root->set_padding(20);

    auto title = std::make_unique<Label>("Second View");
    title->set_width(WidgetSizePolicy::MatchParent);
    root->add_child(std::move(title));

    auto description = std::make_unique<Label>("This is a second view pushed onto the view stack.");
    description->set_width(WidgetSizePolicy::MatchParent);
    description->set_wrap(true);
    root->add_child(std::move(description));

    // Demo OptionBox
    auto optionLabel = std::make_unique<Label>("Select a theme:");
    optionLabel->set_width(WidgetSizePolicy::MatchParent);
    root->add_child(std::move(optionLabel));

    auto optionBox = std::make_unique<OptionBox>();
    optionBox->set_width(WidgetSizePolicy::MatchParent);
    auto themes = ThemeDB::the().theme_names();
    if (themes.empty()) {
        themes.push_back("default");
    }
    optionBox->set_options(themes);
    optionBox->select(0);
    optionBox->set_on_change([](int index, const std::string& value) {
        (void)index;
        ThemeDB::the().load(std::format("themes/{}.ini", value));
    });

    root->add_child(std::move(optionBox));

    auto backBtn = std::make_unique<Button>("Go Back");
    backBtn->set_width(WidgetSizePolicy::MatchParent);
    backBtn->set_on_click([]() {
        ViewManager::the().pop();
    });
    root->add_child(std::move(backBtn));
    
    auto btn_next_screen = std::make_unique<Button>("Go to next screen");
    btn_next_screen->set_width(WidgetSizePolicy::MatchParent);
    btn_next_screen->set_on_click([]() {
        auto view = ThemePreviewView::create();
        ViewManager::the().push(std::move(view));
    });
    root->add_child(std::move(btn_next_screen));

    return std::make_unique<View>(std::move(root));
}

} 
