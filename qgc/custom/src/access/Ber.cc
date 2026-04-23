#include "Ber.h"

#include <cstring>

namespace m130::access::ber {

namespace {

void appendBytes(std::vector<std::uint8_t>& buf, const std::uint8_t* data, std::size_t n)
{
    buf.insert(buf.end(), data, data + n);
}

} // namespace

// ===================== Encoder =====================

void Encoder::_writeLen(std::size_t n)
{
    if (n < 0x80) {
        _buf.push_back(static_cast<std::uint8_t>(n));
        return;
    }
    std::uint8_t tmp[8];
    std::size_t  count = 0;
    std::size_t  v     = n;
    while (v != 0 && count < sizeof(tmp)) {
        tmp[count++] = static_cast<std::uint8_t>(v & 0xFF);
        v >>= 8;
    }
    _buf.push_back(static_cast<std::uint8_t>(0x80 | count));
    for (std::size_t i = 0; i < count; ++i) {
        _buf.push_back(tmp[count - 1 - i]);
    }
}

void Encoder::writeRaw(std::uint8_t ident, const std::vector<std::uint8_t>& payload)
{
    _buf.push_back(ident);
    _writeLen(payload.size());
    appendBytes(_buf, payload.data(), payload.size());
}

void Encoder::writeRaw(std::uint8_t ident, std::string_view payload)
{
    _buf.push_back(ident);
    _writeLen(payload.size());
    appendBytes(_buf,
                reinterpret_cast<const std::uint8_t*>(payload.data()),
                payload.size());
}

void Encoder::writeBool(bool value)
{
    _buf.push_back(tag::kBoolean);
    _buf.push_back(0x01);
    _buf.push_back(value ? 0xFF : 0x00);
}

void Encoder::writeInt(std::int64_t value)
{
    std::uint8_t tmp[9];
    std::size_t  count = 0;

    // Minimal two's-complement big-endian encoding.
    if (value == 0) {
        tmp[0] = 0;
        count  = 1;
    } else if (value > 0) {
        std::uint64_t v = static_cast<std::uint64_t>(value);
        while (v != 0) {
            tmp[count++] = static_cast<std::uint8_t>(v & 0xFF);
            v >>= 8;
        }
        if (tmp[count - 1] & 0x80) tmp[count++] = 0x00; // keep positive sign
    } else {
        std::int64_t v = value;
        while (!(v == -1 && count > 0 && (tmp[count - 1] & 0x80))) {
            tmp[count++] = static_cast<std::uint8_t>(v & 0xFF);
            v >>= 8; // arithmetic shift on signed
            if (count >= sizeof(tmp)) break;
        }
    }

    _buf.push_back(tag::kInteger);
    _writeLen(count);
    for (std::size_t i = 0; i < count; ++i) _buf.push_back(tmp[count - 1 - i]);
}

void Encoder::writeEnum(std::int64_t value)
{
    // ENUMERATED has the same payload shape as INTEGER.
    std::size_t   tag_pos = _buf.size();
    writeInt(value);
    _buf[tag_pos] = tag::kEnumerated;
}

void Encoder::writeNull()
{
    _buf.push_back(tag::kNull);
    _buf.push_back(0x00);
}

void Encoder::writeOctetString(std::string_view value)
{
    writeRaw(tag::kOctetString, value);
}

std::size_t Encoder::startConstructed(std::uint8_t identifier)
{
    _buf.push_back(identifier);
    // Reserve a long-form length placeholder so we can always patch in-place.
    // We use a 3-byte prefix (0x82 + 2 bytes) which covers payloads up to 65535
    // bytes — enough for LDAP messages.
    const std::size_t index = _buf.size();
    _buf.push_back(0x82);
    _buf.push_back(0x00);
    _buf.push_back(0x00);
    return index;
}

void Encoder::closeConstructed(std::size_t index)
{
    const std::size_t payload = _buf.size() - index - 3;
    _buf[index + 1] = static_cast<std::uint8_t>((payload >> 8) & 0xFF);
    _buf[index + 2] = static_cast<std::uint8_t>(payload & 0xFF);
}

// ===================== Decoder =====================

bool Decoder::_readLen(std::size_t& out)
{
    if (_pos >= _size) return false;
    const std::uint8_t first = _data[_pos++];
    if ((first & 0x80) == 0) {
        out = first;
        return true;
    }
    const std::size_t nbytes = first & 0x7F;
    if (nbytes == 0 || nbytes > sizeof(std::size_t)) return false;
    if (_pos + nbytes > _size) return false;
    std::size_t v = 0;
    for (std::size_t i = 0; i < nbytes; ++i) {
        v = (v << 8) | _data[_pos++];
    }
    out = v;
    return true;
}

bool Decoder::next(std::uint8_t& identifier_out,
                   const std::uint8_t*& value_out,
                   std::size_t& length_out)
{
    if (_pos >= _size) return false;
    const std::size_t start = _pos;
    const std::uint8_t ident = _data[_pos++];
    std::size_t len = 0;
    if (!_readLen(len) || _pos + len > _size) {
        _pos = start;
        return false;
    }
    identifier_out = ident;
    value_out      = _data + _pos;
    length_out     = len;
    _pos          += len;
    return true;
}

bool Decoder::skip()
{
    std::uint8_t ident = 0;
    const std::uint8_t* value = nullptr;
    std::size_t len = 0;
    return next(ident, value, len);
}

std::optional<bool> Decoder::readBool()
{
    const std::size_t save = _pos;
    std::uint8_t ident = 0;
    const std::uint8_t* value = nullptr;
    std::size_t len = 0;
    if (!next(ident, value, len) || ident != tag::kBoolean || len != 1) {
        _pos = save;
        return std::nullopt;
    }
    return value[0] != 0x00;
}

namespace {
std::optional<std::int64_t> readIntLike(Decoder& d, std::uint8_t expected)
{
    const std::size_t save = d.position();
    std::uint8_t ident = 0;
    const std::uint8_t* value = nullptr;
    std::size_t len = 0;
    if (!d.next(ident, value, len) || ident != expected || len == 0 || len > 8) {
        // Restore position by reconstructing a new decoder is not possible;
        // the next() method already rewinds on failure, so we return
        // unconditionally here.
        (void)save;
        return std::nullopt;
    }
    std::int64_t v = (value[0] & 0x80) ? -1 : 0;
    for (std::size_t i = 0; i < len; ++i) {
        v = (static_cast<std::uint64_t>(v) << 8) | value[i];
    }
    return v;
}
} // namespace

std::optional<std::int64_t> Decoder::readInt()  { return readIntLike(*this, tag::kInteger); }
std::optional<std::int64_t> Decoder::readEnum() { return readIntLike(*this, tag::kEnumerated); }

bool Decoder::readNull()
{
    std::uint8_t ident = 0;
    const std::uint8_t* value = nullptr;
    std::size_t len = 0;
    if (!next(ident, value, len)) return false;
    return ident == tag::kNull && len == 0;
}

std::optional<std::string> Decoder::readOctetString()
{
    std::uint8_t ident = 0;
    const std::uint8_t* value = nullptr;
    std::size_t len = 0;
    if (!next(ident, value, len) || ident != tag::kOctetString) return std::nullopt;
    return std::string(reinterpret_cast<const char*>(value), len);
}

std::optional<Decoder> Decoder::readConstructed(std::uint8_t expected_ident)
{
    const std::size_t save = _pos;
    std::uint8_t ident = 0;
    const std::uint8_t* value = nullptr;
    std::size_t len = 0;
    if (!next(ident, value, len) || ident != expected_ident) {
        _pos = save;
        return std::nullopt;
    }
    return Decoder(value, len);
}

} // namespace m130::access::ber
