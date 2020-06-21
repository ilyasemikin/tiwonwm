#pragma once

#include <memory>
#include <utility>
#include <xcb/xcb.h>

class window_manager {
public:
    static std::unique_ptr<window_manager> create();

    ~window_manager();

    enum class r_state {
        OK,
        ERROR
    };

    struct result {
        r_state state;
        std::string message;
    };

    result run();
private:
    window_manager(xcb_connection_t *conn, int scr_n);

    bool substructure_redirect(xcb_window_t root_window);
    void event_loop();

    int screen_number;
    xcb_connection_t *connection;
};