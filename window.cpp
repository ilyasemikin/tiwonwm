#include "window.hpp"

#include "container.hpp"
#include "utils.hpp"

using namespace std;

Window::Window(xcb_connection_t *connection, xcb_window_t w_id) :
    connection_(connection),
    id_(w_id),
    x_(0),
    y_(0),
    width_(0),
    height_(0),
    in_focus(false),
    is_maximized(false)
{

}

string Window::ToString() const {
    return to_string(id_);
}

bool Window::IsCorrectSize(uint16_t width, uint16_t height) const {
    return width >= 20 && height >= 20;
}

void Window::Move(int16_t x, int16_t y) {
    if (x_ == x && y_ == y) {
        return;
    }

    x_ = x;
    y_ = y;

    uint32_t values[] {
        static_cast<uint32_t>(x_),
        static_cast<uint32_t>(y_)
    };

    xcb_configure_window(
        connection_,
        id_,
        XCB_CONFIG_WINDOW_X
      | XCB_CONFIG_WINDOW_Y,
        values
    );
}

void Window::Resize(uint16_t width, uint16_t height) {
    if (width_ == width && height_ == height) {
        return;
    }

    width_ = width;
    height_ = height;

    uint32_t values[] {
        static_cast<uint32_t>(width_ - 2 * border_width_),
        static_cast<uint32_t>(height_ - 2 * border_width_)
    };

    xcb_configure_window(
        connection_,
        id_,
        XCB_CONFIG_WINDOW_WIDTH
      | XCB_CONFIG_WINDOW_HEIGHT,
        values
    );
}

void Window::MoveResize(int16_t x, int16_t y, uint16_t width, uint16_t height) {
    Resize(width, height);
    Move(x, y);
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