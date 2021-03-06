#include "tree.hpp"

#include <algorithm>

using namespace std;

Tree::Tree() :
    root_(nullptr)
{

}

void Tree::SetRoot(Frame::ptr frame) {
    id_to_node_.clear();

    if (frame->GetType() == FrameType::WINDOW) {
        auto win = frame;
        auto parent = make_shared<Container>();
        parent->SetOrientation(Orientation::HORIZONTAL);
        parent->AddChild(win);
        
        frame = parent;
    }

    root_ = frame;

    UpdateWindows(root_);
}

void Tree::AddNeighbour(xcb_window_t w_id, std::shared_ptr<Window> new_win, Orientation orient) { 
    if (!id_to_node_.count(w_id)) {
        return;
    }

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

shared_ptr<Window> Tree::GetWindow(xcb_window_t w_id) {
    return dynamic_pointer_cast<Window>(id_to_node_[w_id]);
}

shared_ptr<Container> Tree::GetContainerWithWindow(xcb_window_t w_id) {
    if (!Contains(w_id)) {
        return nullptr;
    }

    return dynamic_pointer_cast<Container>(id_to_node_[w_id]->GetParent());
}

void Tree::UpdateWindows(Frame::ptr node) {
    if (node->GetType() == FrameType::WINDOW) {
        auto win_node = dynamic_pointer_cast<Window>(node);
        id_to_node_.insert({ win_node->GetId(), win_node });
        return;
    }

    auto container = dynamic_pointer_cast<Container>(node);
    for (size_t i = 0; i < container->CountChilds(); i++) {
        UpdateWindows(container->GetChild(i));
    }
}