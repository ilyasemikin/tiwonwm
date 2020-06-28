#pragma once

#include <xcb/xcb.h>

#include <memory>
#include <string>
#include <list>

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

    void SetFocus(xcb_window_t w_id);

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

    std::list<Window> windows_;
    using window_iterator = typename std::list<Window>::iterator;
    window_iterator active_window_;
    
    window_iterator FindWindow(xcb_window_t w_id);

    void ResizeWindows();
};