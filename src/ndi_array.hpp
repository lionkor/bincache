#pragma once

#include <algorithm>
#include <cstddef>
#include <span>
#include <type_traits>

template <typename T>
    requires std::is_trivially_default_constructible_v<T>
class NdiArray {
public:
    explicit NdiArray(size_t size)
        : _data(new T[size])
        , _size(size) {
    }

    NdiArray(const NdiArray& other)
        : NdiArray(other._size) {
        std::copy_n(other._data, _size, _data);
    }

    explicit NdiArray(const std::span<T> array)
        : _data(new T[array.size()])
        , _size(array.size()) {
        std::copy_n(array.data(), _size, _data);
    }

    explicit NdiArray(const std::span<const T> array)
        : _data(new T[array.size()])
        , _size(array.size()) {
        std::copy_n(array.data(), _size, _data);
    }

    NdiArray& operator=(const NdiArray& other) {
        if (this == &other) {
            return *this;
        }
        delete[] _data;
        _size = other._size;
        // yes owning pointer bad, go away
        // NOLINTNEXTLINE
        _data = new T[_size];
        std::copy_n(other._data, _size, _data);
        return *this;
    }

    NdiArray(NdiArray&& other) noexcept
        : _data(other._data)
        , _size(other._size) {
        other._data = nullptr;
        other._size = 0;
    }

    NdiArray& operator=(NdiArray&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        delete[] _data;
        _data = other._data;
        _size = other._size;
        other._data = nullptr;
        other._size = 0;
        return *this;
    }

    std::span<T> span() {
        return std::span(_data, _size);
    }

    std::span<const T> span() const {
        return std::span(_data, _size);
    }

    ~NdiArray() {
        delete[] _data;
        _size = 0;
    }

    [[nodiscard]]
    size_t size() const {
        return _size;
    }

    T* data() {
        return _data;
    }

    const T* data() const {
        return _data;
    }

    void clear(T default_value = { }) {
        std::fill_n(_data, _size, default_value);
    }

private:
    T* _data;
    size_t _size;
};
