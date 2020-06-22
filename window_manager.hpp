#pragma once

#include <functional>
#include <memory>
#include <utility>
#include <unordered_map>
#include <xcb/xcb.h>

class WindowManager {
public:
    static std::unique_ptr<WindowManager> Create();

    ~WindowManager();

    enum class ResultState {
        OK,
        ERROR
    };

    struct RunResult {
        ResultState state;
        std::string message;
    };

    RunResult Run();
private:
    WindowManager(xcb_connection_t *conn, int scr_n);

    bool SubstructureRedirect();
    void EventLoop();

    // Events
    // TODO: rewrite
    using event_handler = std::function<void(xcb_generic_event_t *)>;

    void OnConfigureRequest(xcb_generic_event_t *raw_event);
    void OnMapRequest(xcb_generic_event_t *raw_event);
    void OnUnmapNotify(xcb_generic_event_t *raw_event);

    int screen_number_;
    xcb_window_t root_window_;
    xcb_connection_t *connection_;

    // Clients
    // TODO: rewrite
    struct w_client {
        xcb_window_t id;
        int16_t x, y;
        uint16_t width, height;
    };

    std::unordered_map<uint8_t, w_client> w_clients_;
};