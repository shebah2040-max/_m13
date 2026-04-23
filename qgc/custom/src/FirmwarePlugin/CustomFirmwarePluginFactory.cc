#include "CustomFirmwarePluginFactory.h"
#include "CustomFirmwarePlugin.h"

CustomFirmwarePluginFactory CustomFirmwarePluginFactoryImp;

CustomFirmwarePluginFactory::CustomFirmwarePluginFactory()
    : FirmwarePluginFactory()
{
}

QList<QGCMAVLink::FirmwareClass_t> CustomFirmwarePluginFactory::supportedFirmwareClasses() const
{
    return { QGCMAVLink::FirmwareClassPX4 };
}

QList<QGCMAVLink::VehicleClass_t> CustomFirmwarePluginFactory::supportedVehicleClasses() const
{
    // نقبل كل أنواع المركبات — الصاروخ قد يُعرَّف كأي نوع في HITL/SITL
    return {
        QGCMAVLink::VehicleClassGeneric,
        QGCMAVLink::VehicleClassFixedWing,
        QGCMAVLink::VehicleClassMultiRotor,
    };
}

FirmwarePlugin *CustomFirmwarePluginFactory::firmwarePluginForAutopilot(MAV_AUTOPILOT autopilotType, MAV_TYPE /*vehicleType*/)
{
    if (autopilotType == MAV_AUTOPILOT_PX4) {
        if (!_pluginInstance) {
            _pluginInstance = new CustomFirmwarePlugin();
        }
        return _pluginInstance;
    }
    return nullptr;
}
