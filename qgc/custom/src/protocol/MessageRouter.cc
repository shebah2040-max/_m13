#include "MessageRouter.h"

namespace m130::protocol {

void MessageRouter::on(uint32_t msg_id, MessageHandler h)
{
    _by_id[msg_id].push_back(std::move(h));
}

void MessageRouter::off(uint32_t msg_id)
{
    _by_id.erase(msg_id);
}

void MessageRouter::setTee(MessageHandler tee)
{
    _tee = std::move(tee);
}

std::size_t MessageRouter::dispatch(uint32_t msg_id, const void* payload, std::size_t len)
{
    if (_tee) {
        _tee(msg_id, payload, len);
    }
    ++_dispatched;
    auto it = _by_id.find(msg_id);
    if (it == _by_id.end()) {
        ++_unhandled;
        return 0;
    }
    for (const auto& h : it->second) {
        if (h) h(msg_id, payload, len);
    }
    return it->second.size();
}

} // namespace m130::protocol
