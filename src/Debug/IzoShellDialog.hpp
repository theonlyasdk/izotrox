#pragma once

#include "Graphics/Dialog.hpp"

namespace Izo {

class TextBox;

class IzoShellDialog : public Dialog {
public:
    IzoShellDialog();

    void draw_content(Painter& painter) override;
    void update() override;
    bool on_touch(IntPoint point, bool down, bool captured = false) override;
    bool on_key(KeyCode key) override;
    void close() override;

private:
    TextBox* m_input = nullptr;
};

}  // namespace Izo
