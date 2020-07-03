#pragma once

#include <xcb/xcb.h>

#include "frame.hpp"

class Window : public Frame {
public:
    Window(xcb_connection_t *connection, xcb_window_t w_id);

    inline FrameType GetType() const override {
        return FrameType::WINDOW;
    }

    std::string ToString() const override;

    inline int16_t GetX() const override {
        return x_;
    }

    int16_t GetY() const override {
        return y_;
    }
    
    uint16_t GetWidth() const override {
        return width_;
    }

    uint16_t GetHeight() const override {
        return height_;
    }

    void Move(int16_t x, int16_t y) override;
    void Resize(uint16_t width, uint16_t height) override;
    void MoveResize(int16_t x, int16_t y, uint16_t width, uint16_t height) override;

    inline xcb_window_t GetId() const {
        return id_;
    }

    inline bool InFocus() const {
        return in_focus;
    }

    inline bool IsMaximized() const {
        return is_maximized;
    }

    void SetBorderWidth(uint16_t border_width);

    void Map();
    void Unmap();

    void Focus(uint32_t color);
    void Unfocus(uint32_t color);
private:
    xcb_connection_t *connection_;

    xcb_window_t id_;

    int16_t x_;
    int16_t y_;
    uint16_t width_;
    uint16_t height_;

    uint16_t border_width_;

    bool in_focus;
    bool is_maximized;
};