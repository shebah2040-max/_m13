#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace m130::protocol {

/// Generic handler signature. The router is intentionally lightweight — it is
/// a dispatch table keyed by mavlink message id, with a thin opaque payload
/// view (`const void*` + length). The actual decoding lives in the concrete
/// FactGroups / CustomPlugin layer.
using MessageHandler = std::function<void(uint32_t msg_id, const void* payload, std::size_t len)>;

/// Central router for inbound MAVLink messages. Used by the `CustomPlugin`
/// mavlink interceptor AND by the logging subsystem (tee into FDR).
///
/// REQ-M130-GCS-PROT-004 — every message is tee'd to the FDR BEFORE being
/// dispatched. We expose a first-class `setTee()` so that path is explicit
/// and testable.
class MessageRouter
{
public:
    /// Register a handler for a message id.
    void on(uint32_t msg_id, MessageHandler h);

    /// Remove a handler.
    void off(uint32_t msg_id);

    /// Install a tee that observes every message prior to dispatch.
    void setTee(MessageHandler tee);

    /// Dispatch one message. Returns the number of handlers invoked (not
    /// counting the tee).
    std::size_t dispatch(uint32_t msg_id, const void* payload, std::size_t len);

    /// Observability.
    std::size_t handlerCount() const noexcept { return _by_id.size(); }
    std::uint64_t totalDispatched() const noexcept { return _dispatched; }
    std::uint64_t totalUnhandled() const noexcept { return _unhandled; }

private:
    std::unordered_map<uint32_t, std::vector<MessageHandler>> _by_id;
    MessageHandler _tee;
    std::uint64_t _dispatched = 0;
    std::uint64_t _unhandled  = 0;
};

} // namespace m130::protocol
