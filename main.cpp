#include <iostream>
#include "window_manager.hpp"
using namespace std;

int main() {
    auto wm = window_manager::create();

    if (wm == nullptr) {
        cerr << "Can't launch window manager" << endl;
        exit(EXIT_FAILURE);
    }

    auto wm_exit_state = wm->run();
    if (wm_exit_state.state == window_manager::r_state::ERROR) {
        cerr << "Error: " << wm_exit_state.message << endl;
    }

    return EXIT_SUCCESS;
}