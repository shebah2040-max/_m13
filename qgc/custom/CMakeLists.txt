# ============================================================================
# M130 GCS — Custom QGroundControl Plugin
# محطة أرضية متخصصة للصاروخ M130 / قبضة1
# ============================================================================

message(STATUS "QGC: Adding M130 Rocket GCS Plugin")

# ============================================================================
# Custom Resources and Import Paths
# ============================================================================

list(APPEND CUSTOM_RESOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/custom.qrc
)
set(QGC_RESOURCES ${QGC_RESOURCES} ${CUSTOM_RESOURCES} CACHE STRING "Paths to .qrc Resources" FORCE)

set(QML_IMPORT_PATH ${QML_IMPORT_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/res" CACHE STRING "Extra qml import paths" FORCE)

# ============================================================================
# Custom Plugin Sources
# ============================================================================

set(CUSTOM_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CustomPlugin.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CustomPlugin.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RocketTelemetryFactGroup.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RocketTelemetryFactGroup.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FirmwarePlugin/CustomFirmwarePlugin.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FirmwarePlugin/CustomFirmwarePlugin.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FirmwarePlugin/CustomFirmwarePluginFactory.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FirmwarePlugin/CustomFirmwarePluginFactory.h
    # Safety Kernel (pure-C++ cores + Qt facade)
    ${CMAKE_CURRENT_SOURCE_DIR}/src/safety/MissionStateMachine.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/safety/MissionStateMachine.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/safety/Watchdog.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/safety/Watchdog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/safety/AlertManager.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/safety/AlertManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/safety/CommandAuthorization.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/safety/CommandAuthorization.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/safety/FlightSafetyEnvelope.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/safety/FlightSafetyEnvelope.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/safety/FlightTerminationService.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/safety/FlightTerminationService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/safety/SafetyAggregator.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/safety/SafetyAggregator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/safety/AlertLevel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/safety/FlightPhase.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/safety/AlertListModel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/safety/AlertListModel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/safety/SafetyKernel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/safety/SafetyKernel.h
    # Access layer (used by CommandAuthorization)
    ${CMAKE_CURRENT_SOURCE_DIR}/src/access/Role.h
    # Protocol layer (pure-C++) + generated dialect
    ${CMAKE_CURRENT_SOURCE_DIR}/src/protocol/ProtocolVersion.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/protocol/ProtocolVersion.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/protocol/MessageRouter.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/protocol/MessageRouter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/protocol/M130Dialect.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/protocol/M130Dialect.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/protocol/generated/M130Enums.generated.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/protocol/generated/M130Messages.generated.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/protocol/generated/M130Messages.generated.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/protocol/generated/M130DialectTable.generated.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/protocol/generated/M130DialectTable.generated.h
    # Telemetry FactGroups driven by the generated dialect
    ${CMAKE_CURRENT_SOURCE_DIR}/src/telemetry/M130GncStateFactGroup.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/telemetry/M130GncStateFactGroup.h
    CACHE INTERNAL "" FORCE
)

# ============================================================================
# Custom Build Configuration
# ============================================================================

set(CUSTOM_LIBRARIES
    CACHE INTERNAL "" FORCE
)

set(CUSTOM_INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FirmwarePlugin
    ${CMAKE_CURRENT_SOURCE_DIR}/src/safety
    ${CMAKE_CURRENT_SOURCE_DIR}/src/protocol
    ${CMAKE_CURRENT_SOURCE_DIR}/src/protocol/generated
    CACHE INTERNAL "" FORCE
)

set(CUSTOM_DEFINITIONS
    QGC_CUSTOM_BUILD
    CUSTOMHEADER="CustomPlugin.h"
    CUSTOMCLASS=CustomPlugin
    CACHE INTERNAL "" FORCE
)

set(CUSTOM_QT_COMPONENTS
    Core
    Qml
    CACHE INTERNAL "" FORCE
)
