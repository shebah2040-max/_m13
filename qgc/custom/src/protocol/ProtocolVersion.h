#pragma once

#include <cstdint>
#include <string>

namespace m130::protocol {

/// Semantic version packed as Major<<16 | Minor<<8 | Patch (mirrors m130.xml
/// M130_HEARTBEAT_EXTENDED.protocol_version).
struct ProtocolVersion {
    uint8_t major = 0;
    uint8_t minor = 0;
    uint8_t patch = 0;

    constexpr uint32_t packed() const noexcept
    {
        return (uint32_t(major) << 16) | (uint32_t(minor) << 8) | uint32_t(patch);
    }

    static constexpr ProtocolVersion unpack(uint32_t v)
    {
        ProtocolVersion p;
        p.major = static_cast<uint8_t>((v >> 16) & 0xff);
        p.minor = static_cast<uint8_t>((v >> 8) & 0xff);
        p.patch = static_cast<uint8_t>(v & 0xff);
        return p;
    }

    std::string toString() const;
};

/// Version currently supported by this GCS build.
constexpr ProtocolVersion kSupported{ 1, 0, 0 };

/// Minimum Minor on the same Major we accept. Lower Minor is rejected with
/// a Version advisory (peer too old for us to safely decode).
constexpr uint8_t kMinimumMinor = 0;

enum class VersionCompat : uint8_t {
    Compatible       = 0,
    MajorMismatch    = 1,   ///< reject all messages from peer
    MinorTooOld      = 2,   ///< warn; peer may lack fields
    PatchDelta       = 3,   ///< info
};

struct CompatReport {
    VersionCompat compat = VersionCompat::Compatible;
    ProtocolVersion peer;
    std::string message;
};

/// Check @p peer against `kSupported`.
CompatReport checkCompat(ProtocolVersion peer);

} // namespace m130::protocol
