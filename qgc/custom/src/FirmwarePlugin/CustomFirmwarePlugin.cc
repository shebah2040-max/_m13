#include "CustomFirmwarePlugin.h"
#include "RocketTelemetryFactGroup.h"
#include "telemetry/M130GncStateFactGroup.h"

CustomFirmwarePlugin::CustomFirmwarePlugin()
{
    _rocketFactGroup = new RocketTelemetryFactGroup(this);
    _factGroupMap[QStringLiteral("rocket")] = _rocketFactGroup;

    // Pillar 2 — expose the dialect-driven GNC state FactGroup alongside the
    // legacy DEBUG_FLOAT_ARRAY group. Both are populated by CustomPlugin's
    // mavlinkMessage dispatcher; UIs may migrate at their own pace.
    _m130GncFactGroup = new M130GncStateFactGroup(this);
    _factGroupMap[QStringLiteral("m130Gnc")] = _m130GncFactGroup;
}

CustomFirmwarePlugin::~CustomFirmwarePlugin()
{
    // _rocketFactGroup ملكيته لـ this (parent) — يُحذف تلقائياً
}

QMap<QString, FactGroup*>* CustomFirmwarePlugin::factGroups()
{
    return &_factGroupMap;
}
