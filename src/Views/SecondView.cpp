#include "SecondView.hpp"
#include "Core/ViewManager.hpp"
#include "Core/ThemeDB.hpp"

namespace Izo {

std::shared_ptr<View> SecondView::create(Font* font) {
    auto root = std::make_shared<LinearLayout>(Orientation::Vertical);
    root->set_width(WidgetSizePolicy::MatchParent);
    root->set_height(WidgetSizePolicy::MatchParent);
    root->set_padding(20, 20, 20, 20);
    
    auto title = std::make_shared<Label>("Second View", font, ThemeDB::the().color("Label.Text"));
    title->set_width(WidgetSizePolicy::MatchParent);
    root->add_child(title);
    
    auto description = std::make_shared<Label>("This is a second view pushed onto the view stack.", font, ThemeDB::the().color("Label.Text"));
    description->set_width(WidgetSizePolicy::MatchParent);
    description->set_wrap(true);
    root->add_child(description);
    
    auto backBtn = std::make_shared<Button>("Go Back", font);
    backBtn->set_width(WidgetSizePolicy::MatchParent);
    backBtn->set_on_click([]() {
        ViewManager::the().pop(ViewTransition::SlideRight);
    });
    root->add_child(backBtn);
    
    return std::make_shared<View>(root);
}

} // namespace Izo
