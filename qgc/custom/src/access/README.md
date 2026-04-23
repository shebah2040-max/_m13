# `src/access/` — Identity, Roles, Sessions

| Class | Purpose | Requirements |
|---|---|---|
| `Role` | linear authority enum | ACC-001 |
| `UserManager` | user CRUD + password + TOTP verification | SEC-001/003, ACC-004 |
| `SessionManager` | bounded in-memory sessions with TTL & revocation | SEC-004/005 |

All crypto primitives are pluggable (`Hasher`, `TotpChecker`, `IdGenerator`)
so the Foundation placeholders can be swapped for argon2id / libsodium /
OS keychain without API changes.
