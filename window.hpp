#pragma once

#include <xcb/xcb.h>

class Window {
public:
    Window(xcb_connection_t *connection, xcb_window_t w_id);

    // TODO: в дальнейшем подумать, нужно ли выносить идентификатор вне класса
    inline xcb_window_t GetId() const {
        return id_;
    }

    inline bool InFocus() const {
        return in_focus;
    }

    inline bool IsMaximized() const {
        return is_maximized;
    }

    inline void SetX(int16_t x) {
        x_ = x;
    }

    inline int16_t GetX() {
        return x_;
    }

    inline void SetY(int16_t y) {
        y_ = y;
    }

    inline int16_t GetY() {
        return y_;
    }

    inline void SetWidth(uint32_t width) {
        width_ = width;
    }

    inline uint32_t GetWidth() const {
        return width_;
    }

    inline void SetHeight(uint32_t height) {
        height_ = height;
    }

    inline uint32_t GetHeight() {
        return height_;
    }

    void Map();
    void Unmap();

    void Focus(uint32_t color);
    void Unfocus(uint32_t color);

    void MoveResize(int16_t x, int16_t y, uint16_t width, uint16_t height);
private:
    xcb_connection_t *connection_;

    xcb_window_t id_;

    int16_t x_;
    int16_t y_;
    uint16_t width_;
    uint16_t height_;

    bool in_focus;
    bool is_maximized;
};