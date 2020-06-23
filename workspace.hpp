#pragma once

#include <memory>
#include <string>
#include <vector>

#include <xcb/xcb.h>

#include "display.hpp"
#include "window.hpp"

class Workspace {
public:
    Workspace(xcb_connection_t *connection);

    void AddWindow(xcb_window_t w_id);
    void RemoveWindow(xcb_window_t w_id);

    bool Has(xcb_window_t w_id);

    // TODO: в дальнейшем стоит рассмотреть алтернативные варианты
    // передачи информации о экране
    void SetDisplay(std::shared_ptr<Display> display);
private:    
    xcb_connection_t *connection_;

    std::shared_ptr<Display> display_;

    std::vector<Window> windows_;
    typename std::vector<Window>::iterator active_window_;

    void ResizeWindows();

    const uint32_t kBorderColor;
    const uint32_t kBorderWidth;
};