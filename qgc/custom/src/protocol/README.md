# `src/protocol/` — Message Dispatch

| Class | Purpose | Requirements |
|---|---|---|
| `ProtocolVersion` | major.minor.patch + compat check | PROT-003 |
| `MessageRouter` | msg_id → handler(s) + tee to FDR | PROT-004 |
| `M130Dialect` | descriptors for m130 dialect ids | PROT-002 |

Router is intentionally decoupled from Qt and MAVLink types so we can unit-test
dispatch ordering and tee behavior in isolation. Integration with QGC's
MAVLinkProtocol happens in `CustomPlugin::mavlinkMessage()`, which forwards
`msgid` and payload to `MessageRouter::dispatch()`.
