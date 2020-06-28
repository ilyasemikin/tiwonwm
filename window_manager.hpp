#pragma once

#include <xcb/xcb.h>

#include <functional>
#include <memory>
#include <utility>
#include <vector>
#include <unordered_map>

#include "display.hpp"
#include "workspace.hpp"
#include "configs.hpp"

class WindowManager {
public:
    static std::unique_ptr<WindowManager> Create(const Config &config);

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
    WindowManager(xcb_connection_t *conn, int scr_n, const Config &config);

    bool SetUp();
    bool SetUpKeys();

    void EventLoop();

    void ExecApplication(const std::string &program_name);

    // Events
    using event_handler = std::function<void(xcb_generic_event_t *)>;

    void OnKeyPress(xcb_generic_event_t *raw_event);
    void OnButtonPress(xcb_generic_event_t *raw_event);
    void OnButtonRelease(xcb_generic_event_t *raw_event);
    void OnEnterNotify(xcb_generic_event_t *raw_event);
    void OnConfigureRequest(xcb_generic_event_t *raw_event);
    void OnMapRequest(xcb_generic_event_t *raw_event);
    void OnUnmapNotify(xcb_generic_event_t *raw_event);

    void SetWorkspace(size_t ws_number);

    std::shared_ptr<Display> display_;

    int screen_number_;
    xcb_window_t root_window_;
    xcb_connection_t *connection_;

    Config config_;

    // Workspaces
    std::vector<Workspace> workspaces_;
    size_t current_ws_;

    // Keys
    std::unordered_map<xcb_keycode_t, size_t> ws_change_keys_;
    xcb_keycode_t terminal_open_key_;
};