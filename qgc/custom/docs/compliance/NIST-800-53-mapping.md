# NIST SP 800-53 Rev. 5 — Control Mapping

- **Document ID**: COMP-NIST80053-001
- **Revision**: A

## Baseline: Moderate (مناسب لأنظمة طيران تجريبية أرضية)

### AC — Access Control
| Control | Requirement | Evidence |
|---|---|---|
| AC-2 | Account Management | `src/access/UserManager.cc` |
| AC-3 | Access Enforcement | `src/safety/CommandAuthorization.cc` |
| AC-6 | Least Privilege | RBAC matrix in SRS §2.2 |
| AC-7 | Unsuccessful Logon Attempts | REQ-M130-GCS-ACC-004 (lockout) |
| AC-12 | Session Termination | `src/access/SessionManager.cc` |

### AU — Audit and Accountability
| Control | Requirement | Evidence |
|---|---|---|
| AU-2 | Audit Events | `src/logging/AuditLogger.cc` + REQ-CMD-005, ACC-002 |
| AU-3 | Content of Audit Records | AuditEntry struct (who/what/when/context) |
| AU-6 | Audit Review | Admin Console (Pillar 5) |
| AU-9 | Protection of Audit Info | Hash chain + append-only |
| AU-10 | Non-Repudiation | HMAC signing via ChainOfCustody |
| AU-11 | Audit Record Retention | ≥ 7 years (FDR retention policy) |
| AU-12 | Audit Generation | كل أمر/تنبيه يُسجَّل تلقائياً |

### IA — Identification and Authentication
| Control | Requirement | Evidence |
|---|---|---|
| IA-2 | Identification | UserManager login |
| IA-2(1) | MFA for privileged | TOTP for FTS + admin |
| IA-5 | Authenticator Management | argon2id + rotation policy |
| IA-5(1) | Password-based (argon2id) | REQ-SEC-001 |
| IA-7 | Cryptographic Modules | libsodium (قياسي) |

### SC — System and Communications Protection
| Control | Requirement | Evidence |
|---|---|---|
| SC-7 | Boundary Protection | Firewall + dedicated network |
| SC-8 | Transmission Integrity | TLS 1.3 + MAVLink 2 signing |
| SC-12 | Cryptographic Key Establishment | Key rotation procedure |
| SC-13 | Cryptographic Protection | AES-256 / SHA-256 / HMAC |
| SC-28 | Protection of Info at Rest | Disk encryption + keychain |

### SI — System and Information Integrity
| Control | Requirement | Evidence |
|---|---|---|
| SI-2 | Flaw Remediation | Patch process + CAPA |
| SI-4 | System Monitoring | Watchdog + AlertManager |
| SI-7 | Software/Firmware/Information Integrity | SBOM + signed artifacts |
| SI-10 | Information Input Validation | Bounds check in Protocol + CommandAuthorization |
| SI-11 | Error Handling | Structured error + no leakage in messages |

### CM — Configuration Management
(مُغطاة بالكامل عبر `docs/plans/SCMP.md`)

### RA — Risk Assessment
(`docs/safety/HazardLog.md` + `FMEA.md`)

### CA — Assessment, Authorization, and Monitoring
- CA-2: Security Assessments → pentests (SQAP §5)
- CA-7: Continuous Monitoring → CI + runtime watchdogs

### CP — Contingency Planning
- CP-2: Plan → documented recovery procedures (Pillar Operations)
- CP-10: System Recovery → FDR + replay + graceful restart

## Threat Model (ملخص)

| Threat | Likelihood | Impact | Primary Controls |
|---|---|---|---|
| مصادقة مختطفة | Medium | High | IA-2, IA-5, AC-7, TOTP |
| MITM على القناة | Low | High | SC-8 (TLS+MAVLink signing) |
| تلاعب بالسجلات | Low | High | AU-9 (hash chain + HMAC) |
| تسريب مفاتيح | Low | Critical | IA-5 + keychain + rotation |
| Replay | Medium | Medium | MAVLink signing + nonce + timestamp |
| DoS على GCS | Medium | Medium | SC-7 + rate limiting |
| Supply chain compromise | Low | High | SBOM + dependency review + cosign |
