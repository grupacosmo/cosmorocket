#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stddef.h>

#include <atomic>

template <typename T, size_t size>
class SPSCQueue {
    static_assert((size & (size - 1)) == 0, "size must be power of 2");

    alignas(64) std::atomic<size_t> head{0};
    alignas(64) std::atomic<size_t> tail{0};
    alignas(64) T buffer[size];

   public:
    bool push(const T &item) {
        size_t current_head = head.load(std::memory_order_relaxed);
        size_t next_head = (current_head + 1) & (size - 1);

        if (next_head == tail.load(std::memory_order_acquire)) {
            return false;
        }

        buffer[current_head] = item;

        head.store(next_head, std::memory_order_release);
        return true;
    }

    bool pop(T &item) {
        size_t current_tail = tail.load(std::memory_order_relaxed);

        if (current_tail == head.load(std::memory_order_acquire)) {
            return false;
        }

        item = buffer[current_tail];
        tail.store((current_tail + 1) & (size - 1), std::memory_order_release);
        return true;
    }

    size_t Size() {
        size_t current_head = head.load(std::memory_order_acquire);
        size_t current_tail = tail.load(std::memory_order_acquire);

        if (current_head >= current_tail) {
            return current_head - current_tail;
        } else {
            return size + current_head - current_tail;
        }
    }
    
    bool empty() {
        size_t current_head = head.load(std::memory_order_acquire);
        size_t current_tail = tail.load(std::memory_order_acquire);

        if (current_head == current_tail) return true;

        return false;
    }
};

#endif