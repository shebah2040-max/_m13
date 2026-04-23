# Phase 6c — Real authentication transports

Phase 6b introduced the pure-C++ policy cores for Argon2id, LDAP, and
mTLS. Phase 6c adds **concrete transports** that plug those cores into
real wire-level systems without changing any core logic:

1. **BER/ASN.1 codec** (`access/Ber.{h,cc}`) — minimal DER encoder +
   decoder built from `<cstdint>`/`<vector>` only.
2. **LDAPv3 message codec** (`access/LdapMessage.{h,cc}`) — RFC 4511
   envelope + `BindRequest`/`BindResponse`/`SearchRequest`/
   `SearchResultEntry`/`SearchResultDone`/`UnbindRequest`, with
   `LdapFilter` serialization.
3. **`QtLdapTransport`** — `ILdapTransport` over `QTcpSocket`/
   `QSslSocket` (LDAP + LDAPS). Qt-bound.
4. **`QSslChannelAdapter`** — `IChannelSecurity` that extracts
   `X509CertFields` from `QSslCertificate` + applies a `CertPolicy`.
   Qt-bound.
5. **GSSAPI / Kerberos interface** (`access/GssAuthenticator.{h,cc}`)
   — abstract `IGssProvider`, stub + fake providers, `GssPolicy` for
   principal/realm → role mapping, and `GssAuthenticator` that plugs
   into `AuthenticatorChain`. The real KDC-backed provider is deferred
   (`libgssapi_krb5` on Linux/macOS, SSPI on Windows).

---

## 1. BER/ASN.1 codec

Implements just enough of X.690 DER for LDAPv3:

- `Encoder::writeBool/writeInt/writeEnum/writeNull/writeOctetString`.
- `Encoder::startConstructed/closeConstructed` for SEQUENCE / SET /
  context-specific / application-tagged wrappers. Length is written as
  a 3-byte long-form placeholder and patched once the children are
  emitted — keeping the API one-pass and allocation-efficient.
- `Decoder::next/readBool/readInt/readEnum/readNull/readOctetString/
  readConstructed` each returns `std::nullopt` / `false` on malformed
  input without advancing the cursor.

Covered by `tests/core/BerTest.cc` (9 cases).

## 2. LDAPv3 message codec (RFC 4511)

Requests: `encodeBindRequest`, `encodeSearchRequest`, `encodeUnbindRequest`.

Filter serialization (`encodeFilter`) emits the Filter CHOICE used by
our parser (`&`, `|`, `!`, `=`, `=*`) using context-specific tags
0/1/2/3/7 from §4.5.1.7.

Responses: `decodeMessage` returns an `AnyResponse` with a discriminated
union of `BindResponse` / `SearchResultEntry` / `SearchResultDone`,
including `consumed` bytes so callers can drive a partial-buffer socket
loop.

Covered by `tests/core/LdapMessageTest.cc` (7 cases): envelope
structure, LDAPResult codes (`Success` / `InvalidCredentials`),
`SearchResultEntry` with attribute SET, multi-attribute
`SearchRequest`, `UnbindRequest` NULL payload, filter encoding for
`!(objectClass=*)`.

## 3. `QtLdapTransport`

Synchronous LDAP client:

```cpp
QtLdapEndpoint ep;
ep.host = "ldap.example.com";
ep.port = 636;
ep.use_tls = true;

QtLdapTransport transport(ep);
LdapConfig cfg{ /* dn_template, base, rules... */ };
accessCtl.setLdapProvider(
    std::make_shared<QtLdapTransport>(ep), cfg);
```

- LDAP (389) over `QTcpSocket` or LDAPS (636) over `QSslSocket` with
  CA bundle + `PeerVerifyMode::VerifyNone` override.
- Each `bind()` / `search()` opens a fresh connection, runs a single
  operation, issues an `UnbindRequest`, and disconnects — matching the
  stateless in-memory test double.
- `search()` collects every `SearchResultEntry` until
  `SearchResultDone` is seen or the stream ends.
- **Not exercised by `m130_core` CI** (no Qt in pure-C++ CI job).
  Integration tests against `slapd` / Active Directory live in
  `tests/integration/` and run only when Qt is available.

**Deferred**: StartTLS extended operation (requires `ExtendedRequest`
BER with OID `1.3.6.1.4.1.1466.20037`), referrals, paged results,
async cancel.

## 4. `QSslChannelAdapter`

Wraps `CertPolicy` and populates `X509CertFields` from a
`QSslCertificate`:

- Subject/issuer CN via `subjectInfo(CommonName)`/
  `issuerInfo(CommonName)`.
- SANs via `subjectAlternativeNames()` (flattens DNS + email + URI).
- SHA-256 fingerprint via `digest(QCryptographicHash::Sha256)`.
- EKUs from the `extendedKeyUsage` extension.
- Validity window from `effectiveDate()`/`expiryDate()`.

The adapter stores the last captured peer cert so
`AccessController::setChannelSecurity()` can route
`MtlsAuthenticator::evaluate()` through it.

**Not covered** by `m130_core` CI for the same reason as
`QtLdapTransport`. Qt-side integration testing is enabled by a live
`QSslSocket` fixture (deferred to `tests/integration/` once QGC's Qt
test harness lands).

## 5. GSSAPI / Kerberos interface

### `IGssProvider`

```cpp
class IGssProvider {
public:
    struct AcceptResult {
        bool        complete = false;
        std::string output_token;
        std::string principal;  // "alice@REALM.LOCAL"
        std::string error;
    };
    virtual AcceptResult acceptSecurityContext(std::string_view token) = 0;
    virtual void         releaseContext() {}
};
```

- `StubGssProvider` always returns `"gss provider not configured"` so
  the authenticator chain stays compilable without Kerberos.
- `FakeGssProvider` is the in-tree test double: prearranged
  `token → principal` map used by unit tests.

### `GssPolicy`

Maps either an exact principal (case-insensitive) or a realm to a
`Role`. Unknown principals resolve to `Role::None` → authenticator
returns `AuthStatus::PolicyViolation`.

### `GssAuthenticator`

`IAuthenticator` that:

1. Returns `NotHandled` when no `gss_token` is present (keeps password
   / LDAP / mTLS flows alive).
2. Calls `acceptSecurityContext`. Provider errors → `BadPassword`;
   multi-leg negotiation (`complete=false`) → `NotHandled` so the
   caller can loop with a new token.
3. Resolves a role via `GssPolicy`. Missing role → `PolicyViolation`.
4. Success → emits `"gssapi:<principal>"` as `AuthResult::detail`.

Covered by `tests/core/GssAuthenticatorTest.cc` (7 cases).

**Deferred**: A real `KrbGssProvider` (Linux: `libgssapi_krb5`; Windows:
`SSPI/Secur32`), delegation, channel binding, SPNEGO NegTokenInit
parsing.

---

## 6. `AccessController` integration

```cpp
// Argon2id (Phase 6b)
accessCtl.enableArgon2id();

// LDAPS (Phase 6c)
auto ldap = std::make_shared<QtLdapTransport>(QtLdapEndpoint{/* host/port/tls */});
accessCtl.setLdapProvider(ldap, LdapConfig{/* dn_template, base, rules */});

// mTLS
auto ssl_channel = std::make_shared<QSslChannelAdapter>(certPolicy);
accessCtl.setChannelSecurity(ssl_channel);

// Kerberos (stub by default)
accessCtl.setGssProvider(std::make_shared<FakeGssProvider>(), gssPolicy);
```

## 7. CI impact

| Suite                         | Before 6c | After 6c |
|-------------------------------|-----------|----------|
| `m130_core` pure-C++ tests    | 41        | 44       |

New executables: `BerTest`, `LdapMessageTest`, `GssAuthenticatorTest`.

The Qt-bound files (`QtLdapTransport`, `QSslChannelAdapter`) are
compiled by the main QGC build but not by the `m130_core` CI job.

## 8. Deferred to Phase 6d

- Real `KrbGssProvider` (Kerberos KDC) and `SspiGssProvider` (Windows).
- LDAP StartTLS and paged results.
- QSslSocket CRL / OCSP revocation checks.
- Hot-reload of `CertPolicy` from a JSON settings document.
- Kerberos ticket cache monitoring (kinit / expiry renewal).
