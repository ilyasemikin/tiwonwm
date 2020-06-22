#pragma once

#include <xcb/xcb.h>

#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include "workspace.hpp"

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
    using event_handler = std::function<void(xcb_generic_event_t *)>;

    void OnConfigureRequest(xcb_generic_event_t *raw_event);
    void OnMapRequest(xcb_generic_event_t *raw_event);
    void OnUnmapNotify(xcb_generic_event_t *raw_event);

    int screen_number_;
    xcb_window_t root_window_;
    xcb_connection_t *connection_;

    // Workspaces
    std::vector<Workspace> workspaces_;
    size_t current_ws_;
};