#pragma once

#include <cstdint>

namespace Izo {

// Direction / flow
enum class Orientation {
    Horizontal,
    Vertical
};

enum class FlowDirection {
    LeftToRight,
    RightToLeft,
    TopToBottom,
    BottomToTop
};

// Alignment
enum class Align {
    Start,
    Center,
    End,
    Stretch
};

enum class Anchor {
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight,
    Center,
    CenterStartHoriz, 
    CenterStartVert,  
    CenterEndHoriz,   
    CenterEndVert     
};

enum class TextAlign {
    Left,
    Center,
    Right,
    Justify
};

// Sizing
enum class SizePolicy {
    Fixed,
    WrapContent,
    MatchParent,
    Expand
};

// Visibility / state
enum class Visibility {
    Visible,
    Hidden,
    Collapsed
};

enum class FocusPolicy {
    None,
    Click,
    Tab,
    Strong
};

// Input state
enum class InputMode {
    None,
    Mouse,
    Touch,
    Keyboard,
    Gamepad
};

// Widget state
enum class WidgetState : std::uint32_t {
    None      = 0,
    Hovered   = 1 << 0,
    Pressed   = 1 << 1,
    Focused   = 1 << 2,
    Disabled  = 1 << 3,
    Selected  = 1 << 4
};

inline WidgetState operator|(WidgetState a, WidgetState b) {
    return static_cast<WidgetState>(static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
}

inline WidgetState operator&(WidgetState a, WidgetState b) {
    return static_cast<WidgetState>(static_cast<std::uint32_t>(a) & static_cast<std::uint32_t>(b));
}

inline bool state_has_flag(WidgetState value, WidgetState flag) {
    return (value & flag) != WidgetState::None;
}

} // namespace Izo
