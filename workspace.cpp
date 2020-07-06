#include "workspace.hpp"

#include <algorithm>

#include "container.hpp"

using namespace std;

Workspace::Workspace(xcb_connection_t *connection) :
    connection_(connection)
{
    t_orient_ = Orientation::HORIZONTAL;

    SetDefaultConfig();
}

void Workspace::InsertWindow(xcb_window_t w_id) {
    auto win = make_shared<Window>(connection_, w_id);

    // Устанавливаем цвет рамки окна
    xcb_change_window_attributes(
        connection_,
        win->GetId(),
        XCB_CW_BORDER_PIXEL,
        &config_.focused_border_color
    );

    win->SetBorderWidth(config_.border_width);

    // Подписываемся на события окна
    uint32_t value = XCB_EVENT_MASK_ENTER_WINDOW;
    xcb_change_window_attributes(
        connection_,
        win->GetId(),
        XCB_CW_EVENT_MASK,
        &value
    );
    
    // Добавляем окно в X Save Set, чтобы в случае падения оконного менеджера
    // окно автоматически было востановлено
    xcb_change_save_set(
        connection_,
        XCB_SET_MODE_INSERT,
        win->GetId()
    );

    // Размещаем окно
    win->Map();

    if (wins_tree_.Empty()) {
        wins_tree_.SetRoot(win);
    }
    else {
        wins_tree_.AddNeighbour(
            active_window_->GetId(),
            win,
            t_orient_
        );
    }

    SetFocus(win->GetId());

    ShowFrames();
}

void Workspace::RemoveWindow(xcb_window_t w_id) {
    if (!Contains(w_id)) {
        return;
    }
    
    auto win = wins_tree_.GetWindow(w_id);

    xcb_change_save_set(
        connection_,
        XCB_SET_MODE_DELETE,
        w_id
    );

    wins_tree_.Remove(win->GetId());

    // Если удаляемое окно последнее, то фокусируемся на предпоследнем
    if (active_window_->GetId() == win->GetId()) {
        active_window_ = nullptr;
        if (!wins_tree_.Empty()) {
            // TODO: установить фокус на некотором окне осмысленно, а не случайно
            SetFocus(wins_tree_.begin()->first);
        }
    }

    ShowFrames();
}

void Workspace::Show() {
    for (auto [id, window] : wins_tree_) {
        window->Map();
    }
    xcb_flush(connection_);
}

void Workspace::Hide() {
    for (auto [id, window] : wins_tree_) {
        window->Unmap();
    }
    xcb_flush(connection_);
}

void Workspace::SetFocus(xcb_window_t w_id) {
    if (active_window_ != nullptr && w_id == active_window_->GetId()) {
        return;
    }

    // Сбрасываем текущий фокус
    if (active_window_ != nullptr) {
        active_window_->Unfocus(config_.unfocused_border_color);
        active_window_ = nullptr;
    }

    if (!Contains(w_id)) {
        return;
    }

    active_window_ = wins_tree_.GetWindow(w_id);
    active_window_->Focus(config_.focused_border_color);

    xcb_flush(connection_);
}

void Workspace::RotateFocusFrame() {
    if (active_window_ == nullptr) {
        return;
    }

    wins_tree_.RotateFrameWithWindow(active_window_->GetId());

    ShowFrames();
}

void Workspace::ResizeWindow(Orientation orient, int16_t px) {
    if (active_window_ == nullptr) {
        return;
    }

    auto win = wins_tree_.GetWindow(active_window_->GetId());
    auto parent = wins_tree_.GetContainerWithWindow(active_window_->GetId());
    if (parent->GetOrientation() != orient) {
        // Изменение размера окна в рамках фрейма
        parent->ResizeChild(win, px);
    }
    else {
        // Изменение размера фрейма с активным окном
        // Соответственно находим первый фрейм с типом тайлинга, отличный от
        // того, в котором содержится активное окно
        auto node = parent;

        while (parent != nullptr && parent->GetOrientation() == orient) {
            node = parent;

            parent = dynamic_pointer_cast<Container>(parent->GetParent());
        }

        if (parent == nullptr) {
            return;
        }

        parent->ResizeChild(node, px);
    }

    xcb_flush(connection_);
}

bool Workspace::Contains(xcb_window_t w_id) {
    return wins_tree_.Contains(w_id);
}

void Workspace::ProcessEventByWindow(xcb_window_t w_id, xcb_generic_event_t *raw_event) {
    if (!Contains(w_id)) {
        return;
    }

    auto it = wins_tree_.GetWindow(w_id);
    
    if (raw_event->response_type & XCB_CONFIGURE_REQUEST) {
        // Ответ на CONFIGURE_REQUEST - игнорируем запрос
        // и отправляем собственный ответ, которым задаем нужные
        // нам параметры
        XCB_SendedNotifyEvent<xcb_configure_notify_event_t> e;
        auto &event = e.Get();

        event.event = it->GetId();
        event.window = it->GetId();
        event.response_type = XCB_CONFIGURE_NOTIFY;

        event.x = it->GetX();
        event.y = it->GetY();
        event.width = it->GetWidth();
        event.height = it->GetHeight();

        xcb_send_event(
            connection_,
            false,
            it->GetId(),
            XCB_EVENT_MASK_STRUCTURE_NOTIFY,
            e.GetAsCharArray()
        );

        xcb_flush(connection_);
    }
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

void Workspace::ShowFrames() {
    if (!wins_tree_.Empty()) {
        ShowFrame(
            wins_tree_.GetRoot(),
            0, 0,
            display_->width, display_->height
        );

        xcb_flush(connection_);
    }
}

void Workspace::ShowFrame(Frame::ptr node, int16_t x, int16_t y, uint32_t width, uint32_t height) {
    if (node->GetType() == FrameType::WINDOW) {
        auto win_node = dynamic_pointer_cast<Window>(node);

        win_node->MoveResize(x, y, width, height);

        return;
    }

    auto frame_node = dynamic_pointer_cast<Container>(node);
    auto c_count = frame_node->CountChilds();
    if (frame_node->GetOrientation() == Orientation::VERTICAL) {
        height /= c_count;

        for (size_t i = 0; i < c_count; i++) {
            ShowFrame(
                frame_node->GetChild(i),
                x, y,
                width, height
            );

            y += height;
        }
    }
    else {
        width /= c_count;

        for (size_t i = 0; i < c_count; i++) {
            ShowFrame(
                frame_node->GetChild(i),
                x, y,
                width, height
            );

            x += width;
        }
    }
}