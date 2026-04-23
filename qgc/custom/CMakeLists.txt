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
    # Access layer (Pillar 6: argon2id-ready hasher, TOTP, authenticator chain)
    ${CMAKE_CURRENT_SOURCE_DIR}/src/access/Role.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/access/AccessController.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/access/AccessController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/access/Authenticator.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/access/Authenticator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/access/Base32.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/access/Base32.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/access/PasswordHasher.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/access/PasswordHasher.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/access/Pbkdf2.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/access/Pbkdf2.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/access/SessionManager.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/access/SessionManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/access/Totp.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/access/Totp.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/access/UserManager.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/access/UserManager.h
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
    # Flight Data Recorder (Pillar 4)
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logging/Sha256.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logging/Sha256.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logging/Hmac.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logging/Hmac.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logging/ChainOfCustody.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logging/ChainOfCustody.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logging/FlightDataRecorder.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logging/FlightDataRecorder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logging/FdrReader.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logging/FdrReader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logging/FdrExporter.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logging/FdrExporter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logging/ReplayEngine.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logging/ReplayEngine.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logging/AuditLogger.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logging/AuditLogger.h
    # View-models (pure C++ — also compiled into standalone m130_core)
    ${CMAKE_CURRENT_SOURCE_DIR}/src/views/ChecklistModel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/views/ChecklistModel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/views/CountdownClock.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/views/CountdownClock.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/views/IipPredictor.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/views/IipPredictor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/views/ReplayController.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/views/ReplayController.h
    # Phase B — Qt-bound facades for Pillar 5
    ${CMAKE_CURRENT_SOURCE_DIR}/src/views/MissionController.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/views/MissionController.h
    # Analysis (pure C++ — FFT, window, spectrum, timeseries, MHE innovation)
    ${CMAKE_CURRENT_SOURCE_DIR}/src/analysis/RingBuffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/analysis/Timeseries.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/analysis/Timeseries.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/analysis/Window.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/analysis/Window.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/analysis/Fft.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/analysis/Fft.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/analysis/Spectrum.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/analysis/Spectrum.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/analysis/MheInnovationMonitor.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/analysis/MheInnovationMonitor.h
    # Phase B — Qt-bound facade for Pillar 7 analysis
    ${CMAKE_CURRENT_SOURCE_DIR}/src/analysis/AnalysisController.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/analysis/AnalysisController.h
    # Tuning (pure C++ — bounded MPC weight model + snapshot/rollback)
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tuning/TuningParameter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tuning/MpcTuningModel.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tuning/MpcTuningModel.h
    # Phase B — Qt-bound facade for Pillar 7 tuning
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tuning/TuningController.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tuning/TuningController.h
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
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logging
    ${CMAKE_CURRENT_SOURCE_DIR}/src/access
    ${CMAKE_CURRENT_SOURCE_DIR}/src/analysis
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tuning
    ${CMAKE_CURRENT_SOURCE_DIR}/src/views
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
