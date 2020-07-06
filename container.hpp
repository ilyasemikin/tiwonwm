#pragma once

#include <memory>
#include <string>
#include <vector>

#include "frame.hpp"
#include "utils.hpp"

class Container;

namespace ContainerStates {
    class State {
    public:
        virtual ~State();

        std::vector<xcb_rectangle_t> GetNewFrameRect(std::shared_ptr<Container> cont, xcb_rectangle_t rect);
        std::vector<xcb_rectangle_t> GetFrameWithResizedChild(std::shared_ptr<Container> cont, size_t index, int16_t px);
    protected:
        uint16_t exp_width_;
        uint16_t exp_height_;

        uint16_t cur_width_;
        uint16_t cur_height_;

        Frame::ptr cur_child_;
        Frame::ptr last_child_;

        bool exit_;

        virtual uint16_t GetNewWidth(int px) = 0;
        virtual uint16_t GetNewWdith(double mult) = 0;
        virtual uint16_t GetNewHeight(int px) = 0;
        virtual uint16_t GetNewHeight(double mult) = 0;

        virtual int16_t GetNextX(int16_t x, uint16_t width) = 0;
        virtual int16_t GetNextY(int16_t y, uint16_t height) = 0;
    };

    class Vertical : public State {
    protected:
        uint16_t GetNewWidth(int px) override;
        uint16_t GetNewWdith(double mult) override;
        uint16_t GetNewHeight(int px) override;
        uint16_t GetNewHeight(double mult) override;

        int16_t GetNextX(int16_t x, uint16_t width) override;
        int16_t GetNextY(int16_t y, uint16_t height) override;
    };

    class Horizontal : public State {
    protected:
        uint16_t GetNewWidth(int px) override;
        uint16_t GetNewWdith(double mult) override;
        uint16_t GetNewHeight(int px) override;
        uint16_t GetNewHeight(double mult) override;

        int16_t GetNextX(int16_t x, uint16_t width) override;
        int16_t GetNextY(int16_t y, uint16_t height) override;
    };
}

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

    std::shared_ptr<ContainerStates::State> state_;

    std::vector<Frame::ptr>::iterator FindChild(Frame::ptr node);
    std::vector<Frame::ptr>::const_iterator FindChild(Frame::ptr node) const;
};