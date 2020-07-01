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

// TODO: добавить обработку ошибок
pair<bool, xcb_keycode_t> GetKeyCode(xcb_key_symbols_t *key_symb, xcb_keysym_t key) {
    auto key_code = xcb_key_symbols_get_keycode(key_symb, key);

    auto ret = *key_code;
    free(key_code);

    return { true, ret };
}