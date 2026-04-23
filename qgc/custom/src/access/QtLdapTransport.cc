#include "QtLdapTransport.h"

#include "Ber.h"

#include <QDebug>
#include <QSslConfiguration>
#include <QSslSocket>
#include <QTcpSocket>

namespace m130::access {

struct QtLdapTransport::Connection {
    std::unique_ptr<QAbstractSocket> socket;
    QByteArray                       buffer;
};

QtLdapTransport::QtLdapTransport(QtLdapEndpoint endpoint, QObject* parent)
    : QObject(parent), _endpoint(std::move(endpoint))
{
}

QtLdapTransport::~QtLdapTransport() = default;

std::unique_ptr<QtLdapTransport::Connection> QtLdapTransport::_openConnection()
{
    auto conn = std::make_unique<Connection>();

    if (_endpoint.use_tls) {
        auto* s = new QSslSocket(this);
        QSslConfiguration cfg = s->sslConfiguration();
        if (!_endpoint.verify_peer) {
            cfg.setPeerVerifyMode(QSslSocket::VerifyNone);
        } else if (!_endpoint.ca_bundle_pem.isEmpty()) {
            const auto cas = QSslCertificate::fromData(_endpoint.ca_bundle_pem,
                                                       QSsl::Pem);
            cfg.setCaCertificates(cas);
        }
        s->setSslConfiguration(cfg);
        s->connectToHostEncrypted(_endpoint.host, _endpoint.port);
        if (!s->waitForEncrypted(_endpoint.connect_timeout_ms)) {
            _setError(QStringLiteral("TLS handshake failed: ") + s->errorString());
            return nullptr;
        }
        conn->socket.reset(s);
    } else {
        auto* s = new QTcpSocket(this);
        s->connectToHost(_endpoint.host, _endpoint.port);
        if (!s->waitForConnected(_endpoint.connect_timeout_ms)) {
            _setError(QStringLiteral("TCP connect failed: ") + s->errorString());
            return nullptr;
        }
        conn->socket.reset(s);
    }
    return conn;
}

bool QtLdapTransport::_writeAll(Connection& c, const std::vector<std::uint8_t>& blob)
{
    const qint64 n = c.socket->write(reinterpret_cast<const char*>(blob.data()),
                                     static_cast<qint64>(blob.size()));
    if (n != static_cast<qint64>(blob.size())) {
        _setError(QStringLiteral("short write: ") + c.socket->errorString());
        return false;
    }
    return c.socket->waitForBytesWritten(_endpoint.io_timeout_ms);
}

bool QtLdapTransport::_readMessage(Connection& c,
                                   std::vector<std::uint8_t>& out_message)
{
    while (true) {
        // Attempt to parse an LDAPMessage SEQUENCE envelope from the
        // accumulated buffer.
        if (c.buffer.size() >= 2) {
            const auto* bytes = reinterpret_cast<const std::uint8_t*>(c.buffer.constData());
            ber::Decoder peek(bytes, static_cast<std::size_t>(c.buffer.size()));
            std::uint8_t ident = 0;
            const std::uint8_t* payload = nullptr;
            std::size_t payload_len = 0;
            const std::size_t before = peek.position();
            if (peek.next(ident, payload, payload_len) && ident == ber::tag::kSequence) {
                const std::size_t total = peek.position() - before;
                out_message.assign(bytes, bytes + total);
                c.buffer.remove(0, static_cast<int>(total));
                return true;
            }
        }
        if (!c.socket->waitForReadyRead(_endpoint.io_timeout_ms)) {
            _setError(QStringLiteral("read timeout: ") + c.socket->errorString());
            return false;
        }
        c.buffer.append(c.socket->readAll());
    }
}

bool QtLdapTransport::bind(std::string_view dn, std::string_view password)
{
    auto c = _openConnection();
    if (!c) return false;

    ldap::BindRequest req;
    req.message_id = _next_message_id++;
    req.dn         = std::string(dn);
    req.password   = std::string(password);

    if (!_writeAll(*c, ldap::encodeBindRequest(req))) return false;

    std::vector<std::uint8_t> blob;
    if (!_readMessage(*c, blob)) return false;

    std::size_t consumed = 0;
    auto resp = ldap::decodeMessage(blob, consumed);
    if (!resp || resp->op != ldap::op::kBindResponse) {
        _setError(QStringLiteral("malformed bind response"));
        return false;
    }
    const bool ok = resp->bind.result_code == ldap::ResultCode::Success;
    if (!ok) {
        _setError(QStringLiteral("bind failed: %1").arg(QString::fromStdString(resp->bind.diagnostic)));
    }

    // Close with UnbindRequest regardless of bind outcome.
    _writeAll(*c, ldap::encodeUnbindRequest(_next_message_id++));
    c->socket->disconnectFromHost();
    return ok;
}

std::vector<LdapEntry> QtLdapTransport::search(std::string_view base_dn,
                                               const LdapFilter& filter)
{
    std::vector<LdapEntry> out;
    auto c = _openConnection();
    if (!c) return out;

    ldap::SearchRequest req;
    req.message_id = _next_message_id++;
    req.base_dn    = std::string(base_dn);
    req.filter     = filter;
    req.attributes = {"dn", "memberOf", "objectClass"};

    if (!_writeAll(*c, ldap::encodeSearchRequest(req))) return out;

    while (true) {
        std::vector<std::uint8_t> blob;
        if (!_readMessage(*c, blob)) break;
        std::size_t consumed = 0;
        auto resp = ldap::decodeMessage(blob, consumed);
        if (!resp) { _setError(QStringLiteral("malformed search response")); break; }

        if (resp->op == ldap::op::kSearchResultEntry) {
            LdapEntry entry;
            for (const auto& [name, values] : resp->entry.attrs) {
                for (const auto& v : values) entry.add(name, v);
            }
            out.push_back(std::move(entry));
        } else if (resp->op == ldap::op::kSearchResultDone) {
            break;
        } else {
            break;
        }
    }

    _writeAll(*c, ldap::encodeUnbindRequest(_next_message_id++));
    c->socket->disconnectFromHost();
    return out;
}

void QtLdapTransport::_setError(QString msg)
{
    _last_error = std::move(msg);
    qWarning() << "[QtLdapTransport]" << _last_error;
}

} // namespace m130::access
