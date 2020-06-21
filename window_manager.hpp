#pragma once

#include <memory>
#include <utility>
#include <unordered_map>
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

    // Events
    // TODO: rewrite
    void on_configure_request(xcb_generic_event_t *raw_event);
    void on_map_request(xcb_generic_event_t *raw_event);
    void on_unmap_notify(xcb_generic_event_t *raw_event);

    int screen_number;
    xcb_connection_t *connection;

    // Clients
    // TODO: rewrite
    struct w_client {
        xcb_window_t id;
    };

    std::unordered_map<uint8_t, w_client> w_clients;
};