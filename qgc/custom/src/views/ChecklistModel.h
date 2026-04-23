#pragma once

#include "../access/Role.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace m130::views {

enum class ChecklistStatus : std::uint8_t {
    Pending = 0,
    Done    = 1,
    Skipped = 2,
    Blocked = 3,
};

struct ChecklistItem {
    std::string id;           ///< Stable machine id (e.g. "fuel.load")
    std::string title;        ///< One-line title
    std::string description;  ///< Long description / reference
    bool        requires_auth = false;
    access::Role required_role = access::Role::None;

    ChecklistStatus status = ChecklistStatus::Pending;
    std::string completed_by;
    std::uint64_t completed_at_ms = 0;
    std::string notes;
};

/// Pre-launch checklist (REQ-M130-GCS-UI-004). Pure C++ so it can be unit
/// tested without Qt; a Qt/QML facade lives in a thin wrapper compiled only
/// with the main QGC build.
///
/// Rules:
///   - `markDone(id, user, role)` refuses items whose `requires_auth` is set
///     unless `role` >= `required_role` and `user` is non-empty.
///   - `skip(id, user, reason)` is always allowed but records the reason.
///   - `reset()` clears completion metadata but preserves the item list.
///   - `isReadyForLaunch()` is true only when every item is `Done` (skipped
///     items are considered a blocker on purpose).
class ChecklistModel
{
public:
    using Clock = std::uint64_t (*)();

    ChecklistModel() noexcept = default;
    explicit ChecklistModel(Clock clock) noexcept : _clock(clock) {}

    void setClock(Clock clock) noexcept { _clock = clock; }

    /// Append a new item. Returns true on success, false if id collides.
    bool add(const ChecklistItem& item);

    /// Remove an item by id. Returns true if something was removed.
    bool remove(const std::string& id);

    /// Reset all items to Pending.
    void reset();

    std::size_t size() const noexcept { return _items.size(); }
    bool        empty() const noexcept { return _items.empty(); }

    const std::vector<ChecklistItem>& items() const noexcept { return _items; }
    std::optional<ChecklistItem>      find(const std::string& id) const;

    /// Counts.
    std::size_t doneCount() const noexcept;
    std::size_t pendingCount() const noexcept;
    std::size_t blockedCount() const noexcept;

    /// Mark an item done. Returns false if not found, unauthorised, or the
    /// item is Blocked.
    bool markDone(const std::string& id, const std::string& user,
                  access::Role role, const std::string& notes = {});

    /// Skip an item with a mandatory reason.
    bool skip(const std::string& id, const std::string& user,
              const std::string& reason);

    /// Mark an item blocked (cannot be completed). E.g. hardware fault.
    bool block(const std::string& id, const std::string& reason);

    /// True only if every item is Done.
    bool isReadyForLaunch() const noexcept;

private:
    std::vector<ChecklistItem> _items;
    Clock                      _clock = nullptr;

    ChecklistItem* _find(const std::string& id);
};

} // namespace m130::views
