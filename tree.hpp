#pragma once

#include <xcb/xcb.h>

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "display.hpp"
#include "window.hpp"

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