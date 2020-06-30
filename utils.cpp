#include "utils.hpp"

using namespace std;

string to_string(const Orientation &orient) {
    return orient == Orientation::HORIZONTAL ? "Horizontal" : "Vertical";
}

uint32_t GetColor(uint8_t r, uint8_t g, uint8_t b) {
    return (r << 16) | (g << 8) | b;
}