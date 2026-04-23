# Access Control Architecture (Pillar 6)

Role: identity, authentication, and session management for the M130 GCS.
Scope: pure-C++ libraries that compile and unit-test without Qt. Qt/QObject
facades (for QML-bound login screens and live audit viewer) land in a
follow-up PR.

Traceability:
REQ-M130-GCS-SEC-001, SEC-003, SEC-004, SEC-005, ACC-004 → **Integrated**.

---

## 1. Goals

* Back every privileged action with **strong cryptographic proof of
  identity** — passwords stored as PBKDF2-SHA-256 with per-user salt and
  600 000 iterations (OWASP 2023); TOTP for step-up.
* Make the hashing algorithm a **swap-in interface** (`IPasswordHasher`)
  so an `Argon2idHasher` backed by libsodium can replace `Pbkdf2Hasher`
  without touching `UserManager` or any call site.
* **Time-based lockout** (NIST 800-63B §5.2.2) that auto-unlocks after
  a configurable duration rather than requiring manual admin
  intervention; counters reset on first successful login.
* **Session hygiene** per NIST 800-63B §4.1.3: idle timeout (default 15
  min) + absolute timeout (default 8 h) + session-id rotation on step-up.
* A **pluggable authenticator chain** so LDAP/AD, mTLS cert-based, and
  future providers slot in alongside the built-in local provider.

## 2. Components

| Component                   | File                             | Purpose                                                        |
|-----------------------------|----------------------------------|----------------------------------------------------------------|
| `crypto::Base32`            | `access/Base32.{h,cc}`           | RFC 4648 base32 encode/decode (TOTP secret transport).         |
| `crypto::Totp`              | `access/Totp.{h,cc}`             | RFC 4226 HOTP + RFC 6238 TOTP over HMAC-SHA-256.              |
| `crypto::Pbkdf2`            | `access/Pbkdf2.{h,cc}`           | RFC 8018 PBKDF2-HMAC-SHA-256 derivation.                      |
| `IPasswordHasher`           | `access/PasswordHasher.{h,cc}`   | Abstract hasher; `Pbkdf2Hasher` is the default implementation. |
| `UserManager`               | `access/UserManager.{h,cc}`      | Local identity store; password policy; timed lockout.          |
| `SessionManager`            | `access/SessionManager.{h,cc}`   | Idle + absolute TTL; step-up; session rotation.                |
| `AuthenticatorChain`        | `access/Authenticator.{h,cc}`    | Ordered list of `IAuthenticator`; `LocalAuthenticator` built-in. |

## 3. Password storage

`Pbkdf2Hasher::hash(password, salt)` returns a **self-describing** string:

```
pbkdf2-sha256$i=600000$<base64 salt>$<base64 32-byte DK>
```

`verify` parses this format (never a side-channel lookup), recomputes with
the encoded iteration count, and performs constant-time comparison on
the derived key. The encoding is compatible in spirit with `passlib` /
Python's `passlib.hash.pbkdf2_sha256` — swapping Python-managed users into
the M130 store requires only the hash string.

`Pbkdf2Hasher` accepts a custom iteration count in tests (e.g. 1 024 to
keep unit tests fast). Production code uses the default of 600 000.

### Argon2id path

`IPasswordHasher::id()` distinguishes stored hashes. When libsodium or
libargon2 is authorised for the build, `Argon2idHasher` will emit
`argon2id$m=…$t=…$p=…$<salt>$<hash>` and coexist with existing
`pbkdf2-sha256` records; `UserManager` picks the verifier by prefix. No
user data needs rehashing.

## 4. TOTP

`crypto::Totp::totp(secret, t, step_s=30, digits=6)` returns the zero-
padded decimal code for window `floor(t/step_s)` using HMAC-SHA-256 and
RFC 4226 §5.3 dynamic truncation. `verify(..., lookback=1)` accepts the
current window plus `±lookback` neighbours to absorb small clock drift.

Test coverage: `tests/core/TotpTest.cc` checks the RFC 6238 Appendix B
SHA-256 vectors (`46119246`, `68084774`, `67062674`, `91819424`,
`90698825` at T = 59 / 1111111109 / 1111111111 / 1234567890 / 2000000000).

Code comparison is constant-time to prevent timing oracles.

### Why SHA-256

RFC 6238 explicitly allows SHA-1 / SHA-256 / SHA-512. We chose SHA-256
for the stronger hash and because modern authenticator apps (Google
Authenticator 2023+, Authy, 1Password, Aegis, Raivo) all support the
`algorithm=SHA256` otpauth parameter. Legacy-only apps that require SHA-1
can be added via an `algorithm` enum on `Totp` without breaking callers.

## 5. Lockout + password policy

```cpp
LockoutPolicy { threshold = 5, duration_ms = 15 * 60 * 1000 };
PasswordPolicy { min_length = 12, require_upper/lower/digit/symbol = true };
```

After `threshold` consecutive failed logins the account is locked and
`lockout_started_ms` is stamped. `attemptPassword` treats further attempts
as `AccountLocked` until `now - lockout_started_ms >= duration_ms`, at
which point it **auto-unlocks** and evaluates the password. `setLocked`
also lets an admin lock or clear the flag manually.

`PasswordPolicy::isValid` returns the first violation in an optional
out-string for the caller to display. `setPassword` refuses to store a
hash for a policy-violating password.

## 6. Session management

```cpp
SessionManager sm;
sm.setDefaultTtlMs(8 * 60 * 60 * 1000);  // absolute TTL
sm.setIdleTtlMs(15 * 60 * 1000);         // idle TTL (0 disables)
```

`touch()` refreshes `last_active_ms` and returns the current session.
`state()` returns one of `Valid`, `NotFound`, `Expired`, `Revoked`,
`Idle`. `evictExpired()` removes both absolute- and idle-expired
sessions.

### Step-up

```cpp
std::string new_id;
if (sm.stepUp(sid, [&]{ return users.verifyTotp(user, code); }, &new_id) ==
    StepUpResult::Ok) { /* use new_id */ }
```

* The verify callback lets callers plug in any second factor (TOTP,
  WebAuthn, HSM challenge).
* On success the session id is **rotated** (OWASP session-fixation
  prevention) and `step_up_at_ms` is stamped.
* `requiresStepUp(sid, max_age_ms)` returns `true` when the last step-up
  is older than `max_age_ms` — used to gate FTS fire, user role edits,
  key rotation, and other critical operations (REQ-M130-GCS-SEC-005).

## 7. Authenticator chain

```cpp
AuthenticatorChain chain;
chain.add(std::make_shared<LocalAuthenticator>(&users));
// future: chain.add(std::make_shared<LdapAuthenticator>(ldap_cfg));
// future: chain.add(std::make_shared<MtlsAuthenticator>(trust_store));

auto r = chain.authenticate({ user, password, totp_code, mtls_cn });
```

Each backend returns `AuthStatus::NotHandled` to pass the request along
or a definitive status (`Success`, `BadPassword`, `RequiresTotp`, …) to
end the walk. `LocalAuthenticator` wraps `UserManager` and honours
the TOTP requirement flag per-user.

## 8. Deferred

| Area                                            | Landing point                                 |
|-------------------------------------------------|-----------------------------------------------|
| Concrete `LdapAuthenticator`                    | Qt wrapper PR (needs Qt Network / ldap lib)   |
| `MtlsAuthenticator` + channel security          | Pillar 9 (TLS relay integration)              |
| Argon2id hasher                                 | When libsodium/libargon2 is authorised        |
| Persistent encrypted user store                 | OS keychain / HSM integration                 |
| QML login screens + live audit viewer           | Pillar 6b (QObject wrappers + Admin console)  |
| Key rotation workflow (REQ-SEC-008)             | Pillar 9                                      |

## 9. Test coverage

29 pure-C++ unit tests (up from 25) — all green locally:

* `TotpTest` — RFC 6238 SHA-256 vectors + ± window acceptance + constant
  time verify.
* `Pbkdf2Test` — derivation determinism + known SHA-256 vector +
  `Pbkdf2Hasher` round-trip + self-describing format.
* `Base32Test` — RFC 4648 §10 vectors + case-insensitivity + invalid
  input rejection.
* `UserManagerTest` — PBKDF2-backed login, password policy rejection,
  lockout after N failures, **timed auto-unlock**, TOTP branch.
* `SessionManagerTest` — create/touch/expire, idle timeout, absolute
  TTL, step-up rotation, bad-code rejection, invalid session.
* `AuthenticatorChainTest` — chain ordering (first handled wins),
  empty chain, `LocalAuthenticator` success/TOTP flows.
