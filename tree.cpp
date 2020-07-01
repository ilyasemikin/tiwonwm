#include "tree.hpp"

#include <algorithm>

using namespace std;

namespace TreeNodes {
    Node::Node() :
        parent_(nullptr)
    {

    }

    Node::~Node() {

    }

    Frame::Frame() :
        orient_(Orientation::HORIZONTAL)
    {

    }

    string Frame::ToString() const {
        string ret = "[ " + to_string(orient_) + " frame: ";
        for (size_t i = 0; i < childs_.size(); i++) {
            if (i != 0) {
                ret += ", ";
            }
            ret += childs_[i]->ToString();
        }
        return ret += " ]";
    }
    
    // TODO: добавить способы оповещения вызывающей стороны
    // о ошибке в функции
    void Frame::AddChild(Node::ptr node, size_t pos) {
        if (pos > childs_.size()) {
            return;
        }

        childs_.insert(childs_.begin() + pos, node);
        node->SetParent(shared_from_this());
    }

    void Frame::AddChildAfter(Node::ptr after_node, Node::ptr node) {
        auto it = FindChild(after_node);

        if (it == childs_.end()) {
            return;
        }

        childs_.insert(next(it), node);
        node->SetParent(shared_from_this());
    }

    void Frame::RemoveChild(size_t pos) {
        if (pos > childs_.size()) {
            return;
        }

        childs_.erase(childs_.begin() + pos);
    }

    void Frame::RemoveChild(Node::ptr node) {
        auto it = FindChild(node);

        if (it != end(childs_)) {
            childs_.erase(it);
        }
    }

    void Frame::ReplaceChild(size_t pos, Node::ptr new_node) {
        if (pos > childs_.size()) {
            return;
        }

        childs_[pos] = new_node;
        new_node->SetParent(shared_from_this());
    }

    void Frame::ReplaceChild(Node::ptr node, Node::ptr new_node) {
        auto it = FindChild(node);

        if (it != end(childs_)) {
            *it = new_node;
        }
        new_node->SetParent(shared_from_this());
    }

    bool Frame::ContainsChild(Node::ptr node) const {
        return find(begin(childs_), end(childs_), node) != end(childs_);
    }

    vector<Node::ptr>::iterator Frame::FindChild(Node::ptr node) {
        return find(begin(childs_), end(childs_), node);
    }

    Window::Window(xcb_window_t id) : 
        id_(id)
    {

    }

    string Window::ToString() const {
        return to_string(id_);
    }
}

Tree::Tree() :
    root_(nullptr)
{

}

void Tree::Add(xcb_window_t w_id) {
    auto new_win = make_shared<TreeNodes::Window>(w_id);
    
    if (root_ == nullptr) {
        auto frame = make_shared<TreeNodes::Frame>();
        frame->AddChild(new_win);
        
        root_ = frame;
    }
    else {
        auto frame = dynamic_pointer_cast<TreeNodes::Frame>(root_);
        frame->AddChild(new_win);
    }

    id_to_node_.insert({ new_win->GetId(), new_win });
}

void Tree::AddNeighbour(xcb_window_t w_id, xcb_window_t new_win_id, Orientation orient) { 
    if (!id_to_node_.count(w_id)) {
        return;
    }

    auto new_win = make_shared<TreeNodes::Window>(new_win_id);
    auto node = id_to_node_[w_id];
    auto parent = dynamic_pointer_cast<TreeNodes::Frame>(node->GetParent());

    if (parent->GetTilingType() == orient) {
        parent->AddChildAfter(node, new_win);
    }
    else {
        if (parent->CountChilds() == 1) {
            parent->SetOrientation(orient);
            parent->AddChildAfter(node, new_win);
        }
        else {
            auto new_frame = make_shared<TreeNodes::Frame>();
            new_frame->SetOrientation(orient);
            new_frame->AddChild(new_win);

            parent->ReplaceChild(node, new_frame);
            new_frame->AddChild(node);
        }
    }

    id_to_node_.insert({ new_win->GetId(), new_win });
}

void Tree::Remove(xcb_window_t w_id) {
    if (!id_to_node_.count(w_id)) {
        return;
    }

    auto node = id_to_node_[w_id];
    auto parent = dynamic_pointer_cast<TreeNodes::Frame>(node->GetParent());

    if (parent->CountChilds() == 1 && node->GetParent() == root_) {
        root_.reset();
    }
    else {
        parent->RemoveChild(node);

        if (parent->CountChilds() == 1) {
            if (node->GetParent() != root_) {
                auto p = dynamic_pointer_cast<TreeNodes::Frame>(parent->GetParent());
                p->ReplaceChild(parent, parent->GetChild(0));
            }
            else {
                auto root = dynamic_pointer_cast<TreeNodes::Frame>(root_);
                if (root->GetChild(0)->GetType() == TreeNodes::NodeType::FRAME) {
                    root_ = root->GetChild(0);
                }
            }
        }
    }

    id_to_node_.erase(w_id);
}