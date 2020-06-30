#pragma once

#include <cstdint>
#include <string>

enum class Orientation {
    VERTICAL,
    HORIZONTAL
};

enum class Direction {
    LEFT, RIGHT,
    UP, DOWN
};

std::string to_string(const Orientation &orient);

uint32_t GetColor(uint8_t r, uint8_t g, uint8_t b);