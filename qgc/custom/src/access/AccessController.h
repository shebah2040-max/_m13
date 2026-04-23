#pragma once

#include "SessionManager.h"
#include "UserManager.h"

#include <QtCore/QAbstractListModel>
#include <QtCore/QDateTime>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QVector>

#include <cstdint>
#include <memory>
#include <string>

namespace m130::gui {

class AuditTailModel;

/// Qt-bound facade for the pure-C++ UserManager + SessionManager (Pillar 6).
///
/// Exposes login, logout, step-up, and lockout status as QML-bindable
/// properties + Q_INVOKABLE slots. Owns an AuditTailModel that is fed
/// with every access-relevant event so the Admin console can show a live
/// audit-log viewer (REQ-M130-GCS-ACC-002).
class AccessController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool     loggedIn        READ loggedIn        NOTIFY sessionChanged)
    Q_PROPERTY(QString  currentUser     READ currentUser     NOTIFY sessionChanged)
    Q_PROPERTY(QString  currentRole     READ currentRole     NOTIFY sessionChanged)
    Q_PROPERTY(QString  sessionId       READ sessionId       NOTIFY sessionChanged)
    Q_PROPERTY(bool     stepUpRequired  READ stepUpRequired  NOTIFY sessionChanged)
    Q_PROPERTY(QString  lastError       READ lastError       NOTIFY lastErrorChanged)
    Q_PROPERTY(m130::gui::AuditTailModel* auditTail READ auditTail CONSTANT)

public:
    explicit AccessController(QObject* parent = nullptr);
    ~AccessController() override;

    bool    loggedIn()       const noexcept;
    QString currentUser()    const noexcept;
    QString currentRole()    const noexcept;
    QString sessionId()      const noexcept;
    bool    stepUpRequired() const;
    QString lastError()      const noexcept { return _last_error; }
    AuditTailModel* auditTail() const noexcept { return _audit_model; }

    /// Step-up freshness window for critical ops (e.g. FTS). Default: 5 min.
    Q_INVOKABLE void setStepUpMaxAgeMs(qint64 ms);
    Q_INVOKABLE qint64 stepUpMaxAgeMs() const noexcept { return _step_up_max_age_ms; }

    /// Raw access for non-QML callers (plugin, tests).
    access::UserManager&    users()    noexcept { return _users; }
    access::SessionManager& sessions() noexcept { return _sessions; }

    /// Seed three standard operator accounts for local development.
    /// Safe to call multiple times; no-ops if users already exist.
    Q_INVOKABLE void installDefaultUsers();

public slots:
    /// Attempt password login. If the user requires TOTP, leaves the
    /// session in a not-yet-created state and returns false with
    /// lastError = "RequiresTotp". Use `completeLogin(totp)` to finish.
    bool login(const QString& user, const QString& password);

    /// Finish a RequiresTotp login. Returns false with lastError set on
    /// bad code / timeout.
    bool completeLogin(const QString& totp);

    /// Step up the active session by verifying a TOTP code. On success
    /// rotates the session id and stamps the step-up timestamp.
    bool stepUp(const QString& totp);

    /// Log out and emit sessionChanged.
    void logout();

    /// Record an operator-driven event in the audit tail. `kind` is a
    /// short slug ("checklist.mark_done", "fts.self_test", ...). `detail`
    /// is free-form.
    void recordEvent(const QString& kind, const QString& detail);

signals:
    void sessionChanged();
    void lastErrorChanged();
    /// Fired when an audit entry is appended.
    void auditAppended();
    /// Fired when a login attempt fails (for UI feedback beyond lastError).
    void loginFailed(const QString& reason);

private:
    void _setError(QString msg);
    void _appendAudit(QString user, QString kind, QString detail);
    void _startSession(const std::string& user_id);

private:
    access::UserManager          _users;
    access::SessionManager       _sessions;
    AuditTailModel*              _audit_model;

    std::string   _pending_totp_user;
    std::string   _active_session;
    std::string   _active_user;
    QString       _last_error;
    qint64        _step_up_max_age_ms = 5 * 60 * 1000;
};

/// Bounded circular audit tail — last 512 entries kept in memory.
class AuditTailModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        TimestampRole = Qt::UserRole + 1,
        UserRole,
        KindRole,
        DetailRole,
    };

    explicit AuditTailModel(QObject* parent = nullptr);

    struct Entry {
        qint64  t_ms = 0;
        QString user;
        QString kind;
        QString detail;
    };

    void append(Entry e);
    void clear();

    int              rowCount(const QModelIndex& parent = {}) const override;
    QVariant         data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    QVector<Entry> _entries;
    static constexpr int kCapacity = 512;
};

} // namespace m130::gui
