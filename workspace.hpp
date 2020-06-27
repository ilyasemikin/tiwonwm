#pragma once

#include <xcb/xcb.h>

#include <memory>
#include <string>
#include <vector>

#include "display.hpp"
#include "window.hpp"
#include "configs.hpp"

class Workspace {
public:
    Workspace(xcb_connection_t *connection);

    void AddWindow(xcb_window_t w_id);
    void RemoveWindow(xcb_window_t w_id);

    void Show();
    void Hide();

    bool Has(xcb_window_t w_id);

    // TODO: в дальнейшем стоит рассмотреть алтернативные варианты
    // передачи информации о экране
    void SetDisplay(std::shared_ptr<Display> display);

    void SetConfig(const WorkspaceConfig &config);
    void SetDefaultConfig();
private:    
    xcb_connection_t *connection_;

    std::shared_ptr<Display> display_;

    WorkspaceConfig config_;

    std::vector<Window> windows_;
    typename std::vector<Window>::iterator active_window_;

    typename std::vector<Window>::iterator FindWindow(xcb_window_t w_id);

    void ResizeWindows();
};