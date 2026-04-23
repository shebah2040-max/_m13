#pragma once

#include "FactGroup.h"
#include "QGCMAVLink.h"

// ============================================================================
// RocketTelemetryFactGroup
//
// يستقبل ويفك ترميز DEBUG_FLOAT_ARRAY (array_id=2, name="RktGNC") المُرسَل
// من وحدة rocket_mpc في PX4 بمعدل 20 Hz.
//
// خريطة الـ indices مطابقة حرفياً لـ:
//   m13/AndroidApp/.../mavlink/streams/DEBUG_FLOAT_ARRAY.hpp
//
// يُضاف هذا الـ FactGroup إلى Vehicle عبر CustomFirmwarePlugin::factGroups()
// مما يجعل جميع الحقول متاحة في QML بصيغة:
//   vehicle.getFactGroup("rocket").stage.rawValue
// ============================================================================

class RocketTelemetryFactGroup : public FactGroup
{
    Q_OBJECT

    // -------------------------------------------------------------------------
    // P1 — Flight Critical (indices 0-17)
    // -------------------------------------------------------------------------
    Q_PROPERTY(Fact* stage             READ stage             CONSTANT)  // [0]  مرحلة الطيران (0=IDLE,1=BOOST,2=CRUISE,3=TERMINAL)
    Q_PROPERTY(Fact* tFlight           READ tFlight           CONSTANT)  // [1]  زمن الطيران (s)
    Q_PROPERTY(Fact* qDyn              READ qDyn              CONSTANT)  // [2]  الضغط الديناميكي (Pa)
    Q_PROPERTY(Fact* qKgf              READ qKgf              CONSTANT)  // [3]  الضغط الديناميكي (kgf/m²)
    Q_PROPERTY(Fact* pitchAccelCmd     READ pitchAccelCmd     CONSTANT)  // [4]  أمر تسارع الإيميل (rad/s²)
    Q_PROPERTY(Fact* yawAccelCmd       READ yawAccelCmd       CONSTANT)  // [5]  أمر تسارع الانحراف (rad/s²)
    Q_PROPERTY(Fact* blendAlpha        READ blendAlpha        CONSTANT)  // [6]  معامل الخلط MHE↔EKF (0-1)
    Q_PROPERTY(Fact* deltaRoll         READ deltaRoll         CONSTANT)  // [7]  أمر الدحرجة (deg)
    Q_PROPERTY(Fact* deltaPitch        READ deltaPitch        CONSTANT)  // [8]  أمر الإيميل (deg)
    Q_PROPERTY(Fact* deltaYaw          READ deltaYaw          CONSTANT)  // [9]  أمر الانحراف (deg)
    Q_PROPERTY(Fact* fin1              READ fin1              CONSTANT)  // [10] انحراف الزعنفة 1 (deg)
    Q_PROPERTY(Fact* fin2              READ fin2              CONSTANT)  // [11] انحراف الزعنفة 2 (deg)
    Q_PROPERTY(Fact* fin3              READ fin3              CONSTANT)  // [12] انحراف الزعنفة 3 (deg)
    Q_PROPERTY(Fact* fin4              READ fin4              CONSTANT)  // [13] انحراف الزعنفة 4 (deg)
    Q_PROPERTY(Fact* altitude          READ altitude          CONSTANT)  // [14] الارتفاع (m)
    Q_PROPERTY(Fact* airspeed          READ airspeed          CONSTANT)  // [15] السرعة الهوائية (m/s)
    Q_PROPERTY(Fact* rho               READ rho               CONSTANT)  // [16] كثافة الهواء (kg/m³)
    Q_PROPERTY(Fact* servoOnlineMask   READ servoOnlineMask   CONSTANT)  // [17] قناع السيرفو المتصل (0b1111=كل4)

    // -------------------------------------------------------------------------
    // P2 — Attitude & Position (indices 18-32)
    // -------------------------------------------------------------------------
    Q_PROPERTY(Fact* phi               READ phi               CONSTANT)  // [18] زاوية الدحرجة (deg)
    Q_PROPERTY(Fact* mpcSolveMs        READ mpcSolveMs        CONSTANT)  // [19] زمن حل MPC (ms)
    Q_PROPERTY(Fact* mheQuality        READ mheQuality        CONSTANT)  // [20] جودة MHE (0-1)
    Q_PROPERTY(Fact* mpcSolveCount     READ mpcSolveCount     CONSTANT)  // [21] عداد حلول MPC
    Q_PROPERTY(Fact* theta             READ theta             CONSTANT)  // [22] زاوية الإيميل (deg)
    Q_PROPERTY(Fact* psi               READ psi               CONSTANT)  // [23] زاوية الانحراف (deg)
    Q_PROPERTY(Fact* alphaEst          READ alphaEst          CONSTANT)  // [24] زاوية الهجوم المُقدَّرة (deg)
    Q_PROPERTY(Fact* gammaRad          READ gammaRad          CONSTANT)  // [25] زاوية مسار الطيران (rad)
    Q_PROPERTY(Fact* posDownrange      READ posDownrange      CONSTANT)  // [26] المسافة الأفقية (m)
    Q_PROPERTY(Fact* posCrossrange     READ posCrossrange     CONSTANT)  // [27] الانحراف الجانبي (m)
    Q_PROPERTY(Fact* velDownrange      READ velDownrange      CONSTANT)  // [28] السرعة الأفقية (m/s)
    Q_PROPERTY(Fact* velDown           READ velDown           CONSTANT)  // [29] السرعة النازلة (m/s)
    Q_PROPERTY(Fact* velCrossrange     READ velCrossrange     CONSTANT)  // [30] السرعة الجانبية (m/s)
    Q_PROPERTY(Fact* bearingDeg        READ bearingDeg        CONSTANT)  // [31] الاتجاه (deg)
    Q_PROPERTY(Fact* targetRangeRem    READ targetRangeRem    CONSTANT)  // [32] المسافة المتبقية للهدف (m)

    // -------------------------------------------------------------------------
    // P3 — Diagnostics & Timing (indices 33-48)
    // -------------------------------------------------------------------------
    Q_PROPERTY(Fact* launched          READ launched          CONSTANT)  // [33] هل أُطلق؟ (0/1)
    Q_PROPERTY(Fact* dtActual          READ dtActual          CONSTANT)  // [34] dt الفعلي (ms)
    Q_PROPERTY(Fact* dtMin             READ dtMin             CONSTANT)  // [35] dt الأدنى (ms)
    Q_PROPERTY(Fact* dtMax             READ dtMax             CONSTANT)  // [36] dt الأقصى (ms)
    Q_PROPERTY(Fact* mheSolveMs        READ mheSolveMs        CONSTANT)  // [37] زمن حل MHE (ms)
    Q_PROPERTY(Fact* mpcSolverStatus   READ mpcSolverStatus   CONSTANT)  // [38] حالة محلل MPC
    Q_PROPERTY(Fact* mpcSqpIter        READ mpcSqpIter        CONSTANT)  // [39] عدد تكرارات SQP
    Q_PROPERTY(Fact* mheValid          READ mheValid          CONSTANT)  // [40] هل MHE صالح؟ (0/1)
    Q_PROPERTY(Fact* mheStatus         READ mheStatus         CONSTANT)  // [41] حالة MHE
    Q_PROPERTY(Fact* xvalGammaErr      READ xvalGammaErr      CONSTANT)  // [42] خطأ cross-val gamma (deg)
    Q_PROPERTY(Fact* xvalChiErr        READ xvalChiErr        CONSTANT)  // [43] خطأ cross-val chi (deg)
    Q_PROPERTY(Fact* xvalAltErr        READ xvalAltErr        CONSTANT)  // [44] خطأ cross-val altitude (m)
    Q_PROPERTY(Fact* xvalPenalty       READ xvalPenalty       CONSTANT)  // [45] عقوبة cross-val الكلية
    Q_PROPERTY(Fact* mheSolveUs        READ mheSolveUs        CONSTANT)  // [46] زمن حل MHE (µs)
    Q_PROPERTY(Fact* mpcSolveUs        READ mpcSolveUs        CONSTANT)  // [47] زمن حل MPC (µs)
    Q_PROPERTY(Fact* cycleUs           READ cycleUs           CONSTANT)  // [48] زمن الدورة الكاملة (µs)

    // -------------------------------------------------------------------------
    // P4 — Event Counters & GPS (indices 49-57)
    // -------------------------------------------------------------------------
    Q_PROPERTY(Fact* mpcFailCount      READ mpcFailCount      CONSTANT)  // [49] عداد إخفاقات MPC
    Q_PROPERTY(Fact* mpcNanSkipCount   READ mpcNanSkipCount   CONSTANT)  // [50] عداد تخطي NaN
    Q_PROPERTY(Fact* mheFailCount      READ mheFailCount      CONSTANT)  // [51] عداد إخفاقات MHE
    Q_PROPERTY(Fact* finClampCount     READ finClampCount     CONSTANT)  // [52] عداد تثبيت الزعانف
    Q_PROPERTY(Fact* xvalResetCount    READ xvalResetCount    CONSTANT)  // [53] عداد إعادة تعيين xval
    Q_PROPERTY(Fact* servoOfflineEvt   READ servoOfflineEvt   CONSTANT)  // [54] أحداث انقطاع السيرفو
    Q_PROPERTY(Fact* gpsFixType        READ gpsFixType        CONSTANT)  // [55] نوع إصلاح GPS
    Q_PROPERTY(Fact* gpsSatellites     READ gpsSatellites     CONSTANT)  // [56] عدد الأقمار الصناعية
    Q_PROPERTY(Fact* gpsJammingState   READ gpsJammingState   CONSTANT)  // [57] حالة التشويش GPS

public:
    explicit RocketTelemetryFactGroup(QObject *parent = nullptr);

    // Accessors — P1
    Fact* stage()           { return &_stage;           }
    Fact* tFlight()         { return &_tFlight;         }
    Fact* qDyn()            { return &_qDyn;            }
    Fact* qKgf()            { return &_qKgf;            }
    Fact* pitchAccelCmd()   { return &_pitchAccelCmd;   }
    Fact* yawAccelCmd()     { return &_yawAccelCmd;     }
    Fact* blendAlpha()      { return &_blendAlpha;      }
    Fact* deltaRoll()       { return &_deltaRoll;       }
    Fact* deltaPitch()      { return &_deltaPitch;      }
    Fact* deltaYaw()        { return &_deltaYaw;        }
    Fact* fin1()            { return &_fin1;            }
    Fact* fin2()            { return &_fin2;            }
    Fact* fin3()            { return &_fin3;            }
    Fact* fin4()            { return &_fin4;            }
    Fact* altitude()        { return &_altitude;        }
    Fact* airspeed()        { return &_airspeed;        }
    Fact* rho()             { return &_rho;             }
    Fact* servoOnlineMask() { return &_servoOnlineMask; }

    // Accessors — P2
    Fact* phi()             { return &_phi;             }
    Fact* mpcSolveMs()      { return &_mpcSolveMs;      }
    Fact* mheQuality()      { return &_mheQuality;      }
    Fact* mpcSolveCount()   { return &_mpcSolveCount;   }
    Fact* theta()           { return &_theta;           }
    Fact* psi()             { return &_psi;             }
    Fact* alphaEst()        { return &_alphaEst;        }
    Fact* gammaRad()        { return &_gammaRad;        }
    Fact* posDownrange()    { return &_posDownrange;    }
    Fact* posCrossrange()   { return &_posCrossrange;   }
    Fact* velDownrange()    { return &_velDownrange;    }
    Fact* velDown()         { return &_velDown;         }
    Fact* velCrossrange()   { return &_velCrossrange;   }
    Fact* bearingDeg()      { return &_bearingDeg;      }
    Fact* targetRangeRem()  { return &_targetRangeRem;  }

    // Accessors — P3
    Fact* launched()        { return &_launched;        }
    Fact* dtActual()        { return &_dtActual;        }
    Fact* dtMin()           { return &_dtMin;           }
    Fact* dtMax()           { return &_dtMax;           }
    Fact* mheSolveMs()      { return &_mheSolveMs;      }
    Fact* mpcSolverStatus() { return &_mpcSolverStatus; }
    Fact* mpcSqpIter()      { return &_mpcSqpIter;      }
    Fact* mheValid()        { return &_mheValid;        }
    Fact* mheStatus()       { return &_mheStatus;       }
    Fact* xvalGammaErr()    { return &_xvalGammaErr;    }
    Fact* xvalChiErr()      { return &_xvalChiErr;      }
    Fact* xvalAltErr()      { return &_xvalAltErr;      }
    Fact* xvalPenalty()     { return &_xvalPenalty;     }
    Fact* mheSolveUs()      { return &_mheSolveUs;      }
    Fact* mpcSolveUs()      { return &_mpcSolveUs;      }
    Fact* cycleUs()         { return &_cycleUs;         }

    // Accessors — P4
    Fact* mpcFailCount()    { return &_mpcFailCount;    }
    Fact* mpcNanSkipCount() { return &_mpcNanSkipCount; }
    Fact* mheFailCount()    { return &_mheFailCount;    }
    Fact* finClampCount()   { return &_finClampCount;   }
    Fact* xvalResetCount()  { return &_xvalResetCount;  }
    Fact* servoOfflineEvt() { return &_servoOfflineEvt; }
    Fact* gpsFixType()      { return &_gpsFixType;      }
    Fact* gpsSatellites()   { return &_gpsSatellites;   }
    Fact* gpsJammingState() { return &_gpsJammingState; }

    // MAVLink message handler — يفك ترميز DEBUG_FLOAT_ARRAY array_id=2
    void handleMessage(Vehicle *vehicle, const mavlink_message_t &message) override;

    // معرّف الـ array_id الخاص بنا
    static constexpr uint16_t ROCKET_ARRAY_ID = 2;

private:
    // P1
    Fact _stage            {0, "stage",          FactMetaData::valueTypeDouble};
    Fact _tFlight          {0, "tFlight",         FactMetaData::valueTypeDouble};
    Fact _qDyn             {0, "qDyn",            FactMetaData::valueTypeDouble};
    Fact _qKgf             {0, "qKgf",            FactMetaData::valueTypeDouble};
    Fact _pitchAccelCmd    {0, "pitchAccelCmd",   FactMetaData::valueTypeDouble};
    Fact _yawAccelCmd      {0, "yawAccelCmd",     FactMetaData::valueTypeDouble};
    Fact _blendAlpha       {0, "blendAlpha",      FactMetaData::valueTypeDouble};
    Fact _deltaRoll        {0, "deltaRoll",       FactMetaData::valueTypeDouble};
    Fact _deltaPitch       {0, "deltaPitch",      FactMetaData::valueTypeDouble};
    Fact _deltaYaw         {0, "deltaYaw",        FactMetaData::valueTypeDouble};
    Fact _fin1             {0, "fin1",            FactMetaData::valueTypeDouble};
    Fact _fin2             {0, "fin2",            FactMetaData::valueTypeDouble};
    Fact _fin3             {0, "fin3",            FactMetaData::valueTypeDouble};
    Fact _fin4             {0, "fin4",            FactMetaData::valueTypeDouble};
    Fact _altitude         {0, "altitude",        FactMetaData::valueTypeDouble};
    Fact _airspeed         {0, "airspeed",        FactMetaData::valueTypeDouble};
    Fact _rho              {0, "rho",             FactMetaData::valueTypeDouble};
    Fact _servoOnlineMask  {0, "servoOnlineMask", FactMetaData::valueTypeDouble};

    // P2
    Fact _phi              {0, "phi",             FactMetaData::valueTypeDouble};
    Fact _mpcSolveMs       {0, "mpcSolveMs",      FactMetaData::valueTypeDouble};
    Fact _mheQuality       {0, "mheQuality",      FactMetaData::valueTypeDouble};
    Fact _mpcSolveCount    {0, "mpcSolveCount",   FactMetaData::valueTypeDouble};
    Fact _theta            {0, "theta",           FactMetaData::valueTypeDouble};
    Fact _psi              {0, "psi",             FactMetaData::valueTypeDouble};
    Fact _alphaEst         {0, "alphaEst",        FactMetaData::valueTypeDouble};
    Fact _gammaRad         {0, "gammaRad",        FactMetaData::valueTypeDouble};
    Fact _posDownrange     {0, "posDownrange",    FactMetaData::valueTypeDouble};
    Fact _posCrossrange    {0, "posCrossrange",   FactMetaData::valueTypeDouble};
    Fact _velDownrange     {0, "velDownrange",    FactMetaData::valueTypeDouble};
    Fact _velDown          {0, "velDown",         FactMetaData::valueTypeDouble};
    Fact _velCrossrange    {0, "velCrossrange",   FactMetaData::valueTypeDouble};
    Fact _bearingDeg       {0, "bearingDeg",      FactMetaData::valueTypeDouble};
    Fact _targetRangeRem   {0, "targetRangeRem",  FactMetaData::valueTypeDouble};

    // P3
    Fact _launched         {0, "launched",        FactMetaData::valueTypeDouble};
    Fact _dtActual         {0, "dtActual",         FactMetaData::valueTypeDouble};
    Fact _dtMin            {0, "dtMin",            FactMetaData::valueTypeDouble};
    Fact _dtMax            {0, "dtMax",            FactMetaData::valueTypeDouble};
    Fact _mheSolveMs       {0, "mheSolveMs",       FactMetaData::valueTypeDouble};
    Fact _mpcSolverStatus  {0, "mpcSolverStatus",  FactMetaData::valueTypeDouble};
    Fact _mpcSqpIter       {0, "mpcSqpIter",       FactMetaData::valueTypeDouble};
    Fact _mheValid         {0, "mheValid",         FactMetaData::valueTypeDouble};
    Fact _mheStatus        {0, "mheStatus",        FactMetaData::valueTypeDouble};
    Fact _xvalGammaErr     {0, "xvalGammaErr",     FactMetaData::valueTypeDouble};
    Fact _xvalChiErr       {0, "xvalChiErr",       FactMetaData::valueTypeDouble};
    Fact _xvalAltErr       {0, "xvalAltErr",       FactMetaData::valueTypeDouble};
    Fact _xvalPenalty      {0, "xvalPenalty",      FactMetaData::valueTypeDouble};
    Fact _mheSolveUs       {0, "mheSolveUs",       FactMetaData::valueTypeDouble};
    Fact _mpcSolveUs       {0, "mpcSolveUs",       FactMetaData::valueTypeDouble};
    Fact _cycleUs          {0, "cycleUs",          FactMetaData::valueTypeDouble};

    // P4
    Fact _mpcFailCount     {0, "mpcFailCount",     FactMetaData::valueTypeDouble};
    Fact _mpcNanSkipCount  {0, "mpcNanSkipCount",  FactMetaData::valueTypeDouble};
    Fact _mheFailCount     {0, "mheFailCount",     FactMetaData::valueTypeDouble};
    Fact _finClampCount    {0, "finClampCount",    FactMetaData::valueTypeDouble};
    Fact _xvalResetCount   {0, "xvalResetCount",   FactMetaData::valueTypeDouble};
    Fact _servoOfflineEvt  {0, "servoOfflineEvt",  FactMetaData::valueTypeDouble};
    Fact _gpsFixType       {0, "gpsFixType",       FactMetaData::valueTypeDouble};
    Fact _gpsSatellites    {0, "gpsSatellites",    FactMetaData::valueTypeDouble};
    Fact _gpsJammingState  {0, "gpsJammingState",  FactMetaData::valueTypeDouble};
};
