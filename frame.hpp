#pragma once

#include <memory>

enum class FrameType {
    CONTAINER,
    WINDOW
};

class Frame : public std::enable_shared_from_this<Frame> {
public:
    virtual ~Frame();

    using ptr = std::shared_ptr<Frame>;
    using const_ptr = std::shared_ptr<const Frame>;

    virtual FrameType GetType() const = 0;

    virtual std::string ToString() const = 0;

    virtual int16_t GetX() const = 0;
    virtual int16_t GetY() const = 0;
    virtual uint16_t GetWidth() const = 0;
    virtual uint16_t GetHeight() const = 0;

    virtual bool IsCorrectSize(uint16_t width, uint16_t height) const = 0;

    virtual void Move(int16_t x, int16_t y) = 0;
    virtual void Resize(uint16_t width, uint16_t height) = 0;
    virtual void MoveResize(int16_t x, int16_t y, uint16_t width, uint16_t height) = 0;

    inline void SetParent(ptr node) {
        parent_ = node;
    }

    inline ptr GetParent() {
        return parent_.lock();
    }

    inline const_ptr GetParent() const {
        return parent_.lock();
    }
protected:
    Frame();
private:
    std::weak_ptr<Frame> parent_;
};