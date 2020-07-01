#include "utils.hpp"

using namespace std;

string to_string(const Orientation &orient) {
    return orient == Orientation::HORIZONTAL ? "Horizontal" : "Vertical";
}

Orientation GetOtherOrientation(const Orientation &orient) {
    if (orient == Orientation::HORIZONTAL)
        return Orientation::VERTICAL;
    return Orientation::HORIZONTAL;
}

uint32_t GetColor(uint8_t r, uint8_t g, uint8_t b) {
    return (r << 16) | (g << 8) | b;
}