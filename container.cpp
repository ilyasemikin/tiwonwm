#include "container.hpp"

#include <algorithm>
#include <functional>

using namespace std;

namespace ContainerStates {
    State::~State() {

    }

    vector<xcb_rectangle_t> State::GetNewFrameRect(shared_ptr<Container> cont, xcb_rectangle_t rect) {
        if (cont == nullptr || !cont->CountChilds()) {
            return { };
        }
        
        exit_ = false;

        last_child_ = cont->GetChild(cont->CountChilds() - 1);

        exp_width_ = rect.width;
        exp_height_ = rect.height;

        cur_width_ = cur_height_ = 0;

        auto w_mult_ = static_cast<double>(rect.width) / cont->GetWidth();
        auto h_mult_ = static_cast<double>(rect.height) / cont->GetHeight();

        auto x = rect.x;
        auto y = rect.y;
        vector<xcb_rectangle_t> ret(cont->CountChilds());
        for (size_t i = 0; i < ret.size(); i++) {
            cur_child_ = cont->GetChild(i);

            ret[i].x = x;
            ret[i].y = y;
            ret[i].width = GetNewWdith(w_mult_);
            ret[i].height = GetNewHeight(h_mult_);

            if (exit_) {
                break;
            }

            x = GetNextX(x, ret[i].width);
            y = GetNextY(y, ret[i].height);
        }

        if (exit_) {
            ret.clear();
        }
        
        last_child_ = cur_child_ = nullptr;

        return ret;
    }

    vector<xcb_rectangle_t> State::GetFrameWithResizedChild(std::shared_ptr<Container> cont, size_t index, int16_t px) {
        if (cont == nullptr || cont->CountChilds() < 2) {
            return { };
        }

        exp_width_ = cont->GetWidth();
        exp_height_ = cont->GetHeight();

        auto px_per_other = (2 * px) / static_cast<int>(cont->CountChilds() - 1);

        cur_width_ = cur_height_ = 0;
        last_child_ = cont->GetChild(cont->CountChilds() - 1);

        auto x = cont->GetX();
        auto y = cont->GetY();

        vector<xcb_rectangle_t> ret(cont->CountChilds());
        for (size_t i = 0; i < cont->CountChilds(); i++) {
            cur_child_ = cont->GetChild(i);
            int c_px = (i == index) ?  2 * px : -px_per_other;

            ret[i].x = x;
            ret[i].y = y;
            ret[i].width = GetNewWidth(c_px);
            ret[i].height = GetNewHeight(c_px);

            x = GetNextX(x, ret[i].width);
            y = GetNextY(y, ret[i].height);
        }

        return ret;
    }

    // Vertical state
    uint16_t Vertical::GetNewWidth(int) {
        return exp_width_;
    }

    uint16_t Vertical::GetNewWdith(double) {
        return exp_width_;
    }

    uint16_t Vertical::GetNewHeight(int px) {
        auto ret = cur_child_->GetHeight() + px;
        if (cur_child_ == last_child_) {
            ret = exp_height_ - cur_height_;
        }
        cur_height_ += ret;
        return ret;
    }

    uint16_t Vertical::GetNewHeight(double mult) {
        auto ret = static_cast<uint16_t>(cur_child_->GetHeight() * mult);
        if (cur_child_ == last_child_) {
            ret = exp_height_ - cur_height_;
        }
        cur_height_ += ret;
        return ret;
    }

    int16_t Vertical::GetNextX(int16_t x, uint16_t) {
        return x;
    }
    
    int16_t Vertical::GetNextY(int16_t y, uint16_t height) {
        return y + height;
    }

    // Horizontal state
    uint16_t Horizontal::GetNewWidth(int px) {
        auto ret = cur_child_->GetWidth() + px;
        if (cur_child_ == last_child_) {
            ret = exp_width_ - cur_width_;
        }
        cur_width_ += ret;
        return ret;
    }

    uint16_t Horizontal::GetNewWdith(double mult) {
        auto ret = static_cast<uint16_t>(cur_child_->GetWidth() * mult);
        if (cur_child_ == last_child_) {
            ret = exp_width_ - cur_width_;
        }
        cur_width_ += ret;
        return ret;
    }

    uint16_t Horizontal::GetNewHeight(int) {
        return exp_height_;
    }

    uint16_t Horizontal::GetNewHeight(double) {
        return exp_height_;
    }

    int16_t Horizontal::GetNextX(int16_t x, uint16_t width) {
        return x + width;
    }
    
    int16_t Horizontal::GetNextY(int16_t y, uint16_t) {
        return y;
    }
}

Container::Container() :
    orient_(Orientation::HORIZONTAL),
    // FIXME: грамотно инициализировать
    state_(make_shared<ContainerStates::Horizontal>())
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

    for (size_t i = 0; i < CountChilds(); i++) {
        if (!childs_[i]->IsCorrectSize(width, height)) {
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

    auto cont = dynamic_pointer_cast<Container>(shared_from_this());
    auto sizes = state_->GetNewFrameRect(cont, rect);
    
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
    if (orient_ == Orientation::VERTICAL) {
        state_ = make_shared<ContainerStates::Vertical>();
    }
    else {
        state_ = make_shared<ContainerStates::Horizontal>();
    }
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

    if (it == end(childs_)) {
        return;
    }

    childs_.insert(next(it), node);
    node->SetParent(shared_from_this());
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
        *it = new_node;
    }
    new_node->SetParent(shared_from_this());
}

bool Container::ContainsChild(Frame::ptr node) const {
    return FindChild(node) == end(childs_);
}

void Container::ResizeChild(Frame::ptr node, int16_t px) {
    auto it = FindChild(node);
    if (it == end(childs_)) {
        return;
    }

    auto cont = dynamic_pointer_cast<Container>(shared_from_this());
    auto sizes = state_->GetFrameWithResizedChild(cont, it - begin(childs_), px);

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