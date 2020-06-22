#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <vector>
#include "window_manager.hpp"
using namespace std;

WindowManager::WindowManager(xcb_connection_t *conn, int scr_num) : 
    screen_number_(scr_num),
    connection_(conn)
{
    
}

WindowManager::~WindowManager() {
    xcb_disconnect(connection_);
}

unique_ptr<WindowManager> WindowManager::Create() {
    int screen_n = 0;
    auto connection = xcb_connect(nullptr, &screen_n);

    if (xcb_connection_has_error(connection)) {
        return nullptr;
    }

    return unique_ptr<WindowManager>(new WindowManager(connection, screen_n));
}

WindowManager::RunResult WindowManager::Run() {
    auto iter = xcb_setup_roots_iterator(xcb_get_setup(connection_));

    for (int i = 0; i < screen_number_; i++) {
        xcb_screen_next(&iter);
    }

    root_window_ = iter.data->root;

    if (!SubstructureRedirect()) {
        return { ResultState::ERROR, "can't substructure redirect" };
    }

    EventLoop();

    return { ResultState::OK, "" };
}

bool WindowManager::SubstructureRedirect() {
    uint32_t mask = XCB_CW_EVENT_MASK;
    uint32_t values[] {
        XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT
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

void WindowManager::EventLoop() {
    unordered_map<uint8_t, event_handler> events {
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
        cout << "-> event " << setw(3) << event_type << ": ";

        auto finded = events.find(event_type);
        if (finded != events.end()) {
            finded->second(event);
            
            cout << "processed" << endl;
        }
        else {
            cout <<  "not unknown" << endl;
        }

        free(event);
    }
}

// Events
// TODO: rewrite
void WindowManager::OnConfigureRequest(xcb_generic_event_t *raw_event) {
    auto event = reinterpret_cast<xcb_configure_request_event_t *>(raw_event);

    auto &window_id = event->window;
    
    if (w_clients_.count(window_id)) {
        // TODO: write
        return;
    }

    auto &mask = event->value_mask;
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

    if (w_clients_.count(window_id)) {
        // Окно уже отрисовано
        // TODO: Разобраться, как нужно поступить в таком случае
        // ничего не делаем
        return;
    }

    // Получаем позицию и размер окна
    auto geometry = xcb_get_geometry_reply(
        connection_,
        xcb_get_geometry(connection_, window_id),
        NULL
    );

    auto &client = w_clients_[window_id] = {};

    client.id = window_id;
    client.x = geometry->x;
    client.y = geometry->y;
    client.width = geometry->width;
    client.height = geometry->height;

    free(geometry);

    // Изменяем размеры и позицию окна
    // пробуем
    uint16_t mask = XCB_CONFIG_WINDOW_X
                  | XCB_CONFIG_WINDOW_Y
                  | XCB_CONFIG_WINDOW_WIDTH
                  | XCB_CONFIG_WINDOW_HEIGHT;
    uint32_t values[]{ 
        static_cast<uint32_t>(client.x),
        static_cast<uint32_t>(client.y),
        client.width,
        client.height
    };

    xcb_configure_window(
        connection_,
        window_id,
        mask,
        values
    );

    // Размещаем окно
    xcb_map_window(connection_, window_id);

    xcb_flush(connection_);
}

void WindowManager::OnUnmapNotify(xcb_generic_event_t *raw_event) {
    auto event = reinterpret_cast<xcb_unmap_notify_event_t *>(raw_event);

    auto &window_id = event->window;

    if (w_clients_.count(window_id)) {
        w_clients_.erase(window_id);
    }
}