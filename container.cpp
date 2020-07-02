#include "container.hpp"

#include <algorithm>

using namespace std;

Container::Container() :
    orient_(Orientation::HORIZONTAL)
{

}

string Container::ToString() const {
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
void Container::AddChild(Frame::ptr node, size_t pos) {
    if (pos > childs_.size()) {
        return;
    }

    childs_.insert(childs_.begin() + pos, node);
    node->SetParent(shared_from_this());
}

void Container::AddChildAfter(Frame::ptr after_node, Frame::ptr node) {
    auto it = FindChild(after_node);

    if (it == childs_.end()) {
        return;
    }

    childs_.insert(next(it), node);
    node->SetParent(shared_from_this());
}

void Container::RemoveChild(size_t pos) {
    if (pos > childs_.size()) {
        return;
    }

    childs_.erase(childs_.begin() + pos);
}

void Container::RemoveChild(Frame::ptr node) {
    auto it = FindChild(node);

    if (it != end(childs_)) {
        childs_.erase(it);
    }
}

void Container::ReplaceChild(size_t pos, Frame::ptr new_node) {
    if (pos > childs_.size()) {
        return;
    }

    childs_[pos] = new_node;
    new_node->SetParent(shared_from_this());
}

void Container::ReplaceChild(Frame::ptr node, Frame::ptr new_node) {
    auto it = FindChild(node);

    if (it != end(childs_)) {
        *it = new_node;
    }
    new_node->SetParent(shared_from_this());
}

bool Container::ContainsChild(Frame::ptr node) const {
    return find(begin(childs_), end(childs_), node) != end(childs_);
}

vector<Frame::ptr>::iterator Container::FindChild(Frame::ptr node) {
    return find(begin(childs_), end(childs_), node);
}