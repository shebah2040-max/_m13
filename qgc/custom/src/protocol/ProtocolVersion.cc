#include "ProtocolVersion.h"

#include <sstream>

namespace m130::protocol {

std::string ProtocolVersion::toString() const
{
    std::ostringstream os;
    os << static_cast<int>(major) << '.'
       << static_cast<int>(minor) << '.'
       << static_cast<int>(patch);
    return os.str();
}

CompatReport checkCompat(ProtocolVersion peer)
{
    CompatReport r;
    r.peer = peer;
    if (peer.major != kSupported.major) {
        r.compat = VersionCompat::MajorMismatch;
        r.message = "major version mismatch (peer=" + peer.toString()
                  + ", gcs=" + kSupported.toString() + ")";
        return r;
    }
    if (peer.minor < kMinimumMinor) {
        r.compat = VersionCompat::MinorTooOld;
        r.message = "peer minor below minimum";
        return r;
    }
    if (peer.patch != kSupported.patch) {
        r.compat = VersionCompat::PatchDelta;
        r.message = "patch delta (peer=" + peer.toString() + ")";
        return r;
    }
    r.compat = VersionCompat::Compatible;
    return r;
}

} // namespace m130::protocol
