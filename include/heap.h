#pragma once


#include <cstddef>

#include "buffer.h"


namespace siren {

class HeapNode;


class Heap final
{
public:
    typedef HeapNode Node;

    inline explicit Heap(bool (*)(const Node &, const Node &)) noexcept;
    inline Heap(Heap &&) noexcept;
    inline Heap &operator=(Heap &&) noexcept;

    inline const Node *getTop() const noexcept;
    inline Node *getTop() noexcept;

    void insertNode(Node *);
    void removeNode(Node *) noexcept;
    void removeTop() noexcept;

private:
    bool (*const nodeOrderer_)(const Node &, const Node &);
    Buffer<Node *> nodes_;
    std::size_t numberOfNodes_;

    inline void finalize() noexcept;
    inline void initialize() noexcept;

    void siftUp(Node *, std::size_t) noexcept;
    void siftDown(Node *, std::size_t) noexcept;

    Heap(const Heap &) = delete;
    Heap &operator=(const Heap &) = delete;
};


class HeapNode
{
protected:
    inline explicit HeapNode() noexcept;
    inline HeapNode(const HeapNode &) noexcept;
    inline HeapNode(HeapNode &&) noexcept;
    inline HeapNode &operator=(const HeapNode &) noexcept;
    inline HeapNode &operator=(HeapNode &&) noexcept;

    ~HeapNode() = default;

private:
    std::size_t index_;

    inline void initialize() noexcept;
#ifndef NDEBUG
    inline bool isLinked() const noexcept;
    inline bool isUnlinked() const noexcept;
#endif

    friend Heap;
};

}


/*
 * #include "heap-inl.h"
 */


#include <cassert>
#include <utility>

#include "unsigned_to_signed.h"


namespace siren {

Heap::Heap(bool (*nodeOrderer)(const Node &, const Node &)) noexcept
  : nodeOrderer_(nodeOrderer),
    numberOfNodes_(0)
{
    assert(nodeOrderer_ != nullptr);
}


Heap::Heap(Heap &&other) noexcept
  : nodeOrderer_(other.nodeOrderer_),
    nodes_(std::move(other.nodes_)),
    numberOfNodes_(other.numberOfNodes_)
{
    other.initialize();
}


Heap &
Heap::operator=(Heap &&other) noexcept
{
    if (&other != this) {
        finalize();
        nodes_ = std::move(other.nodes_);
        numberOfNodes_ = other.numberOfNodes_;
        other.initialize();
    }

    return *this;
}


void
Heap::finalize() noexcept
{
#ifndef NDEBUG
    for (std::size_t i = 0; i < numberOfNodes_; ++i) {
        nodes_[i]->initialize();
    }
#endif
}


void
Heap::initialize() noexcept
{
    numberOfNodes_ = 0;
}


const Heap::Node *
Heap::getTop() const noexcept
{
    return numberOfNodes_ == 0 ? nullptr : nodes_[0];
}


Heap::Node *
Heap::getTop() noexcept
{
    return numberOfNodes_ == 0 ? nullptr : nodes_[0];
}


HeapNode::HeapNode() noexcept
#ifndef NDEBUG
  : index_(-1)
#endif
{
}


HeapNode::HeapNode(const HeapNode &other) noexcept
  : HeapNode()
{
    static_cast<void>(other);
}


HeapNode::HeapNode(HeapNode &&other) noexcept
  : HeapNode()
{
    static_cast<void>(other);
}


HeapNode &
HeapNode::operator=(const HeapNode &other) noexcept
{
    static_cast<void>(other);
    return *this;
}


HeapNode &
HeapNode::operator=(HeapNode &&other) noexcept
{
    static_cast<void>(other);
    return *this;
}


void
HeapNode::initialize() noexcept
{
#ifndef NDEBUG
    index_ = -1;
#endif
}


#ifndef NDEBUG
bool
HeapNode::isLinked() const noexcept
{
    return UnsignedToSigned(index_) >= 0;
}


bool
HeapNode::isUnlinked() const noexcept
{
    return UnsignedToSigned(index_) < 0;
}
#endif

}
