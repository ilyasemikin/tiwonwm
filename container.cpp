#include "container.hpp"

#include <algorithm>

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
    // Коэффицент уменьшения/увеличения окна
    double mult = 0;

    uint16_t res_size = 0;

    auto last = childs_.back();
    if (orient_ == Orientation::HORIZONTAL) {
        mult = width / static_cast<double>(GetWidth());
        for (auto child : childs_) {
            auto w = static_cast<uint16_t>(child->GetWidth() * mult);
            if (child == last) {
                w = width - res_size;
            }
            
            res_size += w;

            if (!child->IsCorrectSize(w, height)) {
                return false;
            }
        }
    }
    else {
        mult = height / static_cast<double>(GetHeight());
        for (auto child : childs_) {
            auto h = static_cast<uint16_t>(child->GetHeight() * mult);
            if (child == last) {
                h = height - res_size;
            }
            
            res_size += h;

            if (!child->IsCorrectSize(width, h)) {
                return false;
            }
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
    // Коэффицент уменьшения/увеличения окна
    auto w_mult = static_cast<double>(width) / GetWidth();
    auto h_mult = static_cast<double>(height) / GetHeight();

    uint16_t c_w = 0;
    uint16_t c_h = 0;
    auto win_x = GetX();
    auto win_y = GetY();
    for (auto child : childs_) {
        auto win_w = width;
        auto win_h = height;
        if (orient_ == Orientation::HORIZONTAL) {
            win_w = static_cast<uint16_t>(child->GetWidth() * w_mult);
            if (child == childs_.back()) {
                win_w = width - c_w;
            }
            c_w += win_w;
        }
        else {
            win_h = static_cast<uint16_t>(child->GetHeight() * h_mult);
            if (child == childs_.back()) {
                win_h = height - c_h;
            }
            c_h += win_h;
        }

        child->MoveResize(win_x, win_y, win_w, win_h);

        if (orient_ == Orientation::HORIZONTAL) {
            win_x += child->GetWidth();
        }
        else {
            win_y += child->GetHeight();
        }
    }
}

void Container::MoveResize(int16_t x, int16_t y, uint16_t width, uint16_t height) {
    Resize(width, height);
    Move(x, y);
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

// TODO: требуется рефакторинг
void Container::ResizeChild(Frame::ptr node, int16_t px) {
    if (CountChilds() < 2) {
        return;
    }

    // Для выравнивания и компенсации погрешности сохраняем размеры фрейма
    auto r_w = GetWidth();
    auto r_h = GetHeight();

    // Пиксели, которые будут изменены у фреймах, не являющиеся node
    auto pixels_per_win = (2 * px) / static_cast<int>(CountChilds() - 1);
    if (orient_ == Orientation::VERTICAL) {
        for (auto child : childs_) {
            int new_height = child->GetHeight();
            new_height += child == node ? 2 * px : -pixels_per_win;
            if (!child->IsCorrectSize(r_w, new_height)) {
                return;
            }
        }
    }
    else {
        for (auto child : childs_) {
            int new_width = child->GetWidth();
            new_width += child == node ? 2 * px : -pixels_per_win;
            if (!child->IsCorrectSize(new_width, r_h)) {
                return;
            }
        }
    }

    uint16_t c_w = 0;
    uint16_t c_h = 0;

    auto x = GetX();
    auto y = GetY();
    for (auto child : childs_) {
        int c_px = (child == node) ? 2 * px : -pixels_per_win; 

        auto win_w = child->GetWidth();
        auto win_h = child->GetHeight();
        if (orient_ == Orientation::VERTICAL) {
            win_h += c_px;
            if (child == childs_.back()) {
                win_h = r_h - c_h;
            }
            c_h += win_h;
        }
        else {
            win_w += c_px;
            if (child == childs_.back()) {
                win_w = r_w - c_w;
            }
            c_w += win_w;
        }

        child->MoveResize(x, y, win_w, win_h);

        if (orient_ == Orientation::VERTICAL) {
            y += win_h;
        }
        else {
            x += win_w;
        }
    }
}

vector<Frame::ptr>::iterator Container::FindChild(Frame::ptr node) {
    return find(begin(childs_), end(childs_), node);
}

std::vector<Frame::ptr>::const_iterator Container::FindChild(Frame::ptr node) const {
    return find(begin(childs_), end(childs_), node);
}