#include "io_poller.h"

#include <cassert>
#include <cerrno>
#include <cstddef>
#include <limits>
#include <system_error>

#include <unistd.h>

#include "io_clock.h"


namespace siren {

namespace {

const unsigned int IOEventFlags[2] = {
    EPOLLIN,
    EPOLLOUT
};

}

void
IOPoller::initialize()
{
    fd_ = epoll_create1(0);

    if (fd_ < 0) {
        throw std::system_error(errno, std::system_category(), "epoll_create1() failed");
    }
}


void
IOPoller::finalize()
{
    if (fd_ >= 0) {
        if (close(fd_) < 0 && errno != EINTR) {
            throw std::system_error(errno, std::system_category(), "close() failed");
        }

        for (Object *object : objects_) {
            if (object != nullptr) {
                object->~IOObject();
            }
        }
    }
}


void
IOPoller::createObject(int objectFd)
{
    assert(fd_ >= 0);
    assert(objectFd >= 0);
    assert(!objectExists(objectFd));

    if (static_cast<std::size_t>(objectFd) >= objects_.size()) {
        objects_.resize(objectFd + 1, nullptr);
    }

    auto object = new (objectMemoryPool_.allocateBlock()) Object;
    object->eventFlags = 0;
    object->pendingEventFlags = 0;
    object->isDirty = false;
    objects_[object->fd = objectFd] = object;
}


void
IOPoller::destroyObject(int objectFd)
{
    assert(fd_ >= 0);
    assert(objectFd >= 0);
    assert(objectExists(objectFd));
    Object *object = objects_[objectFd];

    if (object->eventFlags != 0) {
        if (epoll_ctl(fd_, EPOLL_CTL_DEL, objectFd, nullptr) < 0) {
            throw std::system_error(errno, std::system_category(), "epoll_ctl() failed");
        }
    }

    if (object->isDirty) {
        object->remove();
    }

    objects_[objectFd] = nullptr;
    objectMemoryPool_.freeBlock((object->~IOObject(), object));
}


void
IOPoller::addWatcher(Watcher *watcher, int objectFd, Condition condition) noexcept
{
    assert(fd_ >= 0);
    assert(watcher != nullptr);
    assert(objectFd >= 0);
    assert(objectExists(objectFd));
    Object *object = objects_[watcher->objectFd_ = objectFd];
    auto i = static_cast<std::size_t>(watcher->condition_ = condition);
    object->watcherLists[i].addTail(watcher);

    if (watcher->isOnly()) {
        object->pendingEventFlags |= IOEventFlags[i];

        if (!object->isDirty) {
            object->isDirty = true;
            dirtyObjectList_.addTail(object);
        }
    }
}


void
IOPoller::removeWatcher(Watcher *watcher) noexcept
{
    assert(fd_ >= 0);
    assert(watcher != nullptr);
    assert(watcher->objectFd_ >= 0);
    assert(objectExists(watcher->objectFd_));
    Object *object = objects_[watcher->objectFd_];
    auto i = static_cast<std::size_t>(watcher->condition_);
    watcher->remove();

    if (object->watcherLists[i].isEmpty()) {
        object->pendingEventFlags &= ~IOEventFlags[i];

        if (!object->isDirty) {
            object->isDirty = true;
            dirtyObjectList_.addTail(object);
        }
    }
}


void
IOPoller::flushObjects()
{
    SIREN_LIST_FOREACH_REVERSE(listNode, dirtyObjectList_) {
        auto object = static_cast<Object *>(listNode);

        if (object->eventFlags != object->pendingEventFlags) {
            int op;

            if (object->eventFlags == 0) {
                op = EPOLL_CTL_ADD;
            } else {
                if (object->pendingEventFlags == 0) {
                    op = EPOLL_CTL_DEL;
                } else {
                    op = EPOLL_CTL_MOD;
                }
            }

            epoll_event event;
            event.events = object->pendingEventFlags | EPOLLET;
            event.data.ptr = object;

            if (epoll_ctl(fd_, op, object->fd, &event) < 0) {
                throw std::system_error(errno, std::system_category(), "epoll_ctl() failed");
            }

            object->eventFlags = object->pendingEventFlags;
        }

        object->isDirty = false;
    }

    dirtyObjectList_.reset();
}


void
IOPoller::getReadyWatchers(Clock *clock, std::vector<Watcher *> *watchers)
{
    assert(fd_ >= 0);
    assert(clock != nullptr);
    assert(watchers != nullptr);
    flushObjects();
    std::size_t numberOfEvents = 0;
    clock->start();

    for (;;) {
        int result = epoll_wait(fd_, events_ + numberOfEvents, events_.getLength() - numberOfEvents
                                , clock->getDueTime().count());

        if (result >= 0) {
            numberOfEvents += result;

            if (numberOfEvents == events_.getLength()) {
                clock->stop();
                events_.setLength(numberOfEvents + 1);
                clock->start();
            } else {
                clock->stop();
                break;
            }
        } else {
            if (errno != EINTR) {
                clock->stop();
                throw std::system_error(errno, std::system_category(), "epoll_wait() failed");
            }

            clock->restart();
        }
    }

    for (std::size_t i = 0; i < numberOfEvents; ++i) {
        epoll_event *event = &events_[i];
        auto object = static_cast<Object *>(event->data.ptr);

        if ((event->events & (IOEventFlags[0] | EPOLLERR | EPOLLHUP)) != 0) {
            assert(!object->watcherLists[0].isEmpty());

            SIREN_LIST_FOREACH(listNode, object->watcherLists[0]) {
                auto watcher = static_cast<Watcher *>(listNode);
                watchers->push_back(watcher);
            }
        }

        if ((event->events & (IOEventFlags[1] | EPOLLERR | EPOLLHUP)) != 0) {
            assert(!object->watcherLists[1].isEmpty());

            SIREN_LIST_FOREACH(listNode, object->watcherLists[1]) {
                auto watcher = static_cast<Watcher *>(listNode);
                watchers->push_back(watcher);
            }
        }
    }
}

}