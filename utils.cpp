#include "utils.hpp"

uint32_t GetColor(uint8_t r, uint8_t g, uint8_t b) {
    return (r << 16) | (g << 8) | b;
}