#include "window_manager.hpp"

#include <unistd.h>
#include <xcb/xcb_util.h>
#include <xcb/xcb_keysyms.h>
#include <X11/keysym.h>

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <vector>

#include "utils.hpp"

using namespace std;

WindowManager::WindowManager(xcb_connection_t *conn, int scr_num, const Config &config) : 
    screen_number_(scr_num),
    connection_(conn),
    config_(config),
    workspaces_(config_.count_workspaces, Workspace(conn)),
    current_ws_(0)
{
    for (auto &ws : workspaces_) {
        ws.SetConfig(config_.ws_config);
    }
}

WindowManager::~WindowManager() {
    xcb_disconnect(connection_);
}

unique_ptr<WindowManager> WindowManager::Create(const Config &config) {
    int screen_n = 0;
    auto connection = xcb_connect(nullptr, &screen_n);

    if (xcb_connection_has_error(connection)) {
        return nullptr;
    }

    return unique_ptr<WindowManager>(new WindowManager(connection, screen_n, config));
}

WindowManager::RunResult WindowManager::Run() {
    auto iter = xcb_setup_roots_iterator(xcb_get_setup(connection_));

    display_ = make_shared<Display>();

    for (int i = 0; i < screen_number_; i++) {
        xcb_screen_next(&iter);
    }

    auto &data = iter.data;
    root_window_ = data->root;
    display_->width = data->width_in_pixels;
    display_->height = data->height_in_pixels;

    for (auto &ws : workspaces_) {
        ws.SetDisplay(display_);
    }

    if (!SetUpKeys()) {
        return { ResultState::ERROR, "can't set up keys" };
    }

    if (!SetUp()) {
        return { ResultState::ERROR, "can't substructure redirect" };
    }

    EventLoop();

    return { ResultState::OK, "" };
}

bool WindowManager::SetUp() {
    uint32_t mask = XCB_CW_EVENT_MASK;
    uint32_t values[] {
        XCB_EVENT_MASK_BUTTON_PRESS
      | XCB_EVENT_MASK_BUTTON_RELEASE
      | XCB_EVENT_MASK_FOCUS_CHANGE
      | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT
      | XCB_EVENT_MASK_STRUCTURE_NOTIFY
      | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY
    };

    auto attr_cookie = xcb_change_window_attributes_checked(
        connection_,
        root_window_,
        mask,
        values
    );

    xcb_flush(connection_);

    auto error = xcb_request_check(connection_, attr_cookie);

    if (error != nullptr) {
        return false;
    }

    return true;
}

// TODO: в данный момент обработатывается клавиши путем перехвата
// нажатий всех комбинаций с кнопками. В дальнейшем сделать перехват
// определенных последовательной
bool WindowManager::SetUpKeys() {
    vector ws_change_keys {
        XK_1,
        XK_2,
        XK_3,
        XK_4,
        XK_5,
        XK_6,
        XK_7,
        XK_8,
        XK_9,
        XK_0
    };

    size_t ws_num = 0;
    auto key_symbs = xcb_key_symbols_alloc(connection_);

    auto super_l_key_code = xcb_key_symbols_get_keycode(key_symbs, XK_Super_L);

    xcb_grab_key(
        connection_,
        1,
        root_window_,
        XCB_MOD_MASK_ANY,
        *super_l_key_code,
        XCB_GRAB_MODE_ASYNC,
        XCB_GRAB_MODE_ASYNC
    );

    free(super_l_key_code);

    for (const auto &key : ws_change_keys) {
        auto key_code = xcb_key_symbols_get_keycode(key_symbs, key);

        ws_change_keys_.insert({ *key_code, ws_num++ });

        free(key_code);
    }

    auto terminal_open_key_code = xcb_key_symbols_get_keycode(key_symbs, XK_Return);
    terminal_open_key_ = *terminal_open_key_code;
    free(terminal_open_key_code);

    auto switch_tiling_key_code = xcb_key_symbols_get_keycode(key_symbs, XK_S);
    switch_tiling_key_ = *switch_tiling_key_code;
    free(switch_tiling_key_code);

    xcb_key_symbols_free(key_symbs);

    return true;
}

void WindowManager::EventLoop() {
    unordered_map<uint8_t, event_handler> events {
        { XCB_KEY_PRESS,            bind(&WindowManager::OnKeyPress, this, placeholders::_1) },
        { XCB_BUTTON_PRESS,         bind(&WindowManager::OnButtonPress, this, placeholders::_1) },
        { XCB_BUTTON_RELEASE,       bind(&WindowManager::OnButtonRelease, this, placeholders::_1) },
        { XCB_ENTER_NOTIFY,         bind(&WindowManager::OnEnterNotify, this, placeholders::_1 ) },
        { XCB_UNMAP_NOTIFY,         bind(&WindowManager::OnUnmapNotify, this, placeholders::_1) },
        { XCB_MAP_REQUEST,          bind(&WindowManager::OnMapRequest, this, placeholders::_1) },
        { XCB_CONFIGURE_REQUEST,    bind(&WindowManager::OnConfigureRequest, this, placeholders::_1) }
    };

    while (true) {
        auto event = xcb_poll_for_event(connection_);

        if (event == nullptr) {
            continue;
        }

        auto event_type = event->response_type & (~0x80);
        cout << "-> event " << setw(3) << event_type 
             << setw(20) << xcb_event_get_label(event_type)
             << ": ";

        auto finded = events.find(event_type);
        if (finded != events.end()) {
            cout << "processed" << endl;

            finded->second(event);
        }
        else {
            cout <<  "not processed" << endl;
        }

        free(event);
    }
}

void WindowManager::ExecApplication(const std::string &program_name) {
    auto pid = fork();

    if (pid == -1) {
        return;
    }
    else if (pid == 0) {
        char *const argv[] { nullptr };

        // Делаем процесс ведущим в группе
        // т.о. после падения оконного менеджера процесс будет жить дальше
        if (setsid() == -1) {
            exit(EXIT_FAILURE);
        }

        if (execvp(program_name.c_str(), argv) == -1) {
            exit(EXIT_FAILURE);
        }

        exit(EXIT_SUCCESS);
    }
}

// Events
void WindowManager::OnKeyPress(xcb_generic_event_t *raw_event) {
    auto event = reinterpret_cast<xcb_key_press_event_t *>(raw_event);

    if (ws_change_keys_.count(event->detail) && (event->state & XCB_MOD_MASK_4)) {
        SetWorkspace(ws_change_keys_[event->detail]);
    }
    else if (event->detail == terminal_open_key_ 
         && (event->state & XCB_MOD_MASK_4)
         && !config_.terminal.empty()) {
        ExecApplication(config_.terminal);
    }
    else if (event->detail == switch_tiling_key_ && (event->state & XCB_MOD_MASK_4)) {
        auto &cur_ws = workspaces_[current_ws_];
        cur_ws.SetTilingOrient(GetOtherOrientation(cur_ws.GetTilingOrient()));
    }
    else {
        // Передача комбинации в случае, если комбинацию мы не обрабатываем
        // FIXME: Заставить нормально работать
        xcb_send_event(
            connection_,
            false,
            XCB_SEND_EVENT_DEST_ITEM_FOCUS,
            XCB_EVENT_MASK_NO_EVENT,
            reinterpret_cast<char *>(event)
        );
        xcb_flush(connection_);
    }
}

void WindowManager::OnButtonPress(xcb_generic_event_t *) {

}

void WindowManager::OnButtonRelease(xcb_generic_event_t *) {

}

void WindowManager::OnEnterNotify(xcb_generic_event_t *raw_event) {
    auto event = reinterpret_cast<xcb_enter_notify_event_t *>(raw_event);

    if (event->mode == XCB_NOTIFY_MODE_NORMAL) {
        workspaces_[current_ws_].SetFocus(event->event);
    }
}

void WindowManager::OnConfigureRequest(xcb_generic_event_t *raw_event) {
    auto event = reinterpret_cast<xcb_configure_request_event_t *>(raw_event);

    auto &mask = event->value_mask;
    auto &window_id = event->window;
    
    if (workspaces_[current_ws_].Has(window_id)) {
        // TODO: в дальнейшем продумать, как действовать в данном случае.
        // т.е. решить проблему с приложениями, которые требуют иной конфигурации
        // отличной от той, которую задаем мы 
        uint16_t width = 0;
        uint16_t height = 0;

        if (event->value_mask & XCB_CONFIG_WINDOW_WIDTH) {
            width = event->width;
        }
        if (event->value_mask & XCB_CONFIG_WINDOW_HEIGHT) {
            height = event->height;
        }

        if (width || height) {
            uint32_t values[] {
                width,
                height
            };

            xcb_configure_window(
                connection_,
                window_id,
                XCB_CONFIG_WINDOW_WIDTH
              | XCB_CONFIG_WINDOW_HEIGHT,
                values
            );

            xcb_flush(connection_);
        }

        return;
    }

    vector<uint32_t> values;
    values.reserve(7);

    if (mask & XCB_CONFIG_WINDOW_X) {
        values.push_back(event->x);
    }
    if (mask & XCB_CONFIG_WINDOW_Y) {
        values.push_back(event->y);
    }
    if (mask & XCB_CONFIG_WINDOW_WIDTH) {
        values.push_back(event->width);
    }
    if (mask & XCB_CONFIG_WINDOW_HEIGHT) {
        values.push_back(event->height);
    }
    if (mask & XCB_CONFIG_WINDOW_SIBLING) {
        values.push_back(event->sibling);
    }
    if (mask & XCB_CONFIG_WINDOW_STACK_MODE) {
        values.push_back(event->stack_mode);
    }

    if (values.size()) {
        xcb_configure_window(
            connection_,
            window_id,
            mask,
            values.data()
        );
        xcb_flush(connection_);
    }
}

void WindowManager::OnMapRequest(xcb_generic_event_t *raw_event) {
    auto event = reinterpret_cast<xcb_map_request_event_t *>(raw_event);

    auto window_id = event->window;

    if (workspaces_[current_ws_].Has(window_id)) {
        // Окно уже отрисовано
        // TODO: Разобраться, как нужно поступить в таком случае
        // ничего не делаем
        return;
    }

    workspaces_[current_ws_].InsertWindow(window_id);
}

void WindowManager::OnUnmapNotify(xcb_generic_event_t *raw_event) {
    auto event = reinterpret_cast<xcb_unmap_notify_event_t *>(raw_event);

    auto &window_id = event->window;

    if (!workspaces_[current_ws_].Has(window_id)) {
        return;
    }

    workspaces_[current_ws_].RemoveWindow(window_id);
}

void WindowManager::SetWorkspace(size_t ws_number) {
    if (current_ws_ == ws_number) {
        return;
    }
    
    workspaces_[current_ws_].Hide();

    current_ws_ = ws_number;

    workspaces_[current_ws_].Show();
}