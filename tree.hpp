#pragma once

#include <xcb/xcb.h>

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "display.hpp"
#include "utils.hpp"
#include "frame.hpp"
#include "container.hpp"
#include "window.hpp"

// TODO: в данный момент существует возможность изменить структуру дерева из вне, подумать
// над утсранением проблемы

// Дерево расположения элементов на экране
// Принципы построения дерева:
// 1) Листьями могут быть только окна
// 2) Каждый фрейм содержит более 1 потомка. Исключение - корень, может иметь 1-го потомка
class Tree {
public:
    Tree();

    inline bool Empty() const { 
        return root_ == nullptr; 
    }

    inline bool Contains(xcb_window_t w_id) const {
        return id_to_node_.count(w_id);
    }

    void SetRoot(Frame::ptr frame);
    void AddNeighbour(xcb_window_t w_id, std::shared_ptr<Window> new_win, Orientation orient);

    void Remove(xcb_window_t w_id);

    void RotateFrameWithWindow(xcb_window_t w_id);

    std::shared_ptr<Window> GetWindow(xcb_window_t w_id);

    // Проблемный момент - возвращаем корень, который можно модифицировать
    // TODO: попробовать решить проблему
    Frame::ptr GetRoot() { 
        return root_;
    }

    std::shared_ptr<Container> GetContainerWithWindow(xcb_window_t w_id);

    auto begin() {
        return id_to_node_.begin();
    }

    auto end() {
        return id_to_node_.end();
    }
private:
    std::unordered_map<xcb_window_t, std::shared_ptr<Window>> id_to_node_;

    Frame::ptr root_;

    void UpdateWindows(Frame::ptr node);
};