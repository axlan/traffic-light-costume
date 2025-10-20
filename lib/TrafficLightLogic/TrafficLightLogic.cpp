#include "TrafficLightLogic.h"

LightMode TrafficLight::Update(float vel)
{
    vel_history[vel_idx++] = vel;
    vel_idx %= HISTORY_LEN;

    float vel_mean = 0;
    for (size_t i = 0; i < HISTORY_LEN; i++)
    {
        vel_mean += vel_history[i];
    }
    vel_mean /= float(HISTORY_LEN);

    if (vel_mean > GREEN_THRESHOLD)
    {
        return LightMode::GREEN;
    }
    else if (vel_mean > YELLOW_THRESHOLD)
    {

        return LightMode::YELLOW;
    }
    else
    {

        return LightMode::RED;
    }
}
