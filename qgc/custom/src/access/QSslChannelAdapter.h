#pragma once

#include "CertPolicy.h"

#include <QObject>

QT_BEGIN_NAMESPACE
class QSslCertificate;
class QSslSocket;
QT_END_NAMESPACE

namespace m130::access {

/// Bridge that converts a `QSslCertificate` (peer certificate from a
/// live TLS session) into the transport-agnostic `X509CertFields` struct
/// and applies a configured `CertPolicy`. One instance may be shared
/// across every mTLS-guarded operation.
class QSslChannelAdapter : public QObject, public IChannelSecurity
{
    Q_OBJECT
public:
    explicit QSslChannelAdapter(CertPolicy policy, QObject* parent = nullptr);

    void setPolicy(CertPolicy policy);
    const CertPolicy& policy() const noexcept { return _policy; }

    /// Populate an `X509CertFields` record from a peer certificate.
    static X509CertFields fromCertificate(const QSslCertificate& cert);

    /// Capture the peer cert from a live QSslSocket (usually right after
    /// `encrypted()` fires). The cert snapshot is consulted by the next
    /// `evaluate()` call.
    void capturePeer(const QSslSocket& socket);

    /// IChannelSecurity
    CertEvaluation evaluate(const X509CertFields& cert,
                            std::chrono::system_clock::time_point now) const override;
    Role           lookupRole(std::string_view subject_cn) const override;

    /// Convenience: evaluate the most recently captured peer cert.
    CertEvaluation evaluateLastPeer(std::chrono::system_clock::time_point now) const;
    const X509CertFields& lastPeer() const noexcept { return _peer; }

private:
    CertPolicy     _policy;
    X509CertFields _peer;
    bool           _has_peer = false;
};

} // namespace m130::access
