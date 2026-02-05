#include "ThemePreviewView.hpp"
#include "Graphics/Image.hpp"
#include "UI/Widgets/ListItem.hpp"
#include "SecondView.hpp"
#include "Core/ViewManager.hpp"

namespace Izo {

std::shared_ptr<View> ThemePreviewView::create(Font* font) {
    auto root = std::make_shared<LinearLayout>(Orientation::Vertical);
    root->set_focusable(false);
    root->set_width(WidgetSizePolicy::MatchParent);
    root->set_height(WidgetSizePolicy::MatchParent);
    root->set_padding(20);

    auto title = std::make_shared<Label>("Theme Preview", font);
    title->set_width(WidgetSizePolicy::MatchParent);
    root->add_child(title);

    auto btn_row = std::make_shared<LinearLayout>(Orientation::Horizontal);
    btn_row->set_width(WidgetSizePolicy::MatchParent);
    btn_row->set_padding(0);

    auto normal_btn = std::make_shared<Button>("Normal Button", font);
    normal_btn->set_width(WidgetSizePolicy::WrapContent);
    normal_btn->set_on_click([]() {
        ViewManager::the().pop();
    });
    btn_row->add_child(normal_btn);

    auto primary_btn = std::make_shared<Button>("Primary Button", font);
    primary_btn->set_width(WidgetSizePolicy::WrapContent);
    primary_btn->set_on_click([font]() {
        auto view = SecondView::create(font);
        ViewManager::the().push(view);
    });
    btn_row->add_child(primary_btn);

    root->add_child(btn_row);

    auto textbox = std::make_shared<TextBox>("Enter text here...", font);
    textbox->set_width(WidgetSizePolicy::MatchParent);
    root->add_child(textbox);

    auto listbox = std::make_shared<ListBox>();
    listbox->set_width(WidgetSizePolicy::MatchParent);
    listbox->set_height(200);

    for(int i=1; i<=5; i++) {
        auto item = std::make_shared<ListItem>(Orientation::Vertical);
        auto label = std::make_shared<Label>("List Item " + std::to_string(i), font);
        item->add_child(label);
        listbox->add_item(item);
    }
    // listbox->set_selected_index(1); // TODO
    
    root->add_child(listbox);

    auto progressbar = std::make_shared<ProgressBar>(0.6f);
    progressbar->set_width(WidgetSizePolicy::MatchParent);
    root->add_child(progressbar);

    auto slider = std::make_shared<Slider>(0.4f);
    slider->set_width(WidgetSizePolicy::MatchParent);
    root->add_child(slider);

    auto success_lbl = std::make_shared<Label>("Success: Theme loaded", font);
    success_lbl->set_color_variant(ColorVariant::Success);
    success_lbl->set_width(WidgetSizePolicy::MatchParent);
    root->add_child(success_lbl);

    auto error_lbl = std::make_shared<Label>("Error: Sample error", font);
    error_lbl->set_color_variant(ColorVariant::Error);
    error_lbl->set_width(WidgetSizePolicy::MatchParent);
    root->add_child(error_lbl);

    auto optionbox = std::make_shared<OptionBox>(font);
    optionbox->set_width(WidgetSizePolicy::MatchParent);
    optionbox->add_option("Option 1");
    optionbox->add_option("Option 2");
    optionbox->add_option("Option 3");
    optionbox->select(0);
    root->add_child(optionbox);

    return std::make_shared<View>(root);
}

}
