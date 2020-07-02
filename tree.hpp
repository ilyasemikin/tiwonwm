#pragma once

#include <xcb/xcb.h>

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "display.hpp"
#include "utils.hpp"
#include "frame.hpp"

// Дерево расположения элементов на экране
// Принципы построения дерева:
// 1) Листями могут быть только окна
// 2) Каждый фрейм содержит более 1 потомка. Исключение - корень, может иметь 1-го потомка
class Tree {
public:
    Tree(xcb_connection_t *connection);

    inline bool Empty() const { return root_ == nullptr; }

    void Add(xcb_window_t w_id);
    void AddNeighbour(xcb_window_t w_id, xcb_window_t new_win_id, Orientation orient);

    void Remove(xcb_window_t w_id);

    void RotateFrameWithWindow(xcb_window_t w_id);

    Frame::const_ptr GetStructure() { return root_; }
private:
    std::unordered_map<xcb_window_t, Frame::ptr> id_to_node_;

    // FIXME: удалить по окончании введения новой структуры кода фреймов
    xcb_connection_t *connection_;

    Frame::ptr root_;
};