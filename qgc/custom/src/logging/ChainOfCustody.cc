#include "ChainOfCustody.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace m130::logging {

namespace {
uint64_t fnv1a64(std::string_view data, std::string_view key = {})
{
    uint64_t h = 0xcbf29ce484222325ULL;
    for (unsigned char c : data) {
        h ^= c; h *= 0x100000001b3ULL;
    }
    for (unsigned char c : key) {
        h ^= c; h *= 0x100000001b3ULL;
    }
    return h;
}

std::string toHex64(uint64_t x)
{
    std::ostringstream os;
    os << std::hex << std::setw(16) << std::setfill('0') << x;
    return os.str();
}

uint64_t now_ms()
{
    using namespace std::chrono;
    return static_cast<uint64_t>(
        duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
}
} // namespace

void ChainOfCustody::registerKey(std::string key_id, std::string key_hex)
{
    for (auto& k : _keys) {
        if (k.id == key_id) {
            k.hex = std::move(key_hex);
            _active_key_id = key_id;
            return;
        }
    }
    _keys.push_back({ key_id, std::move(key_hex) });
    _active_key_id = std::move(key_id);
}

void ChainOfCustody::setActiveKey(std::string key_id)
{
    _active_key_id = std::move(key_id);
}

const ChainOfCustody::KeyEntry* ChainOfCustody::activeKey() const
{
    for (const auto& k : _keys) {
        if (k.id == _active_key_id) return &k;
    }
    if (!_keys.empty()) return &_keys.back();
    return nullptr;
}

std::string ChainOfCustody::computeMac(std::string_view content, const KeyEntry& key) const
{
    return toHex64(fnv1a64(content, key.hex));
}

std::string ChainOfCustody::computeManifestHash(const Manifest& m) const
{
    std::ostringstream os;
    os << m.path << '|' << m.size_bytes << '|' << m.created_at_ms << '|'
       << m.mac << '|' << m.mac_alg << '|' << m.key_id << '|'
       << m.prev_manifest_hash;
    return toHex64(fnv1a64(os.str()));
}

std::optional<Manifest> ChainOfCustody::signFile(const std::string& path)
{
    const KeyEntry* key = activeKey();
    if (!key) return std::nullopt;

    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) return std::nullopt;

    std::ostringstream buf;
    buf << in.rdbuf();
    std::string content = buf.str();

    Manifest m;
    m.path = path;
    m.size_bytes = content.size();
    m.created_at_ms = now_ms();
    m.mac = computeMac(content, *key);
    m.mac_alg = "FNV1A64"; // upgrade to HMAC-SHA256 in follow-up PR
    m.key_id = key->id;
    m.prev_manifest_hash = _last_hash;
    m.this_manifest_hash = computeManifestHash(m);
    return m;
}

bool ChainOfCustody::verifyFile(const Manifest& m) const
{
    for (const auto& k : _keys) {
        if (k.id != m.key_id) continue;
        std::ifstream in(m.path, std::ios::binary);
        if (!in.is_open()) return false;
        std::ostringstream buf;
        buf << in.rdbuf();
        return computeMac(buf.str(), k) == m.mac;
    }
    return false;
}

Manifest ChainOfCustody::chain(Manifest m)
{
    m.prev_manifest_hash = _last_hash;
    m.this_manifest_hash = computeManifestHash(m);
    _last_hash = m.this_manifest_hash;
    _chain.push_back(m);
    return m;
}

} // namespace m130::logging
