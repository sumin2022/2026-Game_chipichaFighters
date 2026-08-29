#pragma once

#include <algorithm>

inline float Normalize01(
    float value,
    float minValue,
    float maxValue)
{
    if (maxValue <= minValue)
        return 0.0f;

    float result =
        (value - minValue) /
        (maxValue - minValue);

    return std::clamp(result, 0.0f, 1.0f);
}

inline float NormalizeMinusOneToOne(
    float value,
    float minValue,
    float maxValue)
{
    return Normalize01(
        value,
        minValue,
        maxValue) * 2.0f - 1.0f;
}