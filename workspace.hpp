#pragma once

#include <xcb/xcb.h>

#include <memory>

#include "configs.hpp"
#include "display.hpp"
#include "tree.hpp"
#include "frame.hpp"
#include "window.hpp"
#include "utils.hpp"

class Workspace {
public:
    Workspace(xcb_connection_t *connection);

    void InsertWindow(xcb_window_t w_id);
    void RemoveWindow(xcb_window_t w_id);

    inline void SetTilingOrient(Orientation orient) { 
        t_orient_ = orient;
    }
    
    inline Orientation GetTilingOrient() const { 
        return t_orient_;
    }
    
    inline void SwitchTilingOrient() {
        t_orient_ = GetOtherOrientation(t_orient_);
    }

    void Show();
    void Hide();

    void SetFocus(xcb_window_t w_id);

    void RotateFocusFrame();

    void ResizeWindow(Orientation orient, int16_t px);

    void MaximizeWindow();

    bool Contains(xcb_window_t w_id);

    // Обработка событий, для которых требуется определенная информация 
    // о окне, которой не владеет класс WindowManager
    // TODO: подумать, возможно стоит изменить подход к обработки подобных событтий
    void ProcessEventByWindow(xcb_window_t w_id, xcb_generic_event_t *raw_event);

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

    std::shared_ptr<Window> active_window_;

    Orientation t_orient_;

    void ShowFrames();
    void ShowFrame(Frame::ptr node, int16_t x, int16_t y, uint32_t width, uint32_t height);
};