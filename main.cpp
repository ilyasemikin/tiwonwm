#include <iostream>

#include "window_manager.hpp"
#include "utils.hpp"

using namespace std;

int main() {
    Config config;
    config.terminal = "xfce4-terminal";
    config.count_workspaces = 10;
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