#include "tree.hpp"

#include <algorithm>

#include "container.hpp"
#include "window.hpp"

using namespace std;

Tree::Tree(xcb_connection_t *connection) :
    connection_(connection),
    root_(nullptr)
{

}

void Tree::Add(xcb_window_t w_id) {
    auto new_win = make_shared<Window>(connection_, w_id);
    
    if (root_ == nullptr) {
        auto frame = make_shared<Container>();
        frame->AddChild(new_win);
        
        root_ = frame;
    }
    else {
        auto frame = dynamic_pointer_cast<Container>(root_);
        frame->AddChild(new_win);
    }

    id_to_node_.insert({ new_win->GetId(), new_win });
}

void Tree::AddNeighbour(xcb_window_t w_id, xcb_window_t new_win_id, Orientation orient) { 
    if (!id_to_node_.count(w_id)) {
        return;
    }

    auto new_win = make_shared<Window>(connection_, new_win_id);
    auto node = id_to_node_[w_id];
    auto parent = dynamic_pointer_cast<Container>(node->GetParent());

    if (parent->GetOrientation() == orient) {
        parent->AddChildAfter(node, new_win);
    }
    else {
        if (parent->CountChilds() == 1) {
            parent->SetOrientation(orient);
            parent->AddChildAfter(node, new_win);
        }
        else {
            auto new_frame = make_shared<Container>();
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
    auto parent = dynamic_pointer_cast<Container>(node->GetParent());

    if (parent->CountChilds() == 1 && node->GetParent() == root_) {
        root_.reset();
    }
    else {
        parent->RemoveChild(node);

        if (parent->CountChilds() == 1) {
            if (node->GetParent() != root_) {
                auto p = dynamic_pointer_cast<Container>(parent->GetParent());
                p->ReplaceChild(parent, parent->GetChild(0));
            }
            else {
                auto root = dynamic_pointer_cast<Container>(root_);
                if (root->GetChild(0)->GetType() == FrameType::CONTAINER) {
                    root_ = root->GetChild(0);
                }
            }
        }
    }

    id_to_node_.erase(w_id);
}

void Tree::RotateFrameWithWindow(xcb_window_t w_id) {
    if (!id_to_node_.count(w_id)) {
        return;
    }

    auto node = id_to_node_[w_id];
    auto frame = dynamic_pointer_cast<Container>(node->GetParent());

    frame->SetOrientation(GetOtherOrientation(frame->GetOrientation()));
}

std::shared_ptr<Window> Tree::GetWindow(xcb_window_t w_id) {
    return dynamic_pointer_cast<Window>(id_to_node_[w_id]);
}