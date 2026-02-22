#pragma once

#include <memory>

namespace Izo {

class View;

class LauncherView {
   public:
    static std::unique_ptr<View> create();
};

}  // namespace Izo
