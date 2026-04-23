#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

namespace m130::access {

/// Abstract password hashing interface. Implementations MUST be
/// deterministic given (password, salt) and MUST return a self-describing
/// string so verification can round-trip without a side-channel.
///
/// Format is `<id>$<params>$<salt_b64>$<hash_b64>`, e.g.:
///   `pbkdf2-sha256$i=600000$LzqE2I6WqsY$NdAFg...`
class IPasswordHasher
{
public:
    virtual ~IPasswordHasher() = default;

    /// Hash @p password. @p salt is raw bytes; implementations base64-encode
    /// it in the output. If @p salt is empty the implementation may generate
    /// one (it is free to use a seed for tests).
    virtual std::string hash(std::string_view password, std::string_view salt) const = 0;

    /// Constant-time verification against a previously produced hash.
    virtual bool verify(std::string_view password, std::string_view encoded) const = 0;

    /// Stable tag used in the encoded output ("pbkdf2-sha256", "argon2id", …).
    virtual std::string id() const = 0;
};

/// PBKDF2-HMAC-SHA-256 hasher. Default for M130 GCS until libsodium /
/// libargon2 is authorised for the build.
class Pbkdf2Hasher final : public IPasswordHasher
{
public:
    explicit Pbkdf2Hasher(std::uint32_t iterations = 600000) noexcept
        : _iterations(iterations) {}

    std::string id() const override { return "pbkdf2-sha256"; }
    std::string hash(std::string_view password, std::string_view salt) const override;
    bool        verify(std::string_view password, std::string_view encoded) const override;

private:
    std::uint32_t _iterations;
};

/// Default hasher factory — used when UserManager is default-constructed.
std::shared_ptr<IPasswordHasher> defaultHasher();

} // namespace m130::access
