#pragma once

namespace Izo {
class SystemStats {
public:
    static float cpu_temp();
    static int free_memory_mb();
    static int app_memory_usage_mb();
};
}