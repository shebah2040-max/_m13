#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace m130::access::ber {

/// ASN.1 tag classes.
enum class Class : std::uint8_t {
    Universal       = 0x00,
    Application     = 0x40,
    ContextSpecific = 0x80,
    Private         = 0xC0,
};

/// Universal tag numbers used by LDAPv3 (RFC 4511). Only the subset we need.
namespace tag {
constexpr std::uint8_t kBoolean    = 0x01;
constexpr std::uint8_t kInteger    = 0x02;
constexpr std::uint8_t kBitString  = 0x03;
constexpr std::uint8_t kOctetString= 0x04;
constexpr std::uint8_t kNull       = 0x05;
constexpr std::uint8_t kEnumerated = 0x0A;
constexpr std::uint8_t kSequence   = 0x30; ///< SEQUENCE/SEQUENCE OF — constructed
constexpr std::uint8_t kSet        = 0x31; ///< SET/SET OF — constructed
} // namespace tag

/// Primitive / Constructed flags combined with a tag class + number.
constexpr std::uint8_t kConstructed = 0x20;

/// BER/DER identifier octet helpers.
constexpr std::uint8_t identifier(Class cls, bool constructed, std::uint8_t number)
{
    return static_cast<std::uint8_t>(static_cast<std::uint8_t>(cls)
                                     | (constructed ? kConstructed : 0)
                                     | (number & 0x1F));
}

/// Minimal DER encoder. Uses short-form length for < 128, long-form for
/// larger payloads. Streams bytes into an internal buffer.
class Encoder
{
public:
    Encoder() = default;

    const std::vector<std::uint8_t>& bytes() const noexcept { return _buf; }
    std::vector<std::uint8_t> release() { return std::move(_buf); }
    std::size_t               size()    const noexcept { return _buf.size(); }

    /// Emit an identifier octet + length prefix + the bytes of @p payload.
    void writeRaw(std::uint8_t ident, const std::vector<std::uint8_t>& payload);
    void writeRaw(std::uint8_t ident, std::string_view payload);

    /// Universal encodings.
    void writeBool(bool value);
    void writeInt(std::int64_t value);
    void writeEnum(std::int64_t value);
    void writeNull();
    void writeOctetString(std::string_view value);

    /// Start a constructed SEQUENCE / SET / application / context tag. The
    /// returned index is passed to `closeConstructed` to fill in the
    /// length once all children have been emitted.
    std::size_t startConstructed(std::uint8_t identifier);
    void        closeConstructed(std::size_t index);

private:
    void _writeLen(std::size_t n);
    std::vector<std::uint8_t> _buf;
};

/// Decoder view over an already-materialised byte buffer. Every parse
/// method returns false on malformed input and leaves the position
/// unchanged when it fails.
class Decoder
{
public:
    Decoder(const std::uint8_t* data, std::size_t size) noexcept
        : _data(data), _size(size) {}
    explicit Decoder(std::string_view s) noexcept
        : _data(reinterpret_cast<const std::uint8_t*>(s.data())), _size(s.size()) {}
    explicit Decoder(const std::vector<std::uint8_t>& v) noexcept
        : _data(v.data()), _size(v.size()) {}

    std::size_t remaining() const noexcept { return _size - _pos; }
    bool        atEnd()     const noexcept { return _pos >= _size; }
    std::size_t position()  const noexcept { return _pos; }

    /// Peek the next identifier without advancing. Returns 0 on EOF.
    std::uint8_t peekIdent() const noexcept
    { return atEnd() ? std::uint8_t{0} : _data[_pos]; }

    /// Read the next TLV. Returns false on a short buffer; otherwise
    /// stores the identifier, length, and slice of the value bytes in the
    /// out parameters.
    bool next(std::uint8_t& identifier_out,
              const std::uint8_t*& value_out,
              std::size_t&  length_out);

    /// Skip the next TLV entirely.
    bool skip();

    /// Convenience: parse a universal type with the specified tag and
    /// return its value.
    std::optional<bool>          readBool();
    std::optional<std::int64_t>  readInt();
    std::optional<std::int64_t>  readEnum();
    bool                         readNull();
    std::optional<std::string>   readOctetString();

    /// Enter a constructed wrapper: verify the identifier, and return a
    /// decoder scoped to the wrapper's value bytes. The parent's position
    /// is advanced past the wrapper.
    std::optional<Decoder>       readConstructed(std::uint8_t expected_ident);

private:
    bool _readLen(std::size_t& out);

    const std::uint8_t* _data;
    std::size_t         _size;
    std::size_t         _pos = 0;
};

} // namespace m130::access::ber
