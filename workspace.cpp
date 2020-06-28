#include "workspace.hpp"

#include <algorithm>

#include "utils.hpp"

using namespace std;

Workspace::Workspace(xcb_connection_t *connection) :
    connection_(connection)
{
    active_window_ = end(windows_);

    SetDefaultConfig();
}

void Workspace::AddWindow(xcb_window_t w_id) {
    windows_.push_back({ connection_, w_id });
    auto it = prev(end(windows_));

    // Устанавливаем цвет рамки для окна, бывщего до этого в фокусе
    if (active_window_ != end(windows_)) {
        xcb_change_window_attributes(
            connection_,
            active_window_->GetId(),
            XCB_CW_BORDER_PIXEL,
            &config_.unfocused_border_color
        );
    }
    active_window_ = it;

    // Устанавливаем цвет рамки окна
    xcb_change_window_attributes(
        connection_,
        it->GetId(),
        XCB_CW_BORDER_PIXEL,
        &config_.focused_border_color
    );

    uint32_t value = static_cast<uint32_t>(config_.border_width);
    // Устанавливаем ширину рамок окна
    xcb_configure_window(
        connection_,
        it->GetId(),
        XCB_CONFIG_WINDOW_BORDER_WIDTH,
        &value
    );

    // Подписываемся на события окна
    value = XCB_EVENT_MASK_ENTER_WINDOW;
    xcb_change_window_attributes(
        connection_,
        it->GetId(),
        XCB_CW_EVENT_MASK,
        &value
    );
    

    // Добавляем окно в X Save Set, чтобы в случае падения оконного менеджера
    // окно автоматически было востановлено
    xcb_change_save_set(
        connection_,
        XCB_SET_MODE_INSERT,
        it->GetId()
    );

    // Размещаем окно
    it->Map();

    ResizeWindows();
}

void Workspace::RemoveWindow(xcb_window_t w_id) {
    auto it = FindWindow(w_id);

    if (it == end(windows_)) {
        return;
    }

    xcb_change_save_set(
        connection_,
        XCB_SET_MODE_DELETE,
        w_id
    );

    // Если удаляемое окно последнее, то фокусируемся на предпоследнем
    if (active_window_ == it) {
        windows_.erase(it);

        active_window_ = end(windows_);
        if (!windows_.empty()) {
            active_window_ = prev(active_window_);

            xcb_change_window_attributes(
                connection_,
                active_window_->GetId(),
                XCB_CW_BORDER_PIXEL,
                &config_.focused_border_color
            );

            xcb_set_input_focus(
                connection_,
                XCB_INPUT_FOCUS_POINTER_ROOT,
                active_window_->GetId(),
                XCB_CURRENT_TIME
            );
        }
    }
    else {
        windows_.erase(it);
    }

    ResizeWindows();
}

void Workspace::Show() {
    for (auto &window : windows_) {
        window.Map();
    }
    xcb_flush(connection_);
}

void Workspace::Hide() {
    for (auto &window : windows_) {
        window.Unmap();
    }
    xcb_flush(connection_);
}

void Workspace::SetFocus(xcb_window_t w_id) {
    if (w_id == active_window_->GetId()) {
        return;
    }

    // Сбрасываем текущий фокус
    if (active_window_ != end(windows_)) {
        xcb_change_window_attributes(
            connection_,
            active_window_->GetId(),
            XCB_CW_BORDER_PIXEL,
            &config_.unfocused_border_color
        );
    }

    active_window_ = FindWindow(w_id);

    if (active_window_ == end(windows_)) {
        return;
    }

    xcb_change_window_attributes(
        connection_,
        active_window_->GetId(),
        XCB_CW_BORDER_PIXEL,
        &config_.focused_border_color
    );

    xcb_set_input_focus(
        connection_,
        XCB_INPUT_FOCUS_POINTER_ROOT,
        active_window_->GetId(),
        XCB_CURRENT_TIME
    );

    xcb_flush(connection_);
}

bool Workspace::Has(xcb_window_t w_id) {
    return FindWindow(w_id) != end(windows_);
}

void Workspace::SetDisplay(shared_ptr<Display> display) {
    display_ = move(display);
}

void Workspace::SetConfig(const WorkspaceConfig &config) {
    config_ = config;
}

void Workspace::SetDefaultConfig() {
    config_.border_width = 4;
    config_.unfocused_border_color = GetColor(0xFF, 0xFF, 0xFF);
    config_.focused_border_color = GetColor(0, 0, 0);
}

Workspace::window_iterator Workspace::FindWindow(xcb_window_t w_id) {
    return find_if(begin(windows_), end(windows_), [w_id](const auto &x) {
        return x.GetId() == w_id;
    });
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

    size_t x = 0;

    for (auto &window : windows_) {
        window.MoveResize(
            x, 0,
            width_per_win - 2 * config_.border_width,
            height_per_win - 2 * config_.border_width
        );

        x += width_per_win;
    }

    xcb_flush(connection_);
}