#pragma once

#include "FactGroup.h"

namespace m130::protocol::gen { struct M130GncState; }

// ============================================================================
// M130GncStateFactGroup
//
// FactGroup مدعوم بالرسالة الرسمية M130_GNC_STATE (msgid 42001) المُعرَّفة في
// qgc/custom/mavlink/m130.xml. الحقول هنا مطابقة 1:1 لحقول البنية المُولَّدة
// m130::protocol::gen::M130GncState، لذا تغييرها يحدث تلقائياً عبر إعادة
// تشغيل tools/generate-dialect.py ثم تحديث هذه الكتلة.
//
// يُضاف هذا الـ FactGroup إلى Vehicle عبر CustomFirmwarePlugin::factGroups()
// ويصبح متاحاً في QML بصيغة:
//   vehicle.getFactGroup("m130Gnc").altitude.rawValue
//
// ملاحظة هجرة (Pillar 2): يتعايش هذا الـ FactGroup مع RocketTelemetryFactGroup
// القديم حتى يبثّ الـ firmware الرسالة الجديدة. كلاهما مُسجَّل في الـ Vehicle،
// والأحدث تبثّاً هو الذي تعتمد عليه الواجهة.
// ============================================================================

class M130GncStateFactGroup : public FactGroup
{
    Q_OBJECT

    Q_PROPERTY(Fact* timeUsec        READ timeUsec        CONSTANT)
    Q_PROPERTY(Fact* stage           READ stage           CONSTANT)
    Q_PROPERTY(Fact* launched        READ launched        CONSTANT)
    Q_PROPERTY(Fact* mode            READ mode            CONSTANT)
    Q_PROPERTY(Fact* qDyn            READ qDyn            CONSTANT)
    Q_PROPERTY(Fact* rho             READ rho             CONSTANT)
    Q_PROPERTY(Fact* altitude        READ altitude        CONSTANT)
    Q_PROPERTY(Fact* airspeed        READ airspeed        CONSTANT)
    Q_PROPERTY(Fact* phi             READ phi             CONSTANT)
    Q_PROPERTY(Fact* theta           READ theta           CONSTANT)
    Q_PROPERTY(Fact* psi             READ psi             CONSTANT)
    Q_PROPERTY(Fact* alphaEst        READ alphaEst        CONSTANT)
    Q_PROPERTY(Fact* gammaRad        READ gammaRad        CONSTANT)
    Q_PROPERTY(Fact* posDownrange    READ posDownrange    CONSTANT)
    Q_PROPERTY(Fact* posCrossrange   READ posCrossrange   CONSTANT)
    Q_PROPERTY(Fact* velDownrange    READ velDownrange    CONSTANT)
    Q_PROPERTY(Fact* velDown         READ velDown         CONSTANT)
    Q_PROPERTY(Fact* velCrossrange   READ velCrossrange   CONSTANT)
    Q_PROPERTY(Fact* bearingDeg      READ bearingDeg      CONSTANT)
    Q_PROPERTY(Fact* targetRangeRem  READ targetRangeRem  CONSTANT)
    Q_PROPERTY(Fact* iipLat          READ iipLat          CONSTANT)
    Q_PROPERTY(Fact* iipLon          READ iipLon          CONSTANT)
    Q_PROPERTY(Fact* iipAlt          READ iipAlt          CONSTANT)
    Q_PROPERTY(Fact* messagesReceived READ messagesReceived CONSTANT)

public:
    explicit M130GncStateFactGroup(QObject *parent = nullptr);

    Fact* timeUsec()       { return &_timeUsec;       }
    Fact* stage()          { return &_stage;          }
    Fact* launched()       { return &_launched;       }
    Fact* mode()           { return &_mode;           }
    Fact* qDyn()           { return &_qDyn;           }
    Fact* rho()            { return &_rho;            }
    Fact* altitude()       { return &_altitude;       }
    Fact* airspeed()       { return &_airspeed;       }
    Fact* phi()            { return &_phi;            }
    Fact* theta()          { return &_theta;          }
    Fact* psi()            { return &_psi;            }
    Fact* alphaEst()       { return &_alphaEst;       }
    Fact* gammaRad()       { return &_gammaRad;       }
    Fact* posDownrange()   { return &_posDownrange;   }
    Fact* posCrossrange()  { return &_posCrossrange;  }
    Fact* velDownrange()   { return &_velDownrange;   }
    Fact* velDown()        { return &_velDown;        }
    Fact* velCrossrange()  { return &_velCrossrange;  }
    Fact* bearingDeg()     { return &_bearingDeg;     }
    Fact* targetRangeRem() { return &_targetRangeRem; }
    Fact* iipLat()         { return &_iipLat;         }
    Fact* iipLon()         { return &_iipLon;         }
    Fact* iipAlt()          { return &_iipAlt;         }
    Fact* messagesReceived(){ return &_messagesReceived; }

    /// Copy the decoded struct into the Facts. Safe to call from the main
    /// thread (same thread as the parent Vehicle).
    void applyState(const m130::protocol::gen::M130GncState& s);

private:
    Fact _timeUsec        {0, "timeUsec",        FactMetaData::valueTypeDouble};
    Fact _stage           {0, "stage",           FactMetaData::valueTypeUint8};
    Fact _launched        {0, "launched",        FactMetaData::valueTypeUint8};
    Fact _mode            {0, "mode",            FactMetaData::valueTypeUint16};
    Fact _qDyn            {0, "qDyn",            FactMetaData::valueTypeDouble};
    Fact _rho             {0, "rho",             FactMetaData::valueTypeDouble};
    Fact _altitude        {0, "altitude",        FactMetaData::valueTypeDouble};
    Fact _airspeed        {0, "airspeed",        FactMetaData::valueTypeDouble};
    Fact _phi             {0, "phi",             FactMetaData::valueTypeDouble};
    Fact _theta           {0, "theta",           FactMetaData::valueTypeDouble};
    Fact _psi             {0, "psi",             FactMetaData::valueTypeDouble};
    Fact _alphaEst        {0, "alphaEst",        FactMetaData::valueTypeDouble};
    Fact _gammaRad        {0, "gammaRad",        FactMetaData::valueTypeDouble};
    Fact _posDownrange    {0, "posDownrange",    FactMetaData::valueTypeDouble};
    Fact _posCrossrange   {0, "posCrossrange",   FactMetaData::valueTypeDouble};
    Fact _velDownrange    {0, "velDownrange",    FactMetaData::valueTypeDouble};
    Fact _velDown         {0, "velDown",         FactMetaData::valueTypeDouble};
    Fact _velCrossrange   {0, "velCrossrange",   FactMetaData::valueTypeDouble};
    Fact _bearingDeg      {0, "bearingDeg",      FactMetaData::valueTypeDouble};
    Fact _targetRangeRem  {0, "targetRangeRem",  FactMetaData::valueTypeDouble};
    Fact _iipLat          {0, "iipLat",          FactMetaData::valueTypeDouble};
    Fact _iipLon          {0, "iipLon",          FactMetaData::valueTypeDouble};
    Fact _iipAlt          {0, "iipAlt",          FactMetaData::valueTypeDouble};
    Fact _messagesReceived{0, "messagesReceived", FactMetaData::valueTypeUint64};
};
