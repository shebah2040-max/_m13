#include "AlertListModel.h"

#include <QtCore/QByteArray>

namespace m130::gui {

AlertListModel::AlertListModel(QObject* parent)
    : QAbstractListModel(parent)
{}

int AlertListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return _rows.size();
}

QVariant AlertListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= _rows.size()) {
        return {};
    }
    const safety::Alert& a = _rows.at(index.row());
    switch (role) {
    case IdRole:        return QString::fromStdString(a.id);
    case LevelRole:     return int(a.level);
    case LevelTextRole: return QString::fromUtf8(safety::toString(a.level).data());
    case TitleRole:     return QString::fromStdString(a.title);
    case DetailRole:    return QString::fromStdString(a.detail);
    case RaisedMsRole:  return QVariant::fromValue<qulonglong>(a.raised_ms);
    case AckedMsRole:   return QVariant::fromValue<qulonglong>(a.acked_ms);
    case AckUserRole:   return QString::fromStdString(a.ack_user);
    case Qt::DisplayRole:
        return QString::fromStdString(a.title);
    default:
        return {};
    }
}

QHash<int, QByteArray> AlertListModel::roleNames() const
{
    return {
        { IdRole,        "alertId" },
        { LevelRole,     "level" },
        { LevelTextRole, "levelText" },
        { TitleRole,     "title" },
        { DetailRole,    "detail" },
        { RaisedMsRole,  "raisedMs" },
        { AckedMsRole,   "ackedMs" },
        { AckUserRole,   "ackUser" },
    };
}

void AlertListModel::refresh(QVector<safety::Alert> active)
{
    beginResetModel();
    _rows = std::move(active);
    endResetModel();
}

} // namespace m130::gui
