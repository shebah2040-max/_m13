#include "AccessController.h"

#include "PasswordHasher.h"

#include <QtCore/QDateTime>

#include <chrono>
#include <utility>

using m130::access::LoginResult;
using m130::access::Role;
using m130::access::Session;
using m130::access::SessionState;
using m130::access::StepUpResult;
using m130::access::UserRecord;

namespace m130::gui {

namespace {
quint64 nowMs()
{
    using namespace std::chrono;
    return static_cast<quint64>(
        duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
}
QString roleToString(Role r)
{
    switch (r) {
    case Role::Observer:       return QStringLiteral("Observer");
    case Role::Operator:       return QStringLiteral("Operator");
    case Role::FlightDirector: return QStringLiteral("FlightDirector");
    case Role::SafetyOfficer:  return QStringLiteral("SafetyOfficer");
    case Role::RangeSafety:    return QStringLiteral("RangeSafety");
    case Role::Admin:          return QStringLiteral("Admin");
    }
    return QStringLiteral("Unknown");
}
} // namespace

// ---------------- AuditTailModel ----------------

AuditTailModel::AuditTailModel(QObject* parent) : QAbstractListModel(parent) {}

void AuditTailModel::append(Entry e)
{
    const int row = _entries.size();
    beginInsertRows({}, row, row);
    _entries.append(std::move(e));
    endInsertRows();
    if (_entries.size() > kCapacity) {
        beginRemoveRows({}, 0, 0);
        _entries.removeFirst();
        endRemoveRows();
    }
}

void AuditTailModel::clear()
{
    if (_entries.isEmpty()) return;
    beginResetModel();
    _entries.clear();
    endResetModel();
}

int AuditTailModel::rowCount(const QModelIndex&) const
{
    return _entries.size();
}

QVariant AuditTailModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= _entries.size()) {
        return {};
    }
    const Entry& e = _entries.at(index.row());
    switch (role) {
    case TimestampRole: return QDateTime::fromMSecsSinceEpoch(e.t_ms).toString(QStringLiteral("hh:mm:ss"));
    case UserRole:      return e.user;
    case KindRole:      return e.kind;
    case DetailRole:    return e.detail;
    }
    return {};
}

QHash<int, QByteArray> AuditTailModel::roleNames() const
{
    return {
        {TimestampRole, "ts"},
        {UserRole,      "user"},
        {KindRole,      "action"},
        {DetailRole,    "detail"},
    };
}

// ---------------- AccessController ----------------

AccessController::AccessController(QObject* parent)
    : QObject(parent)
    , _users(access::defaultHasher(), {}, []{ return nowMs(); })
    , _sessions([]{ return nowMs(); }, {})
    , _audit_model(new AuditTailModel(this))
{
}

AccessController::~AccessController() = default;

bool AccessController::loggedIn() const noexcept
{
    return !_active_session.empty();
}

QString AccessController::currentUser() const noexcept
{
    return QString::fromStdString(_active_user);
}

QString AccessController::currentRole() const noexcept
{
    if (_active_user.empty()) return {};
    auto r = _users.find(_active_user);
    if (!r) return {};
    return roleToString(r->role);
}

QString AccessController::sessionId() const noexcept
{
    return QString::fromStdString(_active_session);
}

bool AccessController::stepUpRequired() const
{
    if (_active_session.empty()) return true;
    return _sessions.requiresStepUp(_active_session,
                                    static_cast<std::uint64_t>(_step_up_max_age_ms));
}

void AccessController::setStepUpMaxAgeMs(qint64 ms)
{
    if (ms < 0) ms = 0;
    if (ms == _step_up_max_age_ms) return;
    _step_up_max_age_ms = ms;
    emit sessionChanged();
}

void AccessController::installDefaultUsers()
{
    auto seed = [&](const std::string& id, const std::string& name, Role role) {
        if (_users.find(id)) return;
        UserRecord r;
        r.user_id      = id;
        r.display_name = name;
        r.role         = role;
        _users.upsertUser(r);
    };
    seed("observer",    "Observer",        Role::Observer);
    seed("operator",    "Operator",        Role::Operator);
    seed("director",    "Flight Director", Role::FlightDirector);
    seed("safety",      "Safety Officer",  Role::SafetyOfficer);
    seed("rso",         "Range Safety",    Role::RangeSafety);
    seed("admin",       "Administrator",   Role::Admin);
}

void AccessController::enableArgon2id()
{
    _users.setHasher(access::argon2idHasher());
    _appendAudit(QStringLiteral("system"),
                 QStringLiteral("config.hasher"),
                 QStringLiteral("argon2id"));
}

void AccessController::setLdapProvider(std::shared_ptr<access::ILdapTransport> transport,
                                       access::LdapConfig cfg)
{
    if (!transport) {
        _ldap_provider.reset();
        _appendAudit(QStringLiteral("system"),
                     QStringLiteral("config.ldap"),
                     QStringLiteral("disabled"));
        return;
    }
    _ldap_provider = std::make_shared<access::LdapAuthenticator>(std::move(transport),
                                                                 std::move(cfg));
    _appendAudit(QStringLiteral("system"),
                 QStringLiteral("config.ldap"),
                 QStringLiteral("enabled"));
}

void AccessController::setChannelSecurity(std::shared_ptr<access::IChannelSecurity> channel)
{
    _channel_security = std::move(channel);
    _appendAudit(QStringLiteral("system"),
                 QStringLiteral("config.mtls"),
                 _channel_security ? QStringLiteral("installed") : QStringLiteral("cleared"));
}

void AccessController::setGssProvider(std::shared_ptr<access::IGssProvider> provider,
                                      access::GssPolicy policy)
{
    if (!provider) {
        _gss_provider.reset();
        _appendAudit(QStringLiteral("system"),
                     QStringLiteral("config.gss"),
                     QStringLiteral("disabled"));
        return;
    }
    _gss_provider = std::make_shared<access::GssAuthenticator>(std::move(provider),
                                                               std::move(policy));
    _appendAudit(QStringLiteral("system"),
                 QStringLiteral("config.gss"),
                 QStringLiteral("enabled"));
}

bool AccessController::login(const QString& user, const QString& password)
{
    const std::string uid = user.toStdString();
    const auto r = _users.attemptPassword(uid, password.toStdString());
    switch (r.result) {
    case LoginResult::Success:
        _startSession(uid);
        _appendAudit(user, QStringLiteral("login.success"), {});
        _setError({});
        return true;
    case LoginResult::RequiresTotp:
        _pending_totp_user = uid;
        _setError(QStringLiteral("RequiresTotp"));
        emit loginFailed(QStringLiteral("RequiresTotp"));
        return false;
    case LoginResult::BadPassword:
        _setError(QStringLiteral("BadPassword"));
        _appendAudit(user, QStringLiteral("login.failed"), QStringLiteral("bad password"));
        emit loginFailed(QStringLiteral("BadPassword"));
        return false;
    case LoginResult::AccountLocked:
        _setError(QStringLiteral("AccountLocked"));
        _appendAudit(user, QStringLiteral("login.blocked"), QStringLiteral("account locked"));
        emit loginFailed(QStringLiteral("AccountLocked"));
        return false;
    case LoginResult::UnknownUser:
        _setError(QStringLiteral("UnknownUser"));
        emit loginFailed(QStringLiteral("UnknownUser"));
        return false;
    case LoginResult::BadTotp:
    case LoginResult::WeakPassword:
        _setError(QStringLiteral("InvalidState"));
        emit loginFailed(QStringLiteral("InvalidState"));
        return false;
    }
    return false;
}

bool AccessController::completeLogin(const QString& totp)
{
    if (_pending_totp_user.empty()) {
        _setError(QStringLiteral("NoPendingLogin"));
        return false;
    }
    const bool ok = _users.verifyTotp(_pending_totp_user, totp.toStdString());
    if (!ok) {
        _setError(QStringLiteral("BadTotp"));
        _appendAudit(QString::fromStdString(_pending_totp_user),
                     QStringLiteral("login.failed"),
                     QStringLiteral("bad totp"));
        emit loginFailed(QStringLiteral("BadTotp"));
        return false;
    }
    const std::string uid = std::move(_pending_totp_user);
    _pending_totp_user.clear();
    _startSession(uid);
    _appendAudit(QString::fromStdString(uid), QStringLiteral("login.success"), QStringLiteral("totp"));
    _setError({});
    return true;
}

bool AccessController::stepUp(const QString& totp)
{
    if (_active_session.empty()) {
        _setError(QStringLiteral("NoSession"));
        return false;
    }
    std::string new_id;
    const auto rc = _sessions.stepUp(_active_session,
        [&](){ return _users.verifyTotp(_active_user, totp.toStdString()); },
        &new_id);
    if (rc != StepUpResult::Ok) {
        _setError(QStringLiteral("StepUpDenied"));
        _appendAudit(QString::fromStdString(_active_user),
                     QStringLiteral("stepup.failed"), {});
        return false;
    }
    _active_session = new_id;
    _appendAudit(QString::fromStdString(_active_user),
                 QStringLiteral("stepup.success"), {});
    _setError({});
    emit sessionChanged();
    return true;
}

void AccessController::logout()
{
    if (_active_session.empty()) return;
    _sessions.revoke(_active_session);
    _appendAudit(QString::fromStdString(_active_user), QStringLiteral("logout"), {});
    _active_session.clear();
    _active_user.clear();
    emit sessionChanged();
}

void AccessController::recordEvent(const QString& kind, const QString& detail)
{
    _appendAudit(QString::fromStdString(_active_user), kind, detail);
}

void AccessController::_setError(QString msg)
{
    if (msg == _last_error) return;
    _last_error = std::move(msg);
    emit lastErrorChanged();
}

void AccessController::_appendAudit(QString user, QString kind, QString detail)
{
    AuditTailModel::Entry e;
    e.t_ms   = static_cast<qint64>(nowMs());
    e.user   = std::move(user);
    e.kind   = std::move(kind);
    e.detail = std::move(detail);
    _audit_model->append(std::move(e));
    emit auditAppended();
}

void AccessController::_startSession(const std::string& user_id)
{
    auto r = _users.find(user_id);
    if (!r) return;
    _active_session = _sessions.create(user_id, r->role);
    _active_user    = user_id;
    emit sessionChanged();
}

} // namespace m130::gui
