#pragma once

#include "LdapAuthenticator.h"
#include "LdapMessage.h"

#include <QByteArray>
#include <QHostAddress>
#include <QObject>
#include <QString>

#include <atomic>
#include <memory>

QT_BEGIN_NAMESPACE
class QAbstractSocket;
class QTcpSocket;
class QSslSocket;
QT_END_NAMESPACE

namespace m130::access {

/// Connection profile for `QtLdapTransport`. LDAPS is selected whenever
/// `use_tls` is true; StartTLS is deferred to a later phase.
struct QtLdapEndpoint {
    QString     host              = QStringLiteral("127.0.0.1");
    quint16     port              = 389;
    bool        use_tls           = false;   ///< when true, use LDAPS on the same port.
    int         connect_timeout_ms = 5000;
    int         io_timeout_ms     = 5000;
    QByteArray  ca_bundle_pem;                ///< optional extra CAs for LDAPS.
    bool        verify_peer       = true;
};

/// Concrete `ILdapTransport` that speaks LDAPv3 over QTcpSocket or
/// QSslSocket. Each `bind()` / `search()` call opens a fresh connection,
/// runs the operation synchronously, and closes the socket — the surface
/// mirrors the in-memory test double so the rest of the access layer is
/// transport-agnostic.
class QtLdapTransport : public QObject, public ILdapTransport
{
    Q_OBJECT
public:
    explicit QtLdapTransport(QtLdapEndpoint endpoint, QObject* parent = nullptr);
    ~QtLdapTransport() override;

    bool                        bind(std::string_view dn, std::string_view password) override;
    std::vector<LdapEntry>      search(std::string_view base_dn,
                                       const LdapFilter& filter) override;

    QString lastError() const { return _last_error; }

private:
    struct Connection; // pimpl for socket lifetime

    std::unique_ptr<Connection> _openConnection();
    bool   _writeAll(Connection& c, const std::vector<std::uint8_t>& blob);
    bool   _readMessage(Connection& c,
                        std::vector<std::uint8_t>& out_message);
    void   _setError(QString msg);

    QtLdapEndpoint        _endpoint;
    QString               _last_error;
    std::atomic<std::int64_t> _next_message_id{1};
};

} // namespace m130::access
