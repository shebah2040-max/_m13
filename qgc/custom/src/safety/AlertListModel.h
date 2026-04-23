#pragma once

#include "AlertManager.h"

#include <QtCore/QAbstractListModel>
#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QVector>

namespace m130::gui {

/// QAbstractListModel view over the active alerts in an AlertManager.
///
/// Exposes each `m130::safety::Alert` as a row with named roles suitable for
/// QML delegates (level, title, detail, timestamp, acked). Updates are
/// driven by `SafetyKernel` calling `refresh()` after any raise / ack
/// because the underlying `AlertManager` already serialises the map under
/// a single thread.
class AlertListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role {
        IdRole        = Qt::UserRole + 1,
        LevelRole,
        LevelTextRole,
        TitleRole,
        DetailRole,
        RaisedMsRole,
        AckedMsRole,
        AckUserRole,
    };

    explicit AlertListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    /// Replace the displayed rows with @p active, already sorted
    /// most-severe-first by the caller.
    void refresh(QVector<safety::Alert> active);

private:
    QVector<safety::Alert> _rows;
};

} // namespace m130::gui
