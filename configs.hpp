#pragma once

#include <cstdint>

struct WorkspaceConfig {
    uint8_t border_width;
    uint32_t border_color;
};

struct Config {
    uint8_t count_workspaces;

    WorkspaceConfig ws_config;
};