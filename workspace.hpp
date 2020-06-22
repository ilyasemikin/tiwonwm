#pragma once

#include <string>
#include <vector>

#include <xcb/xcb.h>

class Workspace {
public:
    Workspace(xcb_connection_t *connection);

    void AddWindow(xcb_window_t w_id);
    void RemoveWindow(xcb_window_t w_id);

    bool Has(xcb_window_t w_id);
private:    
    xcb_connection_t *connection_;

    std::vector<xcb_window_t> windows_;
    xcb_window_t active_window_;
};