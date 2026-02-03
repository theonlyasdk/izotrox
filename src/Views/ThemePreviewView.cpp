#include "ThemePreviewView.hpp"
#include "Graphics/Image.hpp"
#include "Widgets/ListItem.hpp"

namespace Izo {

std::shared_ptr<View> ThemePreviewView::create(Font* font, Image* sliderHandle, Image* sliderHandleFocus) {
    auto root = std::make_shared<LinearLayout>(Orientation::Vertical);
    root->set_focusable(false);
    root->set_width(WidgetSizePolicy::MatchParent);
    root->set_height(WidgetSizePolicy::MatchParent);
    root->set_padding(10, 10, 10, 10);

    auto title = std::make_shared<Label>("Theme Preview", font);
    title->set_width(WidgetSizePolicy::MatchParent);
    root->add_child(title);

    auto buttonRow = std::make_shared<LinearLayout>(Orientation::Horizontal);
    buttonRow->set_width(WidgetSizePolicy::MatchParent);

    auto normalButton = std::make_shared<Button>("Normal Button", font);
    normalButton->set_width(WidgetSizePolicy::WrapContent);
    buttonRow->add_child(normalButton);

    auto primaryButton = std::make_shared<Button>("Primary Button", font);
    primaryButton->set_width(WidgetSizePolicy::WrapContent);
    buttonRow->add_child(primaryButton);

    root->add_child(buttonRow);

    auto textBoxLabel = std::make_shared<Label>("Text Input:", font);
    textBoxLabel->set_width(WidgetSizePolicy::MatchParent);
    root->add_child(textBoxLabel);

    auto textBox = std::make_shared<TextBox>("Enter text here...", font);
    textBox->set_width(WidgetSizePolicy::MatchParent);
    root->add_child(textBox);

    auto listLabel = std::make_shared<Label>("List View:", font);
    listLabel->set_width(WidgetSizePolicy::MatchParent);
    root->add_child(listLabel);

    auto listView = std::make_shared<ListBox>();
    listView->set_width(WidgetSizePolicy::MatchParent);
    listView->set_height(200);

    for(int i=1; i<=5; i++) {
        auto item = std::make_shared<ListItem>(Orientation::Vertical);
        auto label = std::make_shared<Label>("List Item " + std::to_string(i), font);
        item->add_child(label);
        listView->add_item(item);
    }
    // listView->set_selected_index(1); // TODO
    
    root->add_child(listView);

    auto progressLabel = std::make_shared<Label>("Progress Bar:", font);
    progressLabel->set_width(WidgetSizePolicy::MatchParent);
    root->add_child(progressLabel);

    auto progressBar = std::make_shared<ProgressBar>(0.6f);
    progressBar->set_width(WidgetSizePolicy::MatchParent);
    root->add_child(progressBar);

    auto sliderLabel = std::make_shared<Label>("Slider:", font);
    sliderLabel->set_width(WidgetSizePolicy::MatchParent);
    root->add_child(sliderLabel);

    auto slider = std::make_shared<Slider>(sliderHandle, sliderHandleFocus, 0.4f);
    slider->set_width(WidgetSizePolicy::MatchParent);
    root->add_child(slider);

    auto optionLabel = std::make_shared<Label>("Option Box:", font);
    optionLabel->set_width(WidgetSizePolicy::MatchParent);
    root->add_child(optionLabel);

    auto optionBox = std::make_shared<OptionBox>(font);
    optionBox->set_width(WidgetSizePolicy::MatchParent);
    optionBox->add_option("Option 1");
    optionBox->add_option("Option 2");
    optionBox->add_option("Option 3");
    optionBox->set_selected_index(0);
    root->add_child(optionBox);

    auto successLabel = std::make_shared<Label>("Success: Theme loaded", font);
    successLabel->set_color_variant(ColorVariant::Success);
    successLabel->set_width(WidgetSizePolicy::MatchParent);
    root->add_child(successLabel);

    auto errorLabel = std::make_shared<Label>("Error: Sample error", font);
    errorLabel->set_color_variant(ColorVariant::Error);
    errorLabel->set_width(WidgetSizePolicy::MatchParent);
    root->add_child(errorLabel);

    return std::make_shared<View>(root);
}

}
