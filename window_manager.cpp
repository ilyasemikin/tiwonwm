#include <cstdlib>
#include <iostream>
#include <iomanip>
#include "window_manager.hpp"
using namespace std;

window_manager::window_manager(xcb_connection_t *conn, int scr_num) : 
    connection(conn),
    screen_number(scr_num)    
{
    
}

window_manager::~window_manager() {
    xcb_disconnect(connection);
}

unique_ptr<window_manager> window_manager::create() {
    int screen_n = 0;
    auto connection = xcb_connect(nullptr, &screen_n);

    if (xcb_connection_has_error(connection)) {
        return nullptr;
    }

    return unique_ptr<window_manager>(new window_manager(connection, screen_n));
}

window_manager::result window_manager::run() {
    auto iter = xcb_setup_roots_iterator(xcb_get_setup(connection));

    for (size_t i = 0; i < screen_number; i++) {
        xcb_screen_next(&iter);
    }

    if (!substructure_redirect(iter.data->root)) {
        return { r_state::ERROR, "can't substructure redirect" };
    }

    event_loop();

    return { r_state::OK, "" };
}

bool window_manager::substructure_redirect(xcb_window_t root_window) {
    uint32_t mask = XCB_CW_EVENT_MASK;
    uint32_t values[] {
        XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT
      | XCB_EVENT_MASK_STRUCTURE_NOTIFY
      | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY
    };

    auto attr_cookie = xcb_change_window_attributes_checked(
        connection,
        root_window,
        mask,
        values
    );

    xcb_flush(connection);

    auto error = xcb_request_check(connection, attr_cookie);

    if (error != nullptr) {
        return false;
    }

    return true;
}

void window_manager::event_loop() {
    while (true) {
        auto event = xcb_poll_for_event(connection);

        if (event == nullptr) {
            continue;
        }

        auto event_type = event->response_type & (~0x80);
        cout << "-> event " << setw(3) << event_type << endl;

        free(event);
    }
}