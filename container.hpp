#pragma once

#include <string>
#include <vector>

#include "frame.hpp"
#include "utils.hpp"

class Container : public Frame {
public:
    Container();

    FrameType GetType() const override {
        return FrameType::CONTAINER;
    }

    std::string ToString() const override;

    inline void SetOrientation(Orientation orient) {
        orient_ = orient;
    }

    inline Orientation GetOrientation() const {
        return orient_;
    }

    inline size_t CountChilds() const {
        return childs_.size();
    }

    inline Frame::ptr GetChild(size_t pos) {
        return childs_[pos];
    }

    inline Frame::const_ptr GetConstChild(size_t pos) const {
        return childs_[pos];
    }

    void AddChild(Frame::ptr node, size_t pos = 0);
    void AddChildAfter(Frame::ptr after_node, Frame::ptr node);
    void RemoveChild(size_t pos);
    void RemoveChild(Frame::ptr node);
    void ReplaceChild(size_t pos, Frame::ptr new_node);
    void ReplaceChild(Frame::ptr node, Frame::ptr new_node);
    bool ContainsChild(Frame::ptr node) const;
private:
    std::vector<Frame::ptr> childs_;
    Orientation orient_;

    std::vector<Frame::ptr>::iterator FindChild(Frame::ptr node);
};