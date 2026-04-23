#pragma once

#include <cstddef>
#include <stdexcept>
#include <vector>

namespace m130::analysis {

// Fixed-capacity circular buffer. Append is O(1); once full it overwrites
// the oldest sample. Iteration yields samples in chronological order via
// at(0) = oldest ... at(size()-1) = newest.
template <class T>
class RingBuffer {
public:
    explicit RingBuffer(std::size_t capacity)
        : _buf(capacity), _cap(capacity), _head(0), _size(0)
    {
        if (capacity == 0) {
            throw std::invalid_argument("RingBuffer capacity must be > 0");
        }
    }

    std::size_t capacity() const noexcept { return _cap; }
    std::size_t size()     const noexcept { return _size; }
    bool        empty()    const noexcept { return _size == 0; }
    bool        full()     const noexcept { return _size == _cap; }

    void clear() noexcept { _head = 0; _size = 0; }

    void push(const T& v)
    {
        _buf[_head] = v;
        _head = (_head + 1) % _cap;
        if (_size < _cap) {
            ++_size;
        }
    }

    const T& at(std::size_t chrono_index) const
    {
        if (chrono_index >= _size) {
            throw std::out_of_range("RingBuffer index");
        }
        const std::size_t start = (_head + _cap - _size) % _cap;
        return _buf[(start + chrono_index) % _cap];
    }

    const T& newest() const { return at(_size - 1); }
    const T& oldest() const { return at(0); }

    // Copy chronological contents into a plain vector.
    std::vector<T> snapshot() const
    {
        std::vector<T> out;
        out.reserve(_size);
        for (std::size_t i = 0; i < _size; ++i) {
            out.push_back(at(i));
        }
        return out;
    }

private:
    std::vector<T> _buf;
    std::size_t    _cap;
    std::size_t    _head;
    std::size_t    _size;
};

} // namespace m130::analysis
