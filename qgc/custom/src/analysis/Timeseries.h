#pragma once

#include "RingBuffer.h"

#include <cstdint>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace m130::analysis {

struct Sample {
    std::uint64_t t_ms;
    double        value;
};

// Chronologically ordered (t_ms strictly non-decreasing) ring-backed series.
// Intended for live plotting + FFT input sourcing.
class Timeseries {
public:
    Timeseries(std::string name, std::size_t capacity);

    const std::string& name()    const noexcept { return _name; }
    std::size_t        size()    const noexcept { return _ring.size(); }
    std::size_t        capacity()const noexcept { return _ring.capacity(); }
    bool               empty()   const noexcept { return _ring.empty(); }

    void push(std::uint64_t t_ms, double value);
    void clear() { _ring.clear(); _last_t_ms = 0; }

    const Sample& at(std::size_t i)  const { return _ring.at(i); }
    const Sample& oldest()           const { return _ring.oldest(); }
    const Sample& newest()           const { return _ring.newest(); }

    // Returns up to max_points samples uniformly spread across the current
    // contents — cheap downsampling for plot widgets that cannot display
    // more than a few thousand points per paint cycle.
    std::vector<Sample> downsample(std::size_t max_points) const;

    // Running statistics over the current window.
    double mean()   const;
    double stddev() const;
    double minVal() const;
    double maxVal() const;

private:
    std::string           _name;
    RingBuffer<Sample>    _ring;
    std::uint64_t         _last_t_ms = 0;
};

} // namespace m130::analysis
