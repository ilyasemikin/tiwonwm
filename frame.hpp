#pragma once

#include <memory>

enum class FrameType {
    CONTAINER,
    WINDOW
};

class Frame : public std::enable_shared_from_this<Frame> {
public:
    Frame();
    virtual ~Frame();

    using ptr = std::shared_ptr<Frame>;
    using const_ptr = std::shared_ptr<const Frame>;

    virtual FrameType GetType() const = 0;

    virtual std::string ToString() const = 0;

    inline void SetParent(ptr node) {
        parent_ = node;
    }

    inline ptr GetParent() {
        return parent_;
    }
private:
    ptr parent_;
};