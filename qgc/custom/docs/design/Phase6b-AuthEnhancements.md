# Phase 6b — Access layer enhancements

This document describes the stdlib-only extensions made to the Pillar 6
Access layer in Phase 6b. The goal was to plug three industry-standard
capabilities into the existing pure-C++ core without introducing any new
external dependencies:

1. **Argon2id (RFC 9106)** password hashing
2. **LDAP client primitives** — DN parser (RFC 4514), filter
   parser/evaluator (RFC 4515), pluggable transport interface, in-memory
   test double, and an `IAuthenticator` binding.
3. **mTLS certificate policy** — abstract X.509 field record, configurable
   policy evaluator, `IChannelSecurity` interface, and a role-mapped
   `IAuthenticator` binding.

Everything is **interface-driven**: the real transports (OpenLDAP /
QtNetwork / QSslSocket / OpenSSL) can be dropped in later without touching
core logic or breaking any existing tests.

---

## 1. Argon2id

### Cryptographic primitives

- `access/Blake2b.{h,cc}` — full RFC 7693 BLAKE2b (keyed + variable-length
  digest 1..64 bytes, incremental hashing).
- `access/Argon2.{h,cc}` — Argon2id only, pure C++20 stdlib. Implements:
  - `H'` variable-length hash (iterated BLAKE2b for outputs > 64 B).
  - `G` compression function (fill-block, with XORing pass for
    iterations > 1).
  - Address generator for data-independent slices (pass 0, slices 0–1 in
    Argon2id).
  - Final lane XOR + `H'` tag production.

### `IPasswordHasher` binding

`access/PasswordHasher.{h,cc}` adds `Argon2idHasher` next to the existing
`Pbkdf2Hasher`. Encoded form is the canonical Argon2 PHC string:

```
argon2id$v=19$m=<memory_kib>,t=<iterations>,p=<parallelism>$<salt_b64>$<hash_b64>
```

Defaults follow OWASP 2024 for interactive login: `m = 19456 KiB`, `t = 2`,
`p = 1`. The `argon2idHasher()` factory returns a `shared_ptr<IPasswordHasher>`.

`UserManager::setHasher()` allows swapping the hasher at runtime; existing
stored hashes remain verifiable by the hasher that produced them because
`verify()` routes on the encoded prefix.

`AccessController::enableArgon2id()` is the Q_INVOKABLE entry point from
QML / application code.

### Tests

`tests/core/Blake2bTest.cc` — RFC 7693 Appendix A vectors + incremental
invariants. `tests/core/Argon2Test.cc` — Argon2id hash / verify / salt
sensitivity + `Argon2idHasher` roundtrip at reduced parameters (for CI
speed).

---

## 2. LDAP client primitives

### DN parser — RFC 4514

`access/LdapDn.{h,cc}` parses distinguished names such as
`uid=alice,ou=People,dc=m130,dc=local` into a vector of `RdnAttr{name, value}`.
It handles:

- Comma-separated RDNs with `\` escapes (hex `\\2C` and literal `\,`).
- Case-folded attribute names (`uid`, `CN`, `dc`, …).
- Leading/trailing space trimming per RFC 4514 §2.4.
- `canonical()` serialiser that re-escapes LDAP special characters.

### Filter parser/evaluator — RFC 4515

`access/LdapFilter.{h,cc}` supports exactly the filter operators required
for role lookup:

| Operator | Grammar                     | Supported |
|----------|-----------------------------|-----------|
| AND      | `(& F1 F2 …)`               | yes       |
| OR       | `(\| F1 F2 …)`              | yes       |
| NOT      | `(! F)`                     | yes       |
| equality | `(attr=value)`              | yes       |
| presence | `(attr=*)`                  | yes       |
| approx   | `(attr~=value)`             | rejected at parse time |
| `<=`     | `(attr<=value)`             | rejected at parse time |
| `>=`     | `(attr>=value)`             | rejected at parse time |

Evaluation is case-insensitive per LDAP attribute matching rules for the
attributes the authenticator uses (`uid`, `objectclass`, `memberof`).

### Transport interface

`access/LdapAuthenticator.h` defines `ILdapTransport` with two methods:

```cpp
virtual bool bind(std::string_view dn, std::string_view password) = 0;
virtual std::vector<LdapEntry> search(std::string_view base_dn,
                                      const LdapFilter& filter) = 0;
```

`InMemoryLdap` is the in-tree test double; a production implementation
(`QLdapTransport` wrapping OpenLDAP / QtNetwork) can be dropped in later
without changes to `LdapAuthenticator`.

### `LdapAuthenticator`

- Substitutes `{user}` in the DN template to form the bind DN.
- Binds and — on success — searches for the user entry, reads its
  `memberOf` values, and maps them through `LdapConfig::role_rules` to a
  `Role`.
- Returns `AuthStatus::PolicyViolation` when the user binds successfully
  but matches none of the configured rules. Returns
  `AuthStatus::BadPassword` for bind failures (mirrors `LocalAuthenticator`).

`AccessController::setLdapProvider()` installs an authenticator alongside
the existing local one.

### Tests

- `LdapDnTest.cc` — happy path + malformed input rejection.
- `LdapFilterTest.cc` — AND/OR/NOT/presence, rejection of `~=`/`<=`/`>=`,
  case-insensitive equality.
- `LdapAuthenticatorTest.cc` — role assignment from `memberOf`, bad
  password, unknown user, "no rule matches" → PolicyViolation, chain
  integration.

---

## 3. mTLS certificate policy

### Cert field record

`access/CertPolicy.h` declares `X509CertFields`:

```cpp
struct X509CertFields {
    std::string                           subject_cn;
    std::string                           issuer_cn;
    std::vector<std::string>              subject_alt_names;
    std::vector<std::string>              extended_key_usages;
    std::string                           fingerprint_sha256_hex;
    std::chrono::system_clock::time_point not_before;
    std::chrono::system_clock::time_point not_after;
    std::vector<std::string>              issuer_chain;
};
```

Populated by whatever TLS layer the application binds to — keeping the
struct free of library types ensures policy evaluation stays fully unit
testable.

### `CertPolicy` evaluator

Rules (any or all may be active):

- **Pinning** — `pinFingerprint(sha256_hex)`; optional `setRequirePinned(true)`
  rejects certs whose fingerprint is not pinned.
- **SAN allow-list** — `addAllowedSan(san)`; at least one SAN must match.
- **Required EKUs** — `addRequiredEku(eku)`; each listed EKU must appear.
- **Trusted issuers** — `addTrustedIssuer(cn)`; at least one chain link
  (including the direct issuer) must match.
- **Validity window** — `now ∈ [not_before, not_after]`.
- **Role mapping** — `mapRole(subject_cn, role)`; used by
  `MtlsAuthenticator`.

Return codes (`CertDecision`): `Allow`, `Pinned`, `NotPinned`, `Expired`,
`NotYetValid`, `SanMismatch`, `MissingEku`, `UntrustedIssuer`,
`MissingFingerprint`.

### `IChannelSecurity` + `PolicyEnforcer`

Abstract interface for applying a policy. `PolicyEnforcer` is the thin
concrete adapter holding a `CertPolicy` instance. `AccessController` stores
a `shared_ptr<IChannelSecurity>` to allow replacing the policy source
without churning callers.

### `MtlsAuthenticator`

`IAuthenticator` that:

1. Skips (`NotHandled`) when no subject CN is present — keeps the
   authenticator chain usable for password-only logins.
2. When the TLS layer has called `setPresentedCert`, evaluates the cert
   through the channel and rejects with `PolicyViolation` on failure.
3. Maps `subject_cn → Role` via `IChannelSecurity::lookupRole`. Missing
   mappings return `PolicyViolation`.

### Tests

`tests/core/CertPolicyTest.cc` covers:

- pass-through policy,
- fingerprint pin marking,
- `require_pinned` rejection of unpinned certs,
- expired / not-yet-valid clock windows,
- SAN allow-list miss/hit,
- missing EKU,
- untrusted issuer → trusted-issuer match,
- `MtlsAuthenticator` role assignment,
- `MtlsAuthenticator` policy-failure rejection,
- `MtlsAuthenticator` skip when no subject CN present.

---

## 4. CI / test metrics

| Suite                         | Before Phase 6b | After Phase 6b |
|-------------------------------|-----------------|----------------|
| `m130_core` pure-C++ tests    | 35              | 41             |

New executables: `Blake2bTest`, `Argon2Test`, `LdapDnTest`,
`LdapFilterTest`, `LdapAuthenticatorTest`, `CertPolicyTest`.

No regression to the Phase B Qt wrappers or to the existing 35 tests.

---

## 5. Deferred work

- Concrete `ILdapTransport` implementation over OpenLDAP / QtNetwork,
  including StartTLS, referral handling, connection pooling.
- Concrete `IChannelSecurity` glue around `QSslSocket`/`QSslCertificate`
  that populates `X509CertFields` from the live peer cert.
- Kerberos/GSSAPI single sign-on.
- Q_INVOKABLE JSON config loader for LDAP + mTLS (requires a settings
  schema).
