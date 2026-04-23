#pragma once

#include "FlightDataRecorder.h"

#include <fstream>
#include <optional>
#include <string>
#include <vector>

namespace m130::logging {

/// Streaming reader for `.m130raw` files produced by FlightDataRecorder.
///
/// Record layout (little-endian on target platforms):
///   uint64 ts_us | uint8 channel | uint16 msg_id | uint32 payload_len | payload[*]
class FdrRawReader
{
public:
    /// Open a raw file. Returns false if the file is missing or empty.
    bool open(const std::string& path);

    /// Read the next frame. Returns std::nullopt on EOF or on a truncated
    /// trailing record (which is treated as a benign end-of-stream).
    std::optional<RawFrame> next();

    bool isOpen() const noexcept { return _in.is_open(); }
    std::uint64_t framesRead() const noexcept { return _read; }

    /// Convenience: slurp all frames (bounded by @p max; 0 = no bound).
    std::vector<RawFrame> readAll(std::size_t max = 0);

private:
    std::ifstream _in;
    std::uint64_t _read = 0;
};

/// Reader for `.m130events` (JSON-lines). Returns parsed EventRecord objects.
class FdrEventReader
{
public:
    bool open(const std::string& path);
    std::optional<EventRecord> next();
    std::vector<EventRecord> readAll();

private:
    std::ifstream _in;
};

} // namespace m130::logging
