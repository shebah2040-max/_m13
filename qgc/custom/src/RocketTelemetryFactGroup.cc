#include "RocketTelemetryFactGroup.h"
#include "Vehicle.h"

#include <QtCore/QtMath>

// ============================================================================
// Constructor — يُسجّل جميع الـ 58 Fact مع FactGroup
// ============================================================================

RocketTelemetryFactGroup::RocketTelemetryFactGroup(QObject *parent)
    : FactGroup(0 /* immediate update */, parent)
{
    // P1 — Flight Critical
    _addFact(&_stage,           QStringLiteral("stage"));
    _addFact(&_tFlight,         QStringLiteral("tFlight"));
    _addFact(&_qDyn,            QStringLiteral("qDyn"));
    _addFact(&_qKgf,            QStringLiteral("qKgf"));
    _addFact(&_pitchAccelCmd,   QStringLiteral("pitchAccelCmd"));
    _addFact(&_yawAccelCmd,     QStringLiteral("yawAccelCmd"));
    _addFact(&_blendAlpha,      QStringLiteral("blendAlpha"));
    _addFact(&_deltaRoll,       QStringLiteral("deltaRoll"));
    _addFact(&_deltaPitch,      QStringLiteral("deltaPitch"));
    _addFact(&_deltaYaw,        QStringLiteral("deltaYaw"));
    _addFact(&_fin1,            QStringLiteral("fin1"));
    _addFact(&_fin2,            QStringLiteral("fin2"));
    _addFact(&_fin3,            QStringLiteral("fin3"));
    _addFact(&_fin4,            QStringLiteral("fin4"));
    _addFact(&_altitude,        QStringLiteral("altitude"));
    _addFact(&_airspeed,        QStringLiteral("airspeed"));
    _addFact(&_rho,             QStringLiteral("rho"));
    _addFact(&_servoOnlineMask, QStringLiteral("servoOnlineMask"));

    // P2 — Attitude & Position
    _addFact(&_phi,             QStringLiteral("phi"));
    _addFact(&_mpcSolveMs,      QStringLiteral("mpcSolveMs"));
    _addFact(&_mheQuality,      QStringLiteral("mheQuality"));
    _addFact(&_mpcSolveCount,   QStringLiteral("mpcSolveCount"));
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

    // P3 — Diagnostics & Timing
    _addFact(&_launched,        QStringLiteral("launched"));
    _addFact(&_dtActual,        QStringLiteral("dtActual"));
    _addFact(&_dtMin,           QStringLiteral("dtMin"));
    _addFact(&_dtMax,           QStringLiteral("dtMax"));
    _addFact(&_mheSolveMs,      QStringLiteral("mheSolveMs"));
    _addFact(&_mpcSolverStatus, QStringLiteral("mpcSolverStatus"));
    _addFact(&_mpcSqpIter,      QStringLiteral("mpcSqpIter"));
    _addFact(&_mheValid,        QStringLiteral("mheValid"));
    _addFact(&_mheStatus,       QStringLiteral("mheStatus"));
    _addFact(&_xvalGammaErr,    QStringLiteral("xvalGammaErr"));
    _addFact(&_xvalChiErr,      QStringLiteral("xvalChiErr"));
    _addFact(&_xvalAltErr,      QStringLiteral("xvalAltErr"));
    _addFact(&_xvalPenalty,     QStringLiteral("xvalPenalty"));
    _addFact(&_mheSolveUs,      QStringLiteral("mheSolveUs"));
    _addFact(&_mpcSolveUs,      QStringLiteral("mpcSolveUs"));
    _addFact(&_cycleUs,         QStringLiteral("cycleUs"));

    // P4 — Event Counters & GPS
    _addFact(&_mpcFailCount,    QStringLiteral("mpcFailCount"));
    _addFact(&_mpcNanSkipCount, QStringLiteral("mpcNanSkipCount"));
    _addFact(&_mheFailCount,    QStringLiteral("mheFailCount"));
    _addFact(&_finClampCount,   QStringLiteral("finClampCount"));
    _addFact(&_xvalResetCount,  QStringLiteral("xvalResetCount"));
    _addFact(&_servoOfflineEvt, QStringLiteral("servoOfflineEvt"));
    _addFact(&_gpsFixType,      QStringLiteral("gpsFixType"));
    _addFact(&_gpsSatellites,   QStringLiteral("gpsSatellites"));
    _addFact(&_gpsJammingState, QStringLiteral("gpsJammingState"));
}

// ============================================================================
// handleMessage — يفك ترميز DEBUG_FLOAT_ARRAY array_id=2 ("RktGNC")
//
// الخريطة مطابقة حرفياً لـ DEBUG_FLOAT_ARRAY.hpp في rocket_mpc.
// إذا تغيّرت الخريطة في PX4 فهذا الملف هو الوحيد الذي يحتاج تحديث.
// ============================================================================

void RocketTelemetryFactGroup::handleMessage(Vehicle * /*vehicle*/, const mavlink_message_t &message)
{
    if (message.msgid != MAVLINK_MSG_ID_DEBUG_FLOAT_ARRAY) {
        return;
    }

    mavlink_debug_float_array_t dbg;
    mavlink_msg_debug_float_array_decode(&message, &dbg);

    if (dbg.array_id != ROCKET_ARRAY_ID) {
        return;
    }

    // P1 — Flight Critical (0-17)
    _stage.setRawValue(          static_cast<double>(dbg.data[0]));
    _tFlight.setRawValue(        static_cast<double>(dbg.data[1]));
    _qDyn.setRawValue(           static_cast<double>(dbg.data[2]));
    _qKgf.setRawValue(           static_cast<double>(dbg.data[3]));
    _pitchAccelCmd.setRawValue(  static_cast<double>(dbg.data[4]));
    _yawAccelCmd.setRawValue(    static_cast<double>(dbg.data[5]));
    _blendAlpha.setRawValue(     static_cast<double>(dbg.data[6]));
    _deltaRoll.setRawValue(      static_cast<double>(dbg.data[7]));
    _deltaPitch.setRawValue(     static_cast<double>(dbg.data[8]));
    _deltaYaw.setRawValue(       static_cast<double>(dbg.data[9]));
    _fin1.setRawValue(           static_cast<double>(dbg.data[10]));
    _fin2.setRawValue(           static_cast<double>(dbg.data[11]));
    _fin3.setRawValue(           static_cast<double>(dbg.data[12]));
    _fin4.setRawValue(           static_cast<double>(dbg.data[13]));
    _altitude.setRawValue(       static_cast<double>(dbg.data[14]));
    _airspeed.setRawValue(       static_cast<double>(dbg.data[15]));
    _rho.setRawValue(            static_cast<double>(dbg.data[16]));
    _servoOnlineMask.setRawValue(static_cast<double>(dbg.data[17]));

    // P2 — Attitude & Position (18-32)
    _phi.setRawValue(            static_cast<double>(dbg.data[18]));
    _mpcSolveMs.setRawValue(     static_cast<double>(dbg.data[19]));
    _mheQuality.setRawValue(     static_cast<double>(dbg.data[20]));
    _mpcSolveCount.setRawValue(  static_cast<double>(dbg.data[21]));
    _theta.setRawValue(          static_cast<double>(dbg.data[22]));
    _psi.setRawValue(            static_cast<double>(dbg.data[23]));
    _alphaEst.setRawValue(       static_cast<double>(dbg.data[24]));
    _gammaRad.setRawValue(       static_cast<double>(dbg.data[25]));
    _posDownrange.setRawValue(   static_cast<double>(dbg.data[26]));
    _posCrossrange.setRawValue(  static_cast<double>(dbg.data[27]));
    _velDownrange.setRawValue(   static_cast<double>(dbg.data[28]));
    _velDown.setRawValue(        static_cast<double>(dbg.data[29]));
    _velCrossrange.setRawValue(  static_cast<double>(dbg.data[30]));
    _bearingDeg.setRawValue(     static_cast<double>(dbg.data[31]));
    _targetRangeRem.setRawValue( static_cast<double>(dbg.data[32]));

    // P3 — Diagnostics & Timing (33-48)
    _launched.setRawValue(       static_cast<double>(dbg.data[33]));
    _dtActual.setRawValue(       static_cast<double>(dbg.data[34]));
    _dtMin.setRawValue(          static_cast<double>(dbg.data[35]));
    _dtMax.setRawValue(          static_cast<double>(dbg.data[36]));
    _mheSolveMs.setRawValue(     static_cast<double>(dbg.data[37]));
    _mpcSolverStatus.setRawValue(static_cast<double>(dbg.data[38]));
    _mpcSqpIter.setRawValue(     static_cast<double>(dbg.data[39]));
    _mheValid.setRawValue(       static_cast<double>(dbg.data[40]));
    _mheStatus.setRawValue(      static_cast<double>(dbg.data[41]));
    _xvalGammaErr.setRawValue(   static_cast<double>(dbg.data[42]));
    _xvalChiErr.setRawValue(     static_cast<double>(dbg.data[43]));
    _xvalAltErr.setRawValue(     static_cast<double>(dbg.data[44]));
    _xvalPenalty.setRawValue(    static_cast<double>(dbg.data[45]));
    _mheSolveUs.setRawValue(     static_cast<double>(dbg.data[46]));
    _mpcSolveUs.setRawValue(     static_cast<double>(dbg.data[47]));
    _cycleUs.setRawValue(        static_cast<double>(dbg.data[48]));

    // P4 — Event Counters & GPS (49-57)
    _mpcFailCount.setRawValue(   static_cast<double>(dbg.data[49]));
    _mpcNanSkipCount.setRawValue(static_cast<double>(dbg.data[50]));
    _mheFailCount.setRawValue(   static_cast<double>(dbg.data[51]));
    _finClampCount.setRawValue(  static_cast<double>(dbg.data[52]));
    _xvalResetCount.setRawValue( static_cast<double>(dbg.data[53]));
    _servoOfflineEvt.setRawValue(static_cast<double>(dbg.data[54]));
    _gpsFixType.setRawValue(     static_cast<double>(dbg.data[55]));
    _gpsSatellites.setRawValue(  static_cast<double>(dbg.data[56]));
    _gpsJammingState.setRawValue(static_cast<double>(dbg.data[57]));

    _setTelemetryAvailable(true);
}
