#include <iostream>

#include "window_manager.hpp"
#include "utils.hpp"
// На время тестирования класса Tree
// TODO: убрать по окочанию разработки класса
#include "tree.hpp"

using namespace std;

int main() {
    // Блок тестирования класса Tree
    // TODO: убрать по окочанию разработки класса
    Tree tree;

    tree.Add(0, TilingOrientation::HORIZONTAL);
    tree.Add(1, TilingOrientation::HORIZONTAL);

    tree.AddNeighbour(0, 2, TilingOrientation::VERTICAL);
    tree.AddNeighbour(0, 3, TilingOrientation::VERTICAL);

    tree.AddNeighbour(2, 4, TilingOrientation::HORIZONTAL);

    tree.Remove(4);
    tree.Remove(0);

    cout << tree.GetStructureString() << endl;

    return EXIT_SUCCESS;

    // До тех пор пока до конца не доделан класс Tree
    // непосредственный запуск оконного менеджера не требуется    
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