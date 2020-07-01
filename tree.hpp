#pragma once

#include <xcb/xcb.h>

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "display.hpp"
#include "utils.hpp"
#include "window.hpp"

namespace TreeNodes {
    enum class NodeType {
        FRAME,
        WINDOW
    };

    class Node : public std::enable_shared_from_this<Node> {
    public:
        Node();
        virtual ~Node();

        using ptr = std::shared_ptr<Node>;
        using const_ptr = std::shared_ptr<const Node>;

        virtual NodeType GetType() const = 0;

        virtual std::string ToString() const = 0;

        inline void SetParent(ptr node) {
            parent_ = node;
        }

        inline ptr GetParent() {
            return parent_;
        }
    private:
        ptr parent_;
    };

    class Frame : public Node {
    public:
        Frame();

        NodeType GetType() const override {
            return NodeType::FRAME;
        }

        std::string ToString() const override;

        inline void SetOrientation(Orientation orient) {
            orient_ = orient;
        }

        inline Orientation GetTilingType() const {
            return orient_;
        }

        inline size_t CountChilds() const {
            return childs_.size();
        }

        inline Node::ptr GetChild(size_t pos) {
            return childs_[pos];
        }

        inline Node::const_ptr GetConstChild(size_t pos) const {
            return childs_[pos];
        }

        void AddChild(Node::ptr node, size_t pos = 0);
        void AddChildAfter(Node::ptr after_node, Node::ptr node);
        void RemoveChild(size_t pos);
        void RemoveChild(Node::ptr node);
        void ReplaceChild(size_t pos, Node::ptr new_node);
        void ReplaceChild(Node::ptr node, Node::ptr new_node);
        bool ContainsChild(Node::ptr node) const;
    private:
        std::vector<Node::ptr> childs_;
        Orientation orient_;

        std::vector<Node::ptr>::iterator FindChild(Node::ptr node);
    };

    class Window : public Node {
    public:
        Window(xcb_window_t id);

        inline NodeType GetType() const override {
            return NodeType::WINDOW;
        }

        std::string ToString() const override;

        inline xcb_window_t GetId() const {
            return id_;
        }
    private:
        xcb_window_t id_;
    };
}

// Дерево расположения элементов на экране
// Принципы построения дерева:
// 1) Листями могут быть только окна
// 2) Каждый фрейм содержит более 1 потомка. Исключение - корень, может иметь 1-го потомка
class Tree {
public:
    Tree();

    inline bool Empty() const { return root_ == nullptr; }

    void Add(xcb_window_t w_id);
    void AddNeighbour(xcb_window_t w_id, xcb_window_t new_win_id, Orientation orient);

    void Remove(xcb_window_t w_id);

    TreeNodes::Node::const_ptr GetStructure() { return root_; }
private:
    std::unordered_map<xcb_window_t, TreeNodes::Node::ptr> id_to_node_;

    TreeNodes::Node::ptr root_;
};