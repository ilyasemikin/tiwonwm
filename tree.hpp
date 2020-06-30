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
        virtual ~Node() {}

        using ptr = std::shared_ptr<Node>;

        virtual NodeType GetType() const = 0;

        virtual std::string ToString() const = 0;

        inline void SetParent(ptr node) {
            parent_ = node;
        }
    private:
        ptr parent_;
    };

    class Frame : public Node {
    public:
        NodeType GetType() const override {
            return NodeType::FRAME;
        }

        std::string ToString() const override;

        inline void SetOrientation(Orientation orient) {
            orient_ = orient;
        }

        inline Orientation GetTilingType() {
            return orient_;
        }

        inline size_t CountChilds() const {
            return childs_.size();
        }

        inline Node::ptr GetChild(size_t pos) {
            return childs_[pos];
        }

        void AddChild(Node::ptr node, size_t pos = 0);
        void RemoveChild(size_t pos);
        void RemoveChild(Node::ptr node);
        void ReplaceChild(size_t pos, Node::ptr new_node);
        void ReplaceChild(Node::ptr node, Node::ptr new_node);
        bool ContainsChild(Node::ptr node);
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

enum class TilingOrientation {
    VERTICAL,
    HORIZONTAL
};

class Tree {
public:
    Tree();

    // В данные момент для простоты Frame является публичным
    // Однако стоит рассмотреть вариант с выделением данной сущности вне данного класса
    // TODO: рассмотреть подобный вариант
    class Frame;

    inline bool Empty() const { return root_ == nullptr; }

    void Add(xcb_window_t w_id, TilingOrientation t_orient);
    void AddNeighbour(xcb_window_t w_id, xcb_window_t new_win_id, TilingOrientation t_orient);

    void Remove(xcb_window_t w_id);

    inline Frame GetStructure() { return Frame(root_); }

    std::string GetStructureString();
private:
    enum class NodeType {
        WINDOW,
        V_FRAME,
        H_FRAME
    };
    
    std::string GetNodeTypeString(const NodeType &n_type);
    NodeType TOrientToNodeType(const TilingOrientation &t_orient);

    struct Node {
        using ptr = std::shared_ptr<Node>;
        NodeType type;
        xcb_window_t w_id;
        ptr parent;
        std::vector<ptr> childs;

        Node(NodeType n_type);
    };

public:
    class Frame {
    public:
        Frame(Node::ptr node) : node_(node) {}

        inline bool IsEmpty() const { return node_ == nullptr; }
        inline bool IsWindow() const { return node_->type == NodeType::WINDOW; }
        inline bool IsVerticalFrame() const { return node_->type == NodeType::V_FRAME; }
        inline bool IsHorizontalFrame() const { return node_->type == NodeType::H_FRAME; }

        inline xcb_window_t GetWindowId() const { return node_->w_id; };
        inline size_t ChildsCount() const { return node_->childs.size(); }
        inline Frame GetChild(size_t i) const { return Frame(node_->childs[i]); }
    private:
        Node::ptr node_;
    };

private:
    std::unordered_map<xcb_window_t, Node::ptr> id_to_node_;

    std::shared_ptr<Node> root_;

    std::string GetStructureString(std::shared_ptr<Node> node);
};