#pragma once

#include <xcb/xcb.h>

#include <memory>
#include <string>
#include <list>

#include "configs.hpp"
#include "display.hpp"
#include "tree.hpp"
#include "window.hpp"
#include "utils.hpp"

class Workspace {
public:
    Workspace(xcb_connection_t *connection);

    void InsertWindow(xcb_window_t w_id);
    void RemoveWindow(xcb_window_t w_id);

    inline void SetTilingOrient(Orientation orient) { t_orient_ = orient; }
    inline Orientation GetTilingOrient() const { return t_orient_; }

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

    Tree wins_tree_;

    std::list<Window> windows_;
    using window_iterator = typename std::list<Window>::iterator;
    window_iterator active_window_;

    Orientation t_orient_;

    window_iterator FindWindow(xcb_window_t w_id);

    void ResizeWindows();
    void ShowFrames(const TreeNodes::Node::const_ptr &node, int16_t x, int16_t y, uint32_t width, uint32_t height);
};