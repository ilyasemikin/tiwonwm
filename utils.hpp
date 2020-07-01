#pragma once

#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>

#include <cstdint>
#include <string>
#include <utility>

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

std::pair<bool, xcb_keycode_t> GetKeyCode(xcb_key_symbols_t *key_symb, xcb_keysym_t key);

template <typename EventType>
class XCB_SendedNotifyEvent {
public:
    XCB_SendedNotifyEvent() {
        // Так как X11 всегда принимает фиксированное количество 
        // байт для оповещений - 32, выделяем необходимую память
        raw_memory_ = new uint8_t[32];
        event_ = reinterpret_cast<EventType *>(raw_memory_);
    }

    ~XCB_SendedNotifyEvent() {
        delete[] raw_memory_;
    }

    EventType &Get() {
        return *event_;
    }

    const char *GetAsCharArray() {
        return reinterpret_cast<const char *>(event_);
    }
private:
    uint8_t *raw_memory_;
    EventType *event_;
};