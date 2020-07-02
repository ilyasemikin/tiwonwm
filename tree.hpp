#pragma once

#include <xcb/xcb.h>

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "display.hpp"
#include "utils.hpp"
#include "frame.hpp"
#include "window.hpp"

// Дерево расположения элементов на экране
// Принципы построения дерева:
// 1) Листями могут быть только окна
// 2) Каждый фрейм содержит более 1 потомка. Исключение - корень, может иметь 1-го потомка
class Tree {
public:
    Tree(xcb_connection_t *connection);

    inline bool Empty() const { 
        return root_ == nullptr; 
    }

    inline bool Contains(xcb_window_t w_id) const {
        return id_to_node_.count(w_id);
    }

    void Add(xcb_window_t w_id);
    void AddNeighbour(xcb_window_t w_id, xcb_window_t new_win_id, Orientation orient);

    void Remove(xcb_window_t w_id);

    void RotateFrameWithWindow(xcb_window_t w_id);

    std::shared_ptr<Window> GetWindow(xcb_window_t w_id);

    Frame::ptr GetStructure() { return root_; }

    auto begin() {
        return id_to_node_.begin();
    }

    auto end() {
        return id_to_node_.end();
    }
private:
    std::unordered_map<xcb_window_t, Frame::ptr> id_to_node_;

    // FIXME: удалить по окончании введения новой структуры кода фреймов
    xcb_connection_t *connection_;

    Frame::ptr root_;
};