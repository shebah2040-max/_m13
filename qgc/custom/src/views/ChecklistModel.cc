#include "ChecklistModel.h"

#include <algorithm>

namespace m130::views {

namespace {
std::uint64_t nowMsDefault() { return 0; } // intentional: unit tests inject a clock
} // namespace

bool ChecklistModel::add(const ChecklistItem& item)
{
    if (item.id.empty()) return false;
    if (std::any_of(_items.begin(), _items.end(),
                    [&](const auto& it) { return it.id == item.id; })) {
        return false;
    }
    _items.push_back(item);
    return true;
}

bool ChecklistModel::remove(const std::string& id)
{
    const auto before = _items.size();
    _items.erase(std::remove_if(_items.begin(), _items.end(),
                                [&](const auto& it) { return it.id == id; }),
                 _items.end());
    return _items.size() != before;
}

void ChecklistModel::reset()
{
    for (auto& it : _items) {
        it.status = ChecklistStatus::Pending;
        it.completed_by.clear();
        it.completed_at_ms = 0;
        it.notes.clear();
    }
}

std::optional<ChecklistItem> ChecklistModel::find(const std::string& id) const
{
    for (const auto& it : _items) if (it.id == id) return it;
    return std::nullopt;
}

ChecklistItem* ChecklistModel::_find(const std::string& id)
{
    for (auto& it : _items) if (it.id == id) return &it;
    return nullptr;
}

std::size_t ChecklistModel::doneCount() const noexcept
{
    return static_cast<std::size_t>(std::count_if(_items.begin(), _items.end(),
        [](const auto& it) { return it.status == ChecklistStatus::Done; }));
}

std::size_t ChecklistModel::pendingCount() const noexcept
{
    return static_cast<std::size_t>(std::count_if(_items.begin(), _items.end(),
        [](const auto& it) { return it.status == ChecklistStatus::Pending; }));
}

std::size_t ChecklistModel::blockedCount() const noexcept
{
    return static_cast<std::size_t>(std::count_if(_items.begin(), _items.end(),
        [](const auto& it) { return it.status == ChecklistStatus::Blocked; }));
}

bool ChecklistModel::markDone(const std::string& id, const std::string& user,
                              access::Role role, const std::string& notes)
{
    auto* it = _find(id);
    if (!it) return false;
    if (it->status == ChecklistStatus::Blocked) return false;
    if (it->requires_auth) {
        if (user.empty()) return false;
        if (static_cast<int>(role) < static_cast<int>(it->required_role)) return false;
    }
    it->status          = ChecklistStatus::Done;
    it->completed_by    = user;
    it->completed_at_ms = _clock ? _clock() : nowMsDefault();
    it->notes           = notes;
    return true;
}

bool ChecklistModel::skip(const std::string& id, const std::string& user,
                          const std::string& reason)
{
    auto* it = _find(id);
    if (!it) return false;
    if (reason.empty()) return false;
    if (it->status == ChecklistStatus::Blocked) return false;
    it->status          = ChecklistStatus::Skipped;
    it->completed_by    = user;
    it->completed_at_ms = _clock ? _clock() : nowMsDefault();
    it->notes           = reason;
    return true;
}

bool ChecklistModel::block(const std::string& id, const std::string& reason)
{
    auto* it = _find(id);
    if (!it) return false;
    it->status = ChecklistStatus::Blocked;
    it->notes  = reason;
    return true;
}

bool ChecklistModel::isReadyForLaunch() const noexcept
{
    if (_items.empty()) return false;
    return std::all_of(_items.begin(), _items.end(),
        [](const auto& it) { return it.status == ChecklistStatus::Done; });
}

} // namespace m130::views
