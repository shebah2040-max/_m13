#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace m130::logging {

/// Signed envelope around an arbitrary file (e.g. FDR segment).
/// The envelope stores: path, size, created_at, mac, mac_alg, key_id.
///
/// REQ-M130-GCS-LOG-003 + REQ-M130-GCS-SEC-008 (key rotation).
/// Pillar 4: MAC algorithm is HMAC-SHA256 (FIPS 180-4 + RFC 2104),
/// implemented in-tree with stdlib-only C++. File hashes use SHA-256.
struct Manifest {
    std::string path;
    std::uint64_t size_bytes = 0;
    std::uint64_t created_at_ms = 0;
    std::string mac;          ///< hex (HMAC-SHA256 by default, 64 chars)
    std::string mac_alg;      ///< "HMAC-SHA256"
    std::string key_id;       ///< rotating identifier
    std::string content_sha256; ///< hex SHA-256 of the signed content (64 chars)
    std::string prev_manifest_hash;
    std::string this_manifest_hash;
};

class ChainOfCustody
{
public:
    ChainOfCustody() = default;

    /// Register a key (hex) with an id. The most recently registered key is
    /// used by default.
    void registerKey(std::string key_id, std::string key_hex);

    /// Explicitly choose which key is active.
    void setActiveKey(std::string key_id);

    /// Compute a manifest for an existing file on disk.
    std::optional<Manifest> signFile(const std::string& path);

    /// Verify a file against its manifest (recomputes MAC).
    bool verifyFile(const Manifest& m) const;

    /// Append a manifest to the chain (linked via prev_manifest_hash).
    Manifest chain(Manifest m);

    const std::vector<Manifest>& chainHistory() const noexcept { return _chain; }
    const std::string& lastManifestHash() const noexcept { return _last_hash; }
    std::size_t keysRegistered() const noexcept { return _keys.size(); }

private:
    struct KeyEntry {
        std::string id;
        std::string hex;
    };
    std::vector<KeyEntry> _keys;
    std::string _active_key_id;
    std::vector<Manifest> _chain;
    std::string _last_hash;

    const KeyEntry* activeKey() const;
    std::string computeMac(std::string_view content, const KeyEntry& key) const;
    std::string computeManifestHash(const Manifest& m) const;
};

} // namespace m130::logging
