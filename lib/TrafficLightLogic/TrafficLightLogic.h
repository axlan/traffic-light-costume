#include <stdlib.h>

enum class LightMode
{
    RED,
    YELLOW,
    GREEN
};

class TrafficLight
{
public:
    static constexpr float YELLOW_THRESHOLD = 1.0;
    static constexpr float GREEN_THRESHOLD = 1.5;

    LightMode Update(float vel);

private:
    static constexpr size_t HISTORY_LEN = 5;
    float vel_history[HISTORY_LEN] = {0};
    size_t vel_idx = 0;
};
