#pragma once

#include <arena.h>
#include <cstddef>
#include <limits>
#include <memory>
#include <new>

namespace detail {
struct ArenaDtor final {
    void operator()(Arena* arena);
};
}

template <class T>
struct ArenaAllocator {
    typedef T value_type;

    ArenaAllocator(size_t capacity)
        : _capacity(capacity) {
        _arena = std::unique_ptr<Arena, detail::ArenaDtor>(arena_create(capacity));
    }

    template <class U>
    constexpr ArenaAllocator(const ArenaAllocator<U>&) noexcept { }

    [[nodiscard]] T* allocate(size_t n) {
        if (n > std::numeric_limits<size_t>::max() / sizeof(T))
            throw std::bad_array_new_length();

        arena
        if (auto p = static_cast<T*>(arena_alloc(_arena.get(), n * sizeof(T)))) {
            return p;
        }

        throw std::bad_alloc();
    }

    void deallocate(T* p, size_t n) noexcept {
        report(p, n, 0);
        std::free(p);
    }

private:
    size_t _capacity;
    std::unique_ptr<Arena, detail::ArenaDtor> _arena;
};
