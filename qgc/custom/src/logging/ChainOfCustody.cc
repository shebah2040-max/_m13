#include "ChainOfCustody.h"

#include "Hmac.h"
#include "Sha256.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace m130::logging {

namespace {
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
    return crypto::HmacSha256::macHex(key.hex, content);
}

std::string ChainOfCustody::computeManifestHash(const Manifest& m) const
{
    std::ostringstream os;
    os << m.path << '|' << m.size_bytes << '|' << m.created_at_ms << '|'
       << m.mac << '|' << m.mac_alg << '|' << m.key_id << '|'
       << m.content_sha256 << '|' << m.prev_manifest_hash;
    return crypto::Sha256::toHex(crypto::Sha256::hash(os.str()));
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
    m.mac_alg = "HMAC-SHA256";
    m.key_id = key->id;
    m.content_sha256 = crypto::Sha256::toHex(crypto::Sha256::hash(content));
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
        const std::string content = buf.str();
        if (computeMac(content, k) != m.mac) return false;
        if (!m.content_sha256.empty()) {
            const auto digest = crypto::Sha256::toHex(crypto::Sha256::hash(content));
            if (digest != m.content_sha256) return false;
        }
        return true;
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
