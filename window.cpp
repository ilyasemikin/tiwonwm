#include "window.hpp"

#include "utils.hpp"

using namespace std;

Window::Window(xcb_connection_t *connection, xcb_window_t w_id) :
    connection_(connection),
    id_(w_id),
    in_focus(false),
    is_maximized(false)
{

}

string Window::ToString() const {
    return to_string(id_);
}

#include <iostream>

void Window::MoveResize(int16_t x, int16_t y, uint16_t width, uint16_t height) {
    x_ = x;
    y_ = y;
    width_ = width;
    height_ = height;

    cout << id_ << " " << x << " " << y << " " << width << " " << height << endl;

    uint32_t values[] {
        static_cast<uint32_t>(x),
        static_cast<uint32_t>(y),
        static_cast<uint32_t>(width - 2 * border_width_),
        static_cast<uint32_t>(height - 2 * border_width_)
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

    xcb_flush(connection_);
}

void Window::SetBorderWidth(uint16_t border_width) {
    border_width_ = border_width;
    
    auto value = static_cast<uint32_t>(border_width_);
    xcb_configure_window(
        connection_,
        id_,
        XCB_CONFIG_WINDOW_BORDER_WIDTH,
        &value
    );
}

void Window::Map() {
    xcb_map_window(connection_, id_);
}

void Window::Unmap() {
    xcb_unmap_window(connection_, id_);
}

void Window::Focus(uint32_t color) {
    in_focus = true;

    // Утснавливаем цвет рамки
    xcb_change_window_attributes(
        connection_,
        id_,
        XCB_CW_BORDER_PIXEL,
        &color
    );

    // Устанавливаем курсор (ввод) на окно
    xcb_set_input_focus(
        connection_,
        XCB_INPUT_FOCUS_POINTER_ROOT,
        id_,
        XCB_CURRENT_TIME
    );
}

void Window::Unfocus(uint32_t color) {
    in_focus = false;

    // Устанавливаем цвет рамки
    xcb_change_window_attributes(
        connection_,
        id_,
        XCB_CW_BORDER_PIXEL,
        &color
    );
}