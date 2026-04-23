#pragma once

#include "PX4FirmwarePlugin.h"

class Vehicle;
class RocketTelemetryFactGroup;
class M130GncStateFactGroup;

// ============================================================================
// CustomFirmwarePlugin
//
// يرث PX4FirmwarePlugin ويُضيف RocketTelemetryFactGroup إلى كل Vehicle
// يتصل بالمحطة الأرضية.
//
// القيد: يدعم فقط PX4 على أي نوع مركبة (الصاروخ يُعرَّف كـ Generic)
// ============================================================================

class CustomFirmwarePlugin : public PX4FirmwarePlugin
{
    Q_OBJECT

public:
    CustomFirmwarePlugin();
    ~CustomFirmwarePlugin() override;

    // يُضيف FactGroups الصاروخية إلى Vehicle عند الاتصال
    QMap<QString, FactGroup*>* factGroups() final;

    M130GncStateFactGroup *m130GncFactGroup() const noexcept { return _m130GncFactGroup; }

private:
    RocketTelemetryFactGroup *_rocketFactGroup = nullptr;
    M130GncStateFactGroup    *_m130GncFactGroup = nullptr;
    QMap<QString, FactGroup*> _factGroupMap;
};
