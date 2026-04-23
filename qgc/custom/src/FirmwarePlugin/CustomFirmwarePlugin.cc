#include "CustomFirmwarePlugin.h"
#include "RocketTelemetryFactGroup.h"

CustomFirmwarePlugin::CustomFirmwarePlugin()
{
    _rocketFactGroup = new RocketTelemetryFactGroup(this);
    _factGroupMap[QStringLiteral("rocket")] = _rocketFactGroup;
}

CustomFirmwarePlugin::~CustomFirmwarePlugin()
{
    // _rocketFactGroup ملكيته لـ this (parent) — يُحذف تلقائياً
}

QMap<QString, FactGroup*>* CustomFirmwarePlugin::factGroups()
{
    return &_factGroupMap;
}
