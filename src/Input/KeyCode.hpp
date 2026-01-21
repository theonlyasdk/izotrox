#pragma once

namespace Izo {

enum class KeyCode : int {
    None = 0,
    Backspace = 8,
    Tab = 9,
    Enter = 13,
    Space = 32,
    
    // Printable ASCII range 32-126 are implicitly supported via casting

    Left = 200,
    Right = 201,
    Up = 202,
    Down = 203
};

} // namespace Izo
