#pragma once

#include <cstdint>
#include <string>

enum class Orientation {
    VERTICAL,
    HORIZONTAL
};

std::string to_string(const Orientation &orient);
Orientation GetOtherOrientation(const Orientation &orient);

enum class Direction {
    LEFT, RIGHT,
    UP, DOWN
};

uint32_t GetColor(uint8_t r, uint8_t g, uint8_t b);