#include "M130GncStateFactGroup.h"
#include "protocol/generated/M130Messages.generated.h"

#include <QtCore/QString>

M130GncStateFactGroup::M130GncStateFactGroup(QObject *parent)
    : FactGroup(0 /* immediate update */, parent)
{
    _addFact(&_timeUsec,        QStringLiteral("timeUsec"));
    _addFact(&_stage,           QStringLiteral("stage"));
    _addFact(&_launched,        QStringLiteral("launched"));
    _addFact(&_mode,            QStringLiteral("mode"));
    _addFact(&_qDyn,            QStringLiteral("qDyn"));
    _addFact(&_rho,             QStringLiteral("rho"));
    _addFact(&_altitude,        QStringLiteral("altitude"));
    _addFact(&_airspeed,        QStringLiteral("airspeed"));
    _addFact(&_phi,             QStringLiteral("phi"));
    _addFact(&_theta,           QStringLiteral("theta"));
    _addFact(&_psi,             QStringLiteral("psi"));
    _addFact(&_alphaEst,        QStringLiteral("alphaEst"));
    _addFact(&_gammaRad,        QStringLiteral("gammaRad"));
    _addFact(&_posDownrange,    QStringLiteral("posDownrange"));
    _addFact(&_posCrossrange,   QStringLiteral("posCrossrange"));
    _addFact(&_velDownrange,    QStringLiteral("velDownrange"));
    _addFact(&_velDown,         QStringLiteral("velDown"));
    _addFact(&_velCrossrange,   QStringLiteral("velCrossrange"));
    _addFact(&_bearingDeg,      QStringLiteral("bearingDeg"));
    _addFact(&_targetRangeRem,  QStringLiteral("targetRangeRem"));
    _addFact(&_iipLat,          QStringLiteral("iipLat"));
    _addFact(&_iipLon,          QStringLiteral("iipLon"));
    _addFact(&_iipAlt,          QStringLiteral("iipAlt"));
    _addFact(&_messagesReceived, QStringLiteral("messagesReceived"));
}

void M130GncStateFactGroup::applyState(const m130::protocol::gen::M130GncState& s)
{
    _timeUsec.setRawValue(static_cast<double>(s.time_usec));
    _stage.setRawValue(s.stage);
    _launched.setRawValue(s.launched);
    _mode.setRawValue(s.mode);
    _qDyn.setRawValue(s.q_dyn);
    _rho.setRawValue(s.rho);
    _altitude.setRawValue(s.altitude);
    _airspeed.setRawValue(s.airspeed);
    _phi.setRawValue(s.phi);
    _theta.setRawValue(s.theta);
    _psi.setRawValue(s.psi);
    _alphaEst.setRawValue(s.alpha_est);
    _gammaRad.setRawValue(s.gamma_rad);
    _posDownrange.setRawValue(s.pos_downrange);
    _posCrossrange.setRawValue(s.pos_crossrange);
    _velDownrange.setRawValue(s.vel_downrange);
    _velDown.setRawValue(s.vel_down);
    _velCrossrange.setRawValue(s.vel_crossrange);
    _bearingDeg.setRawValue(s.bearing_deg);
    _targetRangeRem.setRawValue(s.target_range_rem);
    _iipLat.setRawValue(static_cast<double>(s.iip_lat_e7) * 1e-7);
    _iipLon.setRawValue(static_cast<double>(s.iip_lon_e7) * 1e-7);
    _iipAlt.setRawValue(s.iip_alt);
    _messagesReceived.setRawValue(_messagesReceived.rawValue().toULongLong() + 1);
}
