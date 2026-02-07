#pragma once

namespace Izo {
class SystemStats {
public:
    static float cpu_temp();
    static int sys_free_memory_mb();
    static int app_memory_usage_mb();
};
}