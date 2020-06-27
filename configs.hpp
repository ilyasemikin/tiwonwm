#pragma once

#include <cstdint>
#include <string>

struct WorkspaceConfig {
    uint8_t border_width;
    uint32_t focused_border_color;
    uint32_t unfocused_border_color;
};

struct Config {
    uint8_t count_workspaces;

    std::string terminal;

    WorkspaceConfig ws_config;
};