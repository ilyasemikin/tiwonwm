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

    auto child_x = GetX();
    auto child_y = GetY();
    for (auto child : childs_) {
        child->MoveResize(
            child_x,
            child_y,
            static_cast<uint16_t>(child->GetWidth() * w_mult),
            static_cast<uint16_t>(child->GetHeight() * h_mult)
        );

        if (orient_ == Orientation::HORIZONTAL) {
            child_x += child->GetWidth();
        }
        else {
            child_y += child->GetHeight();
        }
    }

    // Компенсируем погрешность, которая возникает при вычисления
    // с плавающей точкой
    int w_diff = width - static_cast<int>(GetWidth());
    int h_diff = height - static_cast<int>(GetHeight());

    auto change_node_size = [](Frame::ptr node, int w_diff, int h_diff) {
        node->Resize(
            node->GetWidth() + w_diff,
            node->GetHeight() + h_diff
        );
    };

    if (w_diff) {
        if (orient_ == Orientation::VERTICAL) {
            for (auto child : childs_) {
                change_node_size(child, w_diff, 0);
            }
        }
        else {
            change_node_size(childs_.back(), w_diff, 0);
        }
    }

    if (h_diff) {
        if (orient_ == Orientation::HORIZONTAL) {
            for (auto child : childs_) {
                change_node_size(child, 0, h_diff);
            }
        }
        else {
            change_node_size(childs_.back(), 0, h_diff);
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

void Container::RemoveChild(size_t pos) {
    if (pos > childs_.size()) {
        return;
    }

    childs_.erase(begin(childs_) + pos);
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
    return FindChild(node) == end(childs_);
}

// TODO: требуется рефакторинг
void Container::ResizeChild(Frame::ptr node, int16_t px) {
    if (CountChilds() < 2) {
        return;
    }

    // Для выравнивания и компенсации погрешности сохраняем размеры фрейма
    auto r_width = GetWidth();
    auto r_height = GetHeight();

    // Пиксели, которые будут изменены у фреймах, не являющиеся node
    auto pixels_per_win = (2 * px) / static_cast<int>(CountChilds() - 1);
    if (orient_ == Orientation::VERTICAL) {
        for (auto child : childs_) {
            int new_height = child->GetHeight();
            new_height += child == node ? 2 * px : -pixels_per_win;
            if (!child->IsCorrectSize(r_width, new_height)) {
                return;
            }
        }
    }
    else {
        for (auto child : childs_) {
            int new_width = child->GetWidth();
            new_width += child == node ? 2 * px : -pixels_per_win;
            if (!child->IsCorrectSize(new_width, r_height)) {
                return;
            }
        }
    }

    auto x = GetX();
    auto y = GetY();
    for (auto child : childs_) {
        int c_px = (child == node) ? 2 * px : -pixels_per_win; 

        auto win_width = child->GetWidth();
        auto win_height = child->GetHeight();
        if (orient_ == Orientation::VERTICAL) {
            win_height += c_px;
        }
        else {
            win_width += c_px;
        }

        child->MoveResize(x, y, win_width, win_height);

        if (orient_ == Orientation::VERTICAL) {
            y += win_height;
        }
        else {
            x += win_width;
        }
    }

    auto w_diff = r_width - static_cast<int>(GetWidth());
    auto h_diff = r_height - static_cast<int>(GetHeight());    
    if (w_diff || h_diff) {
        auto node = childs_.back();
        node->Resize(
            node->GetWidth() + w_diff,
            node->GetHeight() + h_diff
        );
    }
}

vector<Frame::ptr>::iterator Container::FindChild(Frame::ptr node) {
    return find(begin(childs_), end(childs_), node);
}

std::vector<Frame::ptr>::const_iterator Container::FindChild(Frame::ptr node) const {
    return find(begin(childs_), end(childs_), node);
}