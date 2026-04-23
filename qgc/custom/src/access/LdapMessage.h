#pragma once

#include "Ber.h"
#include "LdapFilter.h"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace m130::access::ldap {

/// Subset of the LDAPv3 protocol tag numbers (RFC 4511 §4.2/§4.5).
namespace op {
constexpr std::uint8_t kBindRequest        = 0;
constexpr std::uint8_t kBindResponse       = 1;
constexpr std::uint8_t kUnbindRequest      = 2;
constexpr std::uint8_t kSearchRequest      = 3;
constexpr std::uint8_t kSearchResultEntry  = 4;
constexpr std::uint8_t kSearchResultDone   = 5;
} // namespace op

/// Scope values for SearchRequest (RFC 4511 §4.5.1).
enum class Scope : std::uint8_t {
    BaseObject   = 0,
    SingleLevel  = 1,
    WholeSubtree = 2,
};

/// RFC 4511 §4.1.9 ResultCode values we care about.
enum class ResultCode : std::int32_t {
    Success                  = 0,
    OperationsError          = 1,
    ProtocolError            = 2,
    InvalidCredentials       = 49,
    UnwillingToPerform       = 53,
    Unknown                  = -1,
};

/// Simple-auth BindRequest (RFC 4511 §4.2).
struct BindRequest {
    std::int64_t message_id = 0;
    std::int32_t version    = 3;
    std::string  dn;
    std::string  password;
};

/// BindResponse subset (RFC 4511 §4.2.2).
struct BindResponse {
    std::int64_t message_id    = 0;
    ResultCode   result_code   = ResultCode::Unknown;
    std::string  matched_dn;
    std::string  diagnostic;
};

/// SearchRequest wrapper (RFC 4511 §4.5.1) with the filter kept as a
/// parsed `LdapFilter`. The codec serialises `filter` directly.
struct SearchRequest {
    std::int64_t message_id = 0;
    std::string  base_dn;
    Scope        scope = Scope::WholeSubtree;
    std::int32_t size_limit = 0;
    std::int32_t time_limit = 0;
    bool         types_only = false;
    LdapFilter   filter;
    std::vector<std::string> attributes;
};

/// SearchResultEntry (RFC 4511 §4.5.2).
struct SearchResultEntry {
    std::int64_t message_id = 0;
    std::string  dn;
    std::unordered_map<std::string, std::vector<std::string>> attrs;
};

/// SearchResultDone (RFC 4511 §4.5.2).
struct SearchResultDone {
    std::int64_t message_id  = 0;
    ResultCode   result_code = ResultCode::Unknown;
    std::string  matched_dn;
    std::string  diagnostic;
};

/// Encode an LDAPv3 request envelope for each known operation. Each
/// produces a ready-to-send byte blob with the outer SEQUENCE.
std::vector<std::uint8_t> encodeBindRequest(const BindRequest& req);
std::vector<std::uint8_t> encodeSearchRequest(const SearchRequest& req);
std::vector<std::uint8_t> encodeUnbindRequest(std::int64_t message_id);

/// Serialize a parsed `LdapFilter` to BER using the Filter choice
/// (context-specific tags 0..9). Only the ops we support in parsing are
/// emitted (AND/OR/NOT/equality/presence). Other ops throw a logic error
/// because they cannot be constructed by the parser.
std::vector<std::uint8_t> encodeFilter(const LdapFilter& filter);

/// Parse a single LDAPMessage from @p data. Returns std::nullopt when the
/// buffer does not contain a full envelope.
struct AnyResponse {
    std::uint8_t      op = 0xFF;
    std::int64_t      message_id = 0;
    BindResponse      bind;
    SearchResultEntry entry;
    SearchResultDone  done;
};

std::optional<AnyResponse> decodeMessage(const std::vector<std::uint8_t>& data,
                                         std::size_t& consumed);

} // namespace m130::access::ldap
