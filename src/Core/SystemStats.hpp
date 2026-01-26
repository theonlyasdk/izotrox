#pragma once

namespace Izo {
class SystemStats {
public:
    static float get_cpu_temp();
    static int get_free_memory_mb();
};
}