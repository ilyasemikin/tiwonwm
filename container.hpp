#pragma once

#include <functional>
#include <memory>
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

    int16_t GetX() const override;
    int16_t GetY() const override;
    uint16_t GetWidth() const override;
    uint16_t GetHeight() const override;

    bool IsCorrectSize(uint16_t width, uint16_t height) const override;

    void Move(int16_t x, int16_t y) override;
    void Resize(uint16_t width, uint16_t height) override;
    void MoveResize(int16_t x, int16_t y, uint16_t width, uint16_t height) override;

    void SetOrientation(Orientation orient);

    inline Orientation GetOrientation() const {
        return orient_;
    }

    inline size_t CountChilds() const {
        return childs_.size();
    }

    inline Frame::ptr GetChild(size_t pos) {
        return childs_[pos];
    }

    inline Frame::const_ptr GetChild(size_t pos) const {
        return childs_[pos];
    }

    void AddChild(Frame::ptr node, size_t pos = 0);
    void AddChildAfter(Frame::ptr after_node, Frame::ptr node);
    void RemoveChild(Frame::ptr node);
    void ReplaceChild(Frame::ptr node, Frame::ptr new_node);
    bool ContainsChild(Frame::ptr node) const;

    void ResizeChild(Frame::ptr node, int16_t px);
private:
    std::vector<Frame::ptr> childs_;
    Orientation orient_;

    std::vector<Frame::ptr>::iterator FindChild(Frame::ptr node);
    std::vector<Frame::ptr>::const_iterator FindChild(Frame::ptr node) const;
    
        std::vector<xcb_rectangle_t> GetNewFrameRect(xcb_rectangle_t rect) const;
    std::vector<xcb_rectangle_t> GetFrameWithResizedChild(size_t index, int16_t px) const;
};