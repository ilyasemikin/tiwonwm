#include "tree.hpp"

#include <algorithm>

using namespace std;

Tree::Tree() :
    root_(nullptr)
{

}

void Tree::Add(xcb_window_t w_id, TilingOrientation t_orient) {
    if (root_ == nullptr) {
        root_ = make_shared<Node>(TOrientToNodeType(t_orient));
    }

    auto new_win = make_shared<Node>(NodeType::WINDOW);
    new_win->w_id = w_id;
    new_win->parent = root_;

    root_->childs.push_back(new_win);

    id_to_node.insert({ new_win->w_id, new_win });
}

void Tree::AddNeighbour(xcb_window_t w_id, xcb_window_t new_win_id, TilingOrientation t_orient) { 
    if (!id_to_node.count(w_id)) {
        return;
    }

    auto get_new_win = [](auto id, auto parent) {
        auto ret = make_shared<Node>(NodeType::WINDOW);
        ret->w_id = id;
        ret->parent = parent;
        return ret;
    };

    auto node = id_to_node[w_id];
    if (node->parent->type == TOrientToNodeType(t_orient)) {
        auto node_pos = find(
            begin(node->parent->childs),
            end(node->parent->childs),
            node
        );

        auto new_win = get_new_win(new_win_id, node->parent);

        new_win->parent->childs.insert(next(node_pos), new_win);

        id_to_node.insert({ new_win->w_id, new_win });
    }
    else {
        auto frame_pos = node->parent->childs.erase(
            find(
                begin(node->parent->childs),
                end(node->parent->childs),
                node
            )
        );
        
        auto new_frame = make_shared<Node>(TOrientToNodeType(t_orient));
        new_frame->parent = node->parent;
        node->parent = new_frame;
        new_frame->childs.push_back(node);
        new_frame->parent->childs.insert(frame_pos, new_frame);

        auto new_win = get_new_win(new_win_id, new_frame);
        new_win->parent->childs.push_back(new_win);

        id_to_node.insert({ new_win->w_id, new_win });
    }
}

string Tree::GetStructureString() {
    return GetStructureString(root_);
}

string Tree::GetStructureString(std::shared_ptr<Node> node) {
    if (node == nullptr) {
        return "";
    }

    if (node->type == NodeType::WINDOW) {
        return to_string(node->w_id);
    }
    
    string ret = "[ " + GetNodeTypeString(node->type) + ": ";
    for (size_t i = 0; i < node->childs.size(); i++) {
        if (i != 0) {
            ret += ", ";
        }
        ret += GetStructureString(node->childs[i]);
    }

    ret += " ]";

    return ret;
}

string Tree::GetNodeTypeString(const NodeType &n_type) {
    const unordered_map<NodeType, string> type_to_string {
        { NodeType::WINDOW, "Window" },
        { NodeType::H_FRAME, "Horizontal frame" },
        { NodeType::V_FRAME, "Vertical frame" }
    };
    return type_to_string.at(n_type);
}

Tree::NodeType Tree::TOrientToNodeType(const TilingOrientation &t_orient) {
    return t_orient == TilingOrientation::HORIZONTAL ? NodeType::H_FRAME : NodeType::V_FRAME;
}

Tree::Node::Node(NodeType n_type) :
    type(n_type),
    w_id(0),
    parent(nullptr)
{

}