#pragma once

#include <xcb/xcb.h>

class Window {
public:
    Window(xcb_connection_t *connection, xcb_window_t w_id);

    xcb_window_t GetId() const;

    void Map();
    void Unmap();

    void MoveResize(int16_t x, int16_t y, uint16_t width, uint16_t height);
private:
    xcb_connection_t *connection_;

    xcb_window_t id_;

    int16_t x_;
    int16_t y_;
    uint16_t width_;
    uint16_t height_;
};