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

// TODO: требуется рефакторинг
void Container::MoveResize(int16_t x, int16_t y, uint16_t width, uint16_t height) {
    // Коэффицент уменьшения/увеличения окна
    auto w_mult = static_cast<double>(width) / GetWidth();
    auto h_mult = static_cast<double>(height) / GetHeight();

    auto x_d = x;
    auto y_d = y;
    for (auto child : childs_) {
        child->MoveResize(
            x_d,
            y_d,
            static_cast<uint16_t>(child->GetWidth() * w_mult),
            static_cast<uint16_t>(child->GetHeight() * h_mult)
        );

        if (orient_ == Orientation::HORIZONTAL) {
            x_d += child->GetWidth();
        }
        else {
            y_d += child->GetHeight();
        }
    }

    // Компенсируем погрешность, которая возникает при вычисления
    // с плавающей точкой
    int w_diff = width - static_cast<int>(GetWidth());
    int h_diff = height - static_cast<int>(GetHeight());

    if (w_diff) {
        if (orient_ == Orientation::VERTICAL) {
            for (auto child : childs_) {
                child->MoveResize(
                    child->GetX(),
                    child->GetY(),
                    child->GetWidth() + w_diff,
                    child->GetHeight()
                );
            }
        }
        else {
            auto node = childs_.back();
            node->MoveResize(
                node->GetX(),
                node->GetY(),
                node->GetWidth() + w_diff,
                node->GetHeight()
            );
        }
    }

    if (h_diff) {
        if (orient_ == Orientation::HORIZONTAL) {
            for (auto child : childs_) {
                child->MoveResize(
                    child->GetX(),
                    child->GetY(),
                    child->GetWidth(),
                    child->GetHeight() + h_diff
                );
            }
        }
        else {
            auto node = childs_.back();
            node->MoveResize(
                node->GetX(),
                node->GetY(),
                node->GetWidth(),
                node->GetHeight() + h_diff
            );
        }
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

// TODO: требуется рефакторинг
void Container::ResizeChild(Frame::ptr node, int16_t px) {
    // Для выравнивания и компенсации погрешности сохраняем размеры фрейма
    auto r_width = GetWidth();
    auto r_height = GetHeight();

    if (orient_ == Orientation::VERTICAL) {
        // Пиксели, которые будут изменены у фреймах, не являющиеся node
        auto pixels_per_win = (2 * px) / (CountChilds() - 1);
        for (auto child : childs_) {
            int new_height = child->GetHeight();
            new_height += child == node ? 2 * px : -pixels_per_win;
            if (new_height >= GetHeight() || new_height <= 5) {
                return;
            }
        }

        auto x = GetX();
        auto y = GetY();
        for (auto child : childs_) {
            if (child == node) {
                node->MoveResize(
                    x,
                    y,
                    node->GetWidth(),
                    static_cast<uint16_t>(node->GetHeight() + 2 * px)
                );
            }
            else {
                child->MoveResize(
                    x,
                    y,
                    child->GetWidth(),
                    static_cast<uint16_t>(child->GetHeight() - pixels_per_win)
                );
            }

            y += child->GetHeight();
        }

        auto c_height = GetHeight();
        // Размер фрейма увеличился после вычислений
        // Для решения данной пробелмы изменяем размер последнего фрейма
        if (c_height != r_height) {
            auto diff = r_height - static_cast<int>(c_height);

            auto node = childs_.back();
            node->MoveResize(
                node->GetX(),
                node->GetY(),
                node->GetWidth(),
                node->GetHeight() + diff
            );
        }
    }
    else {
        // Пиксели, которые будут изменены у фреймах, не являющиеся node
        int pixels_per_win = (2 * px) / (CountChilds() - 1);
        for (auto child : childs_) {
            int new_width = child->GetWidth();
            new_width += child == node ? 2 * px : -pixels_per_win;
            if (new_width >= GetWidth() || new_width <= 5) {
                return;
            }
        }

        auto x = GetX();
        auto y = GetY();
        for (auto child : childs_) {
            if (child == node) {
                node->MoveResize(
                    x,
                    y,
                    static_cast<uint16_t>(node->GetWidth() + 2 * px),
                    node->GetHeight()
                );
            }
            else {
                child->MoveResize(
                    x,
                    y,
                    static_cast<uint16_t>(child->GetWidth() - pixels_per_win),
                    node->GetHeight()
                );
            }

            x += child->GetWidth();
        }
        
        auto c_width = GetWidth();
        // Размер фрейма увеличился после вычислений
        // Для решения данной пробелмы изменяем размер последнего фрейма
        if (c_width != r_width) {
            auto diff = r_width - static_cast<int>(c_width);

            auto node = childs_.back();
            node->MoveResize(
                node->GetX(),
                node->GetY(),
                node->GetWidth() + diff,
                node->GetHeight()
            );
        }
    }
}

vector<Frame::ptr>::iterator Container::FindChild(Frame::ptr node) {
    return find(begin(childs_), end(childs_), node);
}