#include "workspace.hpp"

#include <algorithm>

#include "utils.hpp"

using namespace std;

Workspace::Workspace(xcb_connection_t *connection) :
    connection_(connection)
{
    SetDefaultConfig();
}

void Workspace::AddWindow(xcb_window_t w_id) {
    windows_.push_back({ w_id });

    // Устанавливаем цвет рамки окна
    xcb_change_window_attributes_checked(
        connection_,
        w_id,
        XCB_CW_BORDER_PIXEL,
        &config_.border_color
    );

    uint32_t value = static_cast<uint32_t>(config_.border_width);
    // Устанавливаем ширину рамок окна
    xcb_configure_window(
        connection_,
        w_id,
        XCB_CONFIG_WINDOW_BORDER_WIDTH,
        &value
    );

    // Добавляем окно в X Save Set, чтобы в случае падения оконного менеджера
    // окно автоматически было востановлено
    xcb_change_save_set(
        connection_,
        XCB_SET_MODE_INSERT,
        w_id
    );

    // Размещаем окно
    xcb_map_window(connection_, w_id);

    ResizeWindows();
}

void Workspace::RemoveWindow(xcb_window_t w_id) {
    auto it = find_if(begin(windows_), end(windows_), [w_id](const auto &x) {
        return x.id == w_id;
    });

    if (it == end(windows_)) {
        return;
    }

    xcb_change_save_set(
        connection_,
        XCB_SET_MODE_DELETE,
        w_id
    );

    windows_.erase(it);

    ResizeWindows();
}

bool Workspace::Has(xcb_window_t w_id) {
    return find_if(
        begin(windows_),
        end(windows_),
        [w_id](const auto &x) {
            return x.id == w_id;
        }
    ) != end(windows_);
}

void Workspace::SetDisplay(shared_ptr<Display> display) {
    display_ = move(display);
}

void Workspace::SetConfig(const WorkspaceConfig &config) {
    config_ = config;
}

void Workspace::SetDefaultConfig() {
    config_.border_width = 4;
    config_.border_color = GetColor(0xFF, 0xFF, 0xFF);
}

// FIXME: при определенном количестве окон на экране появляется
// полоса в пару пикселей справа
void Workspace::ResizeWindows() {
    auto count_windows = windows_.size();

    if (count_windows == 0) {
        return;
    }

    uint32_t width_per_win = display_->width / count_windows;
    uint32_t height_per_win = display_->height;

    uint32_t values[4] = {
        0, 0,
        width_per_win - 2 * config_.border_width,
        height_per_win - 2 * config_.border_width
    };
    
    size_t x = 0;

    for (auto &window : windows_) {
        values[0] = x;

        xcb_configure_window(
            connection_,
            window.id,
            XCB_CONFIG_WINDOW_X
          | XCB_CONFIG_WINDOW_Y
          | XCB_CONFIG_WINDOW_WIDTH
          | XCB_CONFIG_WINDOW_HEIGHT,
            values
        );

        x += width_per_win;
    }

    xcb_flush(connection_);
}