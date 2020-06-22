#include <iostream>
#include "window_manager.hpp"
using namespace std;

int main() {
    unique_ptr<WindowManager> wm = WindowManager::Create();

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