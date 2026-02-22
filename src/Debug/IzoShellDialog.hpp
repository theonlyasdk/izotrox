#pragma once

#include "Graphics/Dialog.hpp"
#include "Motion/Animator.hpp"

#include <string>

namespace Izo {

class TextBox;
class Label;

class IzoShellDialog : public Dialog {
public:
    IzoShellDialog();

    void draw_content(Painter& painter) override;
    void update() override;
    bool on_touch(IntPoint point, bool down, bool captured = false) override;
    bool on_key(KeyCode key) override;
    void close() override;
    bool has_running_animations() const override;

private:
    void set_output_text(const std::string& text, bool is_error);

    TextBox* m_input = nullptr;
    Label* m_output_label = nullptr;
    Animator<float> m_dialog_height_anim{0.0f};
    float m_dialog_target_height = 0.0f;
    bool m_dialog_size_initialized = false;
    bool m_touch_started_outside = false;
    std::string m_output_text;
    bool m_output_visible = false;
};

}  // namespace Izo
