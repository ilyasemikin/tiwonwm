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

void Workspace::InsertWindow(xcb_window_t w_id) {
    windows_.push_back({ connection_, w_id });
    auto it = prev(end(windows_));

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

    if (wins_tree_.Empty()) {
        wins_tree_.Add(it->GetId(), TilingOrientation::VERTICAL);
    }
    else {
        wins_tree_.AddNeighbour(
            active_window_->GetId(),
            it->GetId(),
            tiling_orient_
        );
    }

    SetFocus(it->GetId());

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

    wins_tree_.Remove(it->GetId());

    // Если удаляемое окно последнее, то фокусируемся на предпоследнем
    if (active_window_ == it) {
        windows_.erase(it);

        active_window_ = end(windows_);
        if (!windows_.empty()) {
            SetFocus(windows_.back().GetId());
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
        active_window_->Unfocus(config_.unfocused_border_color);
    }

    active_window_ = FindWindow(w_id);
    if (active_window_ == end(windows_)) {
        return;
    }

    active_window_->Focus(config_.focused_border_color);

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
    auto main_frame = wins_tree_.GetStructure();

    if (!main_frame.IsEmpty()) {
        ShowFrames(
            main_frame,
            0, 0,
            display_->width, display_->height
        );

        xcb_flush(connection_);
    }
}

// В данный момент имеется проблема - не учитывается рамка, создаваемая оконным менеджер
// TODO: доработать
void Workspace::ShowFrames(const Tree::Frame &frame, int16_t x, int16_t y, uint32_t width, uint32_t height) {
    if (frame.IsWindow()) {
        auto it = FindWindow(frame.GetWindowId());

        it->MoveResize(
            x, y,
            width, height
        );

        return;
    }

    auto c_count = frame.ChildsCount();
    if (frame.IsVerticalFrame()) {
        height /= c_count;

        for (size_t i = 0; i < c_count; i++) {
            ShowFrames(
                frame.GetChild(i),
                x, y,
                width, height
            );

            y += height;
        }
    }
    else {
        width /= c_count;

        for (size_t i = 0; i < c_count; i++) {
            ShowFrames(
                frame.GetChild(i),
                x, y,
                width, height
            );

            x += width;
        }
    }
}