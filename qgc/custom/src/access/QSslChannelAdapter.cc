#include "QSslChannelAdapter.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QSslCertificate>
#include <QSslCertificateExtension>
#include <QSslSocket>
#include <QString>
#include <QStringList>

namespace m130::access {

namespace {

std::string qtoStd(const QString& s) { return s.toStdString(); }

} // namespace

QSslChannelAdapter::QSslChannelAdapter(CertPolicy policy, QObject* parent)
    : QObject(parent), _policy(std::move(policy))
{
}

void QSslChannelAdapter::setPolicy(CertPolicy policy)
{
    _policy = std::move(policy);
}

X509CertFields QSslChannelAdapter::fromCertificate(const QSslCertificate& cert)
{
    X509CertFields out;

    const auto subject_cns = cert.subjectInfo(QSslCertificate::CommonName);
    if (!subject_cns.isEmpty()) out.subject_cn = qtoStd(subject_cns.first());

    const auto issuer_cns = cert.issuerInfo(QSslCertificate::CommonName);
    if (!issuer_cns.isEmpty()) out.issuer_cn = qtoStd(issuer_cns.first());

    const auto sans = cert.subjectAlternativeNames();
    for (auto it = sans.cbegin(); it != sans.cend(); ++it) {
        out.subject_alt_names.push_back(qtoStd(it.value()));
    }

    const QByteArray digest = cert.digest(QCryptographicHash::Sha256).toHex();
    out.fingerprint_sha256_hex = qtoStd(QString::fromLatin1(digest).toLower());

    const QDateTime nb = cert.effectiveDate().toUTC();
    const QDateTime na = cert.expiryDate().toUTC();
    out.not_before = std::chrono::system_clock::from_time_t(nb.toSecsSinceEpoch());
    out.not_after  = std::chrono::system_clock::from_time_t(na.toSecsSinceEpoch());

    for (const auto& ext : cert.extensions()) {
        if (ext.name() == QStringLiteral("extendedKeyUsage")) {
            const QString value = ext.value().toString();
            const QStringList parts = value.split(QLatin1Char(','),
                                                  Qt::SkipEmptyParts);
            for (const auto& p : parts) {
                out.extended_key_usages.push_back(qtoStd(p.trimmed()));
            }
        }
    }

    out.issuer_chain.push_back(out.issuer_cn);
    return out;
}

void QSslChannelAdapter::capturePeer(const QSslSocket& socket)
{
    const auto chain = socket.peerCertificateChain();
    if (chain.isEmpty()) {
        _has_peer = false;
        return;
    }
    _peer = fromCertificate(chain.first());
    for (int i = 1; i < chain.size(); ++i) {
        const auto cn = chain.at(i).subjectInfo(QSslCertificate::CommonName);
        if (!cn.isEmpty()) _peer.issuer_chain.push_back(qtoStd(cn.first()));
    }
    _has_peer = true;
}

CertEvaluation QSslChannelAdapter::evaluate(const X509CertFields& cert,
                                            std::chrono::system_clock::time_point now) const
{
    return _policy.evaluate(cert, now);
}

Role QSslChannelAdapter::lookupRole(std::string_view subject_cn) const
{
    return _policy.lookupRole(subject_cn);
}

CertEvaluation QSslChannelAdapter::evaluateLastPeer(std::chrono::system_clock::time_point now) const
{
    if (!_has_peer) {
        CertEvaluation ev;
        ev.decision = CertDecision::MissingFingerprint;
        ev.detail   = "no peer certificate captured";
        return ev;
    }
    return _policy.evaluate(_peer, now);
}

} // namespace m130::access
