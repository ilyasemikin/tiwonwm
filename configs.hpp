#pragma once

#include <xcb/xcb.h>

#include <cstdint>
#include <string>
#include <vector>

struct WorkspaceConfig {
    uint8_t border_width;
    uint32_t focused_border_color;
    uint32_t unfocused_border_color;
};

struct KeysConfig {
    std::vector<xcb_keysym_t> ws_change;
    xcb_keysym_t open_terminal;
    xcb_keysym_t switch_tiling;
    xcb_keysym_t rotate_frame;
    xcb_keysym_t left;
    xcb_keysym_t right;
    xcb_keysym_t up;
    xcb_keysym_t down;
};

struct Config {
    size_t count_workspaces;
    std::string terminal;
    uint16_t resize_px;
    KeysConfig keys;
    WorkspaceConfig ws_config;
};