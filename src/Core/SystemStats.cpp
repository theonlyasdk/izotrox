#include <fstream>
#include <string>
#include <sstream>
#include "SystemStats.hpp"

namespace Izo {
    float SystemStats::get_cpu_temp() {
        std::ifstream f("/sys/class/thermal/thermal_zone0/temp");
        if (f) {
            int temp;
            f >> temp;
            return temp / 1000.0f;
        }
        return 0.0f;
    }

    int SystemStats::get_free_memory_mb() {
        std::ifstream f("/proc/meminfo");
        std::string line;
        while (std::getline(f, line)) {
            if (line.find("MemAvailable:") == 0) {
                std::stringstream ss(line);
                std::string label;
                int kb;
                ss >> label >> kb;
                return kb / 1024;
            }
        }
        return 0;
    }
}
