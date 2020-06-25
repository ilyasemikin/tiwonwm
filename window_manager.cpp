#include "window_manager.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <vector>

using namespace std;

WindowManager::WindowManager(xcb_connection_t *conn, int scr_num) : 
    screen_number_(scr_num),
    connection_(conn),
    // TODO: не использовать константу, получать число извне
    workspaces_(9, Workspace(conn)),
    current_ws_(0)
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

// TODO: По завершении разработки убрать
// используется исключительно для понимания работы
#define EVENT_WITH_NAME(x) { x, #x }

void WindowManager::EventLoop() {
    unordered_map<uint8_t, event_handler> events {
        { XCB_BUTTON_PRESS,         bind(&WindowManager::OnButtonPress, this, placeholders::_1) },
        { XCB_BUTTON_RELEASE,       bind(&WindowManager::OnButtonRelease, this, placeholders::_1) },
        { XCB_UNMAP_NOTIFY,         bind(&WindowManager::OnUnmapNotify, this, placeholders::_1) },
        { XCB_MAP_REQUEST,          bind(&WindowManager::OnMapRequest, this, placeholders::_1) },
        { XCB_CONFIGURE_REQUEST,    bind(&WindowManager::OnConfigureRequest, this, placeholders::_1) }
    };

    unordered_map<uint8_t, string> events_names {
        EVENT_WITH_NAME(XCB_BUTTON_PRESS),
        EVENT_WITH_NAME(XCB_BUTTON_RELEASE),
        EVENT_WITH_NAME(XCB_CREATE_NOTIFY),
        EVENT_WITH_NAME(XCB_DESTROY_NOTIFY),
        EVENT_WITH_NAME(XCB_UNMAP_NOTIFY),
        EVENT_WITH_NAME(XCB_MAP_NOTIFY),
        EVENT_WITH_NAME(XCB_MAP_REQUEST),
        EVENT_WITH_NAME(XCB_CONFIGURE_NOTIFY),
        EVENT_WITH_NAME(XCB_CONFIGURE_REQUEST),
        EVENT_WITH_NAME(XCB_CLIENT_MESSAGE),
        EVENT_WITH_NAME(XCB_MAPPING_NOTIFY)
    };

    while (true) {
        auto event = xcb_poll_for_event(connection_);

        if (event == nullptr) {
            continue;
        }

        auto event_type = event->response_type & (~0x80);
        cout << "-> event " << setw(3) << event_type 
             << setw(30) << (events_names.count(event_type) ? events_names[event_type] : "UNKNOWN NAME")
             << ": ";

        auto finded = events.find(event_type);
        if (finded != events.end()) {
            finded->second(event);
            
            cout << "processed" << endl;
        }
        else {
            cout <<  "not processed" << endl;
        }

        free(event);
    }
}

// Events
void WindowManager::OnButtonPress(xcb_generic_event_t *) {

}

void WindowManager::OnButtonRelease(xcb_generic_event_t *) {

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

    workspaces_[current_ws_].AddWindow(window_id);
}

void WindowManager::OnUnmapNotify(xcb_generic_event_t *raw_event) {
    auto event = reinterpret_cast<xcb_unmap_notify_event_t *>(raw_event);

    auto &window_id = event->window;

    if (!workspaces_[current_ws_].Has(window_id)) {
        return;
    }

    workspaces_[current_ws_].RemoveWindow(window_id);
}