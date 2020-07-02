#include "workspace.hpp"

#include <algorithm>

#include "container.hpp"

using namespace std;

Workspace::Workspace(xcb_connection_t *connection) :
    connection_(connection),
    wins_tree_(connection_)
{
    active_window_.exist = false;

    t_orient_ = Orientation::HORIZONTAL;

    SetDefaultConfig();
}

void Workspace::InsertWindow(xcb_window_t w_id) {
    auto it = make_shared<Window>(connection_, w_id);

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
        wins_tree_.Add(it->GetId());
    }
    else {
        wins_tree_.AddNeighbour(
            active_window_.id,
            it->GetId(),
            t_orient_
        );
    }

    SetFocus(it->GetId());

    ResizeWindows();
}

void Workspace::RemoveWindow(xcb_window_t w_id) {
    if (!Contains(w_id)) {
        return;
    }
    
    auto it = wins_tree_.GetWindow(w_id);

    xcb_change_save_set(
        connection_,
        XCB_SET_MODE_DELETE,
        w_id
    );

    wins_tree_.Remove(it->GetId());

    // Если удаляемое окно последнее, то фокусируемся на предпоследнем
    if (active_window_.id == it->GetId()) {
        active_window_.exist = false;
        if (!wins_tree_.Empty()) {
            // TODO: установить фокус на некотором окне
        }
    }

    ResizeWindows();
}

// TODO: избавиться от кастов
void Workspace::Show() {
    for (auto [id, frame] : wins_tree_) {
        auto window = dynamic_pointer_cast<Window>(frame);
        window->Map();
    }
    xcb_flush(connection_);
}

// TODO: избавиться от кастов
void Workspace::Hide() {
    for (auto [id, frame] : wins_tree_) {
        auto window = dynamic_pointer_cast<Window>(frame);
        window->Unmap();
    }
    xcb_flush(connection_);
}

void Workspace::SetFocus(xcb_window_t w_id) {
    if (active_window_.exist && w_id == active_window_.id) {
        return;
    }

    // Сбрасываем текущий фокус
    if (active_window_.exist) {
        active_window_.exist = false;
        wins_tree_.GetWindow(w_id)->Unfocus(config_.unfocused_border_color);
    }

    if (!Contains(w_id)) {
        return;
    }

    active_window_.id = w_id;
    active_window_.exist = true;
    wins_tree_.GetWindow(w_id)->Focus(config_.focused_border_color);

    xcb_flush(connection_);
}

void Workspace::RotateFocusFrame() {
    if (!active_window_.exist) {
        return;
    }

    wins_tree_.RotateFrameWithWindow(active_window_.id);

    ResizeWindows();
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

// FIXME: при определенном количестве окон на экране появляется
// полоса в пару пикселей справа
void Workspace::ResizeWindows() {
    if (!wins_tree_.Empty()) {
        ShowFrames(
            wins_tree_.GetStructure(),
            0, 0,
            display_->width, display_->height
        );

        xcb_flush(connection_);
    }
}

void Workspace::ShowFrames(Frame::ptr node, int16_t x, int16_t y, uint32_t width, uint32_t height) {
    if (node->GetType() == FrameType::WINDOW) {
        auto win_node = dynamic_pointer_cast<Window>(node);

        win_node->MoveResize(
            x, y,
            width - 2 * config_.border_width,
            height - 2 * config_.border_width
        );

        return;
    }

    auto frame_node = dynamic_pointer_cast<Container>(node);
    auto c_count = frame_node->CountChilds();
    if (frame_node->GetOrientation() == Orientation::VERTICAL) {
        height /= c_count;

        for (size_t i = 0; i < c_count; i++) {
            ShowFrames(
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
            ShowFrames(
                frame_node->GetChild(i),
                x, y,
                width, height
            );

            x += width;
        }
    }
}