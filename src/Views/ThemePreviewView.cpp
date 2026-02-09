#include "ThemePreviewView.hpp"
#include "Graphics/Image.hpp"
#include "UI/Widgets/ListItem.hpp"
#include "SecondView.hpp"
#include "Core/ViewManager.hpp"

namespace Izo {

std::unique_ptr<View> ThemePreviewView::create(Font* font) {
    auto root = std::make_unique<LinearLayout>(Orientation::Vertical);
    root->set_focusable(false);
    root->set_width(WidgetSizePolicy::MatchParent);
    root->set_height(WidgetSizePolicy::MatchParent);
    root->set_padding(20);

    auto title = std::make_unique<Label>("Theme Preview", font);
    title->set_width(WidgetSizePolicy::MatchParent);
    root->add_child(std::move(title));

    auto btn_row = std::make_unique<LinearLayout>(Orientation::Horizontal);
    btn_row->set_width(WidgetSizePolicy::MatchParent);
    btn_row->set_padding(0);

    auto normal_btn = std::make_unique<Button>("Normal Button", font);
    normal_btn->set_width(WidgetSizePolicy::WrapContent);
    normal_btn->set_on_click([]() {
        ViewManager::the().pop();
    });
    btn_row->add_child(std::move(normal_btn));

    auto primary_btn = std::make_unique<Button>("Primary Button", font);
    primary_btn->set_width(WidgetSizePolicy::WrapContent);
    primary_btn->set_on_click([font]() {
        auto view = SecondView::create(font);
        ViewManager::the().push(std::move(view));
    });
    btn_row->add_child(std::move(primary_btn));

    root->add_child(std::move(btn_row));

    auto textbox = std::make_unique<TextBox>("Enter text here...", font);
    textbox->set_width(WidgetSizePolicy::MatchParent);
    root->add_child(std::move(textbox));

    auto listbox = std::make_unique<ListBox>();
    listbox->set_width(WidgetSizePolicy::MatchParent);
    listbox->set_height(200);

    for(int i=1; i<=5; i++) {
        auto item = std::make_unique<ListItem>(Orientation::Vertical);
        auto label = std::make_unique<Label>("List Item " + std::to_string(i), font);
        item->add_child(std::move(label));
        listbox->add_item(std::move(item));
    }
    // listbox->set_selected_index(1); // TODO
    
    root->add_child(std::move(listbox));

    auto progressbar = std::make_unique<ProgressBar>(true);
    progressbar->set_width(WidgetSizePolicy::MatchParent);
    root->add_child(std::move(progressbar));

    auto slider = std::make_unique<Slider>(0.4f);
    slider->set_width(WidgetSizePolicy::MatchParent);
    root->add_child(std::move(slider));

    auto success_lbl = std::make_unique<Label>("Success: Theme loaded", font);
    success_lbl->set_color_variant(ColorVariant::Success);
    success_lbl->set_width(WidgetSizePolicy::MatchParent);
    root->add_child(std::move(success_lbl));

    auto error_lbl = std::make_unique<Label>("Error: Sample error", font);
    error_lbl->set_color_variant(ColorVariant::Error);
    error_lbl->set_width(WidgetSizePolicy::MatchParent);
    root->add_child(std::move(error_lbl));

    auto optionbox = std::make_unique<OptionBox>(font);
    optionbox->set_width(WidgetSizePolicy::WrapContent);
    for(size_t i = 1; i <= 100; i++) {
        optionbox->add_option("Option " + std::to_string(i));
    }

    optionbox->select(0);
    root->add_child(std::move(optionbox));

    return std::make_unique<View>(std::move(root));
}

}
