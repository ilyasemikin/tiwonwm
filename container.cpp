#include "container.hpp"

#include <algorithm>
#include <functional>

using namespace std;

Container::Container() :
    orient_(Orientation::HORIZONTAL)
{

}

string Container::ToString() const {
    string ret = "[ " + to_string(orient_) + " container: ";
    for (size_t i = 0; i < childs_.size(); i++) {
        if (i != 0) {
            ret += ", ";
        }
        ret += childs_[i]->ToString();
    }
    return ret += " ]";
}

int16_t Container::GetX() const {
    if (childs_.empty()) {
        return 0;
    }

    return childs_[0]->GetX();
}

int16_t Container::GetY() const {
    if (childs_.empty()) {
        return 0;
    }

    return childs_[0]->GetY();
}

uint16_t Container::GetWidth() const {
    if (orient_ == Orientation::VERTICAL) {
        return childs_.size() == 0 ? 0 : childs_[0]->GetWidth();
    }

    uint16_t ret = 0;
    for (const auto &child : childs_) {
        ret += child->GetWidth();
    }
    return ret;
}

uint16_t Container::GetHeight() const {
    if (orient_ == Orientation::HORIZONTAL) {
        return childs_.size() == 0 ? 0 : childs_[0]->GetHeight();
    }

    uint16_t ret = 0;
    for (const auto child : childs_) {
        ret += child->GetHeight();
    }
    return ret;
}

bool Container::IsCorrectSize(uint16_t width, uint16_t height) const {
    if (!CountChilds()) {
        return false;
    }

    xcb_rectangle_t rect;
    rect.width = width;
    rect.height = height;
    auto sizes = GetNewFrameRect(rect);

    if (sizes.empty()) {
        return false;
    }

    for (size_t i = 0; i < CountChilds(); i++) {
        if (!childs_[i]->IsCorrectSize(sizes[i].width, sizes[i].height)) {
            return false;
        }
    }
    return true;
}

void Container::Move(int16_t x, int16_t y) {
    auto diff_x = x - GetX();
    auto diff_y = y - GetY();

    for (auto child : childs_) {
        child->Move(
            child->GetX() + diff_x,
            child->GetY() + diff_y
        );
    }
}

void Container::Resize(uint16_t width, uint16_t height) {
    xcb_rectangle_t rect;
    rect.x = GetX();
    rect.y = GetY();
    rect.width = width;
    rect.height = height;

    auto sizes = GetNewFrameRect(rect);
    
    for (size_t i = 0; i < childs_.size(); i++) {
        childs_[i]->MoveResize(
            sizes[i].x,
            sizes[i].y,
            sizes[i].width, 
            sizes[i].height
        );
    }
}

void Container::MoveResize(int16_t x, int16_t y, uint16_t width, uint16_t height) {
    Resize(width, height);
    Move(x, y);
}

// FIXME: не создавать классы состояния каждый раз по новой
void Container::SetOrientation(Orientation orient) {
    if (orient_ == orient) {
        return;
    }

    orient_ = orient;
}

// TODO: добавить способы оповещения вызывающей стороны
// о ошибке в функции
void Container::AddChild(Frame::ptr node, size_t pos) {
    if (pos > childs_.size()) {
        return;
    }

    vector<xcb_rectangle_t> sizes;

    if (childs_.size()) {
        xcb_rectangle_t exp_rect;
        exp_rect.width = GetWidth();
        exp_rect.height = GetHeight();

        uint16_t vert = orient_ == Orientation::VERTICAL ? 1 : 0;
        xcb_rectangle_t win_r;
        win_r.width = exp_rect.width / ((1 - vert) * CountChilds() + 1);
        win_r.height = exp_rect.height / (vert * CountChilds() + 1);

        xcb_rectangle_t w_old_sizes;
        w_old_sizes.x = GetX();
        w_old_sizes.y = GetY();
        w_old_sizes.width = exp_rect.width - (1 - vert) * win_r.width;
        w_old_sizes.height = exp_rect.height - vert * win_r.height;

        sizes = GetNewFrameRect(w_old_sizes);
        if (pos == 0) {
            win_r.x = sizes[0].x;
            win_r.y = sizes[0].y;
        }
        else if (pos == sizes.size()) {
            win_r.x = sizes[pos - 1].x;
            win_r.y = sizes[pos - 1].y;
        }
        else {
            win_r.x = sizes[pos].x;
            win_r.y = sizes[pos].y;
        }
        sizes.insert(begin(sizes) + pos, win_r);

        for (size_t i = pos + 1; i < sizes.size(); i++) {
            sizes[i].x += (1 - vert) * win_r.width;
            sizes[i].y += vert * win_r.height;
        }
    }

    childs_.insert(childs_.begin() + pos, node);
    node->SetParent(shared_from_this());

    if (!sizes.empty()) {
        for (size_t i = 0; i < sizes.size(); i++) {
            childs_[i]->MoveResize(
                sizes[i].x,
                sizes[i].y,
                sizes[i].width,
                sizes[i].height
            );
        }
    }
}

void Container::AddChildAfter(Frame::ptr after_node, Frame::ptr node) {
    auto it = FindChild(after_node);

    if (it == end(childs_)) {
        return;
    }

    AddChild(node, it - begin(childs_));
}

void Container::RemoveChild(Frame::ptr node) {
    auto it = FindChild(node);

    if (it != end(childs_)) {
        childs_.erase(it);
    }
}

void Container::ReplaceChild(Frame::ptr node, Frame::ptr new_node) {
    auto it = FindChild(node);

    if (it != end(childs_)) {
        new_node->MoveResize(
            (*it)->GetX(),
            (*it)->GetY(),
            (*it)->GetWidth(),
            (*it)->GetHeight()
        );
        *it = new_node;
        new_node->SetParent(shared_from_this());
    }
}

bool Container::ContainsChild(Frame::ptr node) const {
    return FindChild(node) == end(childs_);
}

void Container::ResizeChild(Frame::ptr node, int16_t px) {
    if (CountChilds() == 1) {
        return;
    }
    
    auto it = FindChild(node);
    if (it == end(childs_)) {
        return;
    }

    auto sizes = GetFrameWithResizedChild(it - begin(childs_), px);

    for (size_t i = 0; i < CountChilds(); i++) {
        if (!childs_[i]->IsCorrectSize(sizes[i].width, sizes[i].height)) {
            return;
        }
    }

    for (size_t i = 0; i < CountChilds(); i++) {
        childs_[i]->MoveResize(
            sizes[i].x,
            sizes[i].y,
            sizes[i].width,
            sizes[i].height
        );
    }
}

vector<Frame::ptr>::iterator Container::FindChild(Frame::ptr node) {
    return find(begin(childs_), end(childs_), node);
}

std::vector<Frame::ptr>::const_iterator Container::FindChild(Frame::ptr node) const {
    return find(begin(childs_), end(childs_), node);
}

vector<xcb_rectangle_t> Container::GetNewFrameRect(xcb_rectangle_t rect) const {
    if (!CountChilds()) {
        return {};
    }

    xcb_rectangle_t cur_rect;
    cur_rect.width = 0;
    cur_rect.height = 0;

    if (orient_ == Orientation::VERTICAL) {
        cur_rect.width = rect.width;
    }
    else {
        cur_rect.height = rect.height;
    }

    auto w_mult_ = static_cast<double>(rect.width) / GetWidth();
    auto h_mult_ = static_cast<double>(rect.height) / GetHeight();

    auto x = rect.x;
    auto y = rect.y;
    
    uint16_t vert = orient_ == Orientation::VERTICAL ? 1 : 0;
    vector<xcb_rectangle_t> ret(CountChilds());
    for (size_t i = 0; i < ret.size(); i++) {
        auto child = GetChild(i);

        ret[i].x = x;
        ret[i].y = y;
        ret[i].width = vert * rect.width + (1 - vert) * static_cast<uint16_t>(child->GetWidth() * w_mult_);
        ret[i].height = (1 - vert) * rect.height + vert * static_cast<uint16_t>(child->GetHeight() * h_mult_);

        cur_rect.width += (1 - vert) * ret[i].width;
        cur_rect.height += vert * ret[i].height;

        x += (1 - vert) * ret[i].width;
        y += vert * ret[i].height;
    }

    int w_d = rect.width - cur_rect.width;
    int h_d = rect.height - cur_rect.height;
    if (w_d || h_d) {
        auto &item = ret.back();
        item.width += w_d;
        item.height += h_d;
    }

    return ret;
}

vector<xcb_rectangle_t> Container::GetFrameWithResizedChild(size_t index, int16_t px) const {
    if (CountChilds() < 2) {
        return {};
    }

    xcb_rectangle_t exp_rect;
    exp_rect.width = GetWidth();
    exp_rect.height = GetHeight();

    auto px_per_other = (2 * px) / static_cast<int>(CountChilds() - 1);

    xcb_rectangle_t cur_rect;
    cur_rect.width = 0;
    cur_rect.height = 0;

    if (orient_ == Orientation::VERTICAL) {
        cur_rect.width = exp_rect.width;
    }
    else {
        cur_rect.height = exp_rect.height;
    }

    auto x = GetX();
    auto y = GetY();

    uint16_t vert = orient_ == Orientation::VERTICAL ? 1 : 0;

    vector<xcb_rectangle_t> ret(CountChilds());
    for (size_t i = 0; i < ret.size(); i++) {
        auto child = GetChild(i);
        int c_px = (i == index) ?  2 * px : -px_per_other;

        ret[i].x = x;
        ret[i].y = y;
        ret[i].width = vert * exp_rect.width + (1 - vert) * (child->GetWidth() + c_px );
        ret[i].height = (1 - vert) * exp_rect.height + vert * (child->GetHeight() + c_px );

        cur_rect.width += (1 - vert) * ret[i].width;
        cur_rect.height += vert * ret[i].height;

        x += (1 - vert) * ret[i].width;
        y += vert * ret[i].height;
    }

    int w_d = exp_rect.width - cur_rect.width;
    int h_d = exp_rect.height - cur_rect.height;
    if (w_d || h_d) {
        auto &item = ret.back();
        item.width += w_d;
        item.height += h_d;
    }

    return ret;
}