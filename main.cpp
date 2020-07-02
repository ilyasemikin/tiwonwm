#include <iostream>

#include <X11/keysym.h>

#include "window_manager.hpp"
#include "utils.hpp"

using namespace std;

int main() {
    Config config;
    config.terminal = "xfce4-terminal";
    config.count_workspaces = 10;
    config.resize_px = 10;

    config.keys.open_terminal = XK_Return;
    config.keys.rotate_frame = XK_R;
    config.keys.switch_tiling = XK_S;
    config.keys.left = XK_Left;
    config.keys.right = XK_Right;
    config.keys.up = XK_Up;
    config.keys.down = XK_Down;
    config.keys.ws_change = {
        XK_1, XK_2, XK_3, XK_4, XK_5, XK_6, XK_7, XK_8, XK_9, XK_0
    };

    config.ws_config.border_width = 2;
    config.ws_config.unfocused_border_color = GetColor(255, 0, 0);
    config.ws_config.focused_border_color = GetColor(0, 0, 255);

    unique_ptr<WindowManager> wm = WindowManager::Create(config);

    if (wm == nullptr) {
        cerr << "Error: can't launch window manager" << endl;
        exit(EXIT_FAILURE);
    }

    WindowManager::RunResult wm_exit_state = wm->Run();
    if (wm_exit_state.state == WindowManager::ResultState::ERROR) {
        cerr << "Error: " << wm_exit_state.message << endl;
    }

    return EXIT_SUCCESS;
}