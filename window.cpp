#include "window.hpp"

Window::Window(xcb_connection_t *connection, xcb_window_t w_id) :
    connection_(connection),
    id_(w_id)
{

}

xcb_window_t Window::GetId() const {
    return id_;
}

void Window::Map() {
    xcb_map_window(connection_, id_);
}

void Window::Unmap() {
    xcb_unmap_window(connection_, id_);
}

void Window::MoveResize(int16_t x, int16_t y, uint16_t width, uint16_t height) {
    x_ = x;
    y_ = y;
    width_ = width;
    height_ = height;

    uint32_t values[] {
        static_cast<uint32_t>(x),
        static_cast<uint32_t>(y),
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };

    xcb_configure_window(
        connection_,
        id_,
        XCB_CONFIG_WINDOW_X
      | XCB_CONFIG_WINDOW_Y
      | XCB_CONFIG_WINDOW_WIDTH
      | XCB_CONFIG_WINDOW_HEIGHT,
        values
    );
}