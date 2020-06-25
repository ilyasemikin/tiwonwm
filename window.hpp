#pragma once

#include <xcb/xcb.h>

struct Window {
    xcb_window_t id;

    Window(xcb_window_t w_id) :
        id(w_id)
    {

    }
};