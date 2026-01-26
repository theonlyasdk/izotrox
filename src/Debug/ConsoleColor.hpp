#pragma once

namespace Izo {

struct ConsoleColor {
    static constexpr const char* Reset          = "\033[0m";
    static constexpr const char* Black          = "\033[30m";
    static constexpr const char* Red            = "\033[31m";
    static constexpr const char* Green          = "\033[32m";
    static constexpr const char* Yellow         = "\033[33m";
    static constexpr const char* Blue           = "\033[34m";
    static constexpr const char* Magenta        = "\033[35m";
    static constexpr const char* Cyan           = "\033[36m";
    static constexpr const char* White          = "\033[37m";
    static constexpr const char* BrightBlack    = "\033[90m";
    static constexpr const char* BrightRed      = "\033[91m";
    static constexpr const char* BrightGreen    = "\033[92m";
    static constexpr const char* BrightYellow   = "\033[93m";
    static constexpr const char* BrightBlue     = "\033[94m";
    static constexpr const char* BrightMagenta  = "\033[95m";
    static constexpr const char* BrightCyan     = "\033[96m";
    static constexpr const char* BrightWhite    = "\033[97m";
};

}