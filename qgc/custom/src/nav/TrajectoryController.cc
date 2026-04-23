#include "TrajectoryController.h"

#include <chrono>

namespace m130::gui {

namespace {
std::chrono::system_clock::time_point msToTp(qint64 ms)
{
    return std::chrono::system_clock::time_point{std::chrono::milliseconds(ms)};
}
} // namespace

TrajectoryController::TrajectoryController(QObject* parent)
    : QObject(parent), _track_model(new TrajectoryTrackModel(&_core, this))
{
}

TrajectoryController::~TrajectoryController() = default;

void TrajectoryController::setOrigin(double lat_deg, double lon_deg, double alt_m)
{
    _core.setOrigin(geo::GeoPoint{lat_deg, lon_deg, alt_m});
    _track_model->rebuild();
    emit originChanged();
    emit trackChanged();
}

void TrajectoryController::clear()
{
    _core.clear();
    _track_model->rebuild();
    emit trackChanged();
}

bool TrajectoryController::appendSample(qint64 t_ms,
                                        double lat_deg, double lon_deg, double alt_m,
                                        double ve_mps, double vn_mps, double vu_mps)
{
    nav::TrajectorySample s;
    s.t = msToTp(t_ms);
    s.position = geo::GeoPoint{lat_deg, lon_deg, alt_m};
    s.ve_mps = ve_mps; s.vn_mps = vn_mps; s.vu_mps = vu_mps;
    const bool ok = _core.append(s);
    if (ok) {
        _track_model->rebuild();
        emit trackChanged();
    }
    return ok;
}

void TrajectoryController::appendIip(qint64 t_ms,
                                     double lat_deg, double lon_deg, double alt_m)
{
    _core.appendIip(msToTp(t_ms), geo::GeoPoint{lat_deg, lon_deg, alt_m});
    emit trackChanged();
}

// ----------------------- TrajectoryTrackModel ------------------------------

TrajectoryTrackModel::TrajectoryTrackModel(nav::TrajectoryModel* core, QObject* parent)
    : QAbstractListModel(parent), _core(core)
{
}

void TrajectoryTrackModel::rebuild()
{
    beginResetModel();
    _points = _core ? _core->trackEnu(4096) : std::vector<geo::EnuOffset>{};
    endResetModel();
}

int TrajectoryTrackModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return static_cast<int>(_points.size());
}

QVariant TrajectoryTrackModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) return {};
    const auto r = static_cast<std::size_t>(index.row());
    if (r >= _points.size()) return {};
    const auto& p = _points[r];
    switch (role) {
    case EastRole:  return p.east_m;
    case NorthRole: return p.north_m;
    case UpRole:    return p.up_m;
    default:        return {};
    }
}

QHash<int, QByteArray> TrajectoryTrackModel::roleNames() const
{
    return {
        { EastRole,  "east_m"  },
        { NorthRole, "north_m" },
        { UpRole,    "up_m"    },
    };
}

} // namespace m130::gui
