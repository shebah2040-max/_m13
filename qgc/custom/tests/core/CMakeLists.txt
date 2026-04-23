cmake_minimum_required(VERSION 3.22)

# Standalone Foundation test target. Does NOT depend on Qt/QGC.
#
# Builds the pure-C++ M130 core (safety, access, protocol, logging) as a
# static library and links a set of self-contained assertion-based unit
# tests. The tests are plain executables whose exit code is 0 on success —
# wired into CTest so they run under ctest --output-on-failure.

project(m130_core_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT DEFINED M130_CUSTOM_DIR)
    get_filename_component(M130_CUSTOM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
endif()

add_library(m130_core STATIC
    ${M130_CUSTOM_DIR}/src/safety/MissionStateMachine.cc
    ${M130_CUSTOM_DIR}/src/safety/Watchdog.cc
    ${M130_CUSTOM_DIR}/src/safety/AlertManager.cc
    ${M130_CUSTOM_DIR}/src/safety/CommandAuthorization.cc
    ${M130_CUSTOM_DIR}/src/safety/FlightSafetyEnvelope.cc
    ${M130_CUSTOM_DIR}/src/safety/FlightTerminationService.cc
    ${M130_CUSTOM_DIR}/src/safety/SafetyAggregator.cc
    ${M130_CUSTOM_DIR}/src/logging/AuditLogger.cc
    ${M130_CUSTOM_DIR}/src/logging/ChainOfCustody.cc
    ${M130_CUSTOM_DIR}/src/logging/FdrExporter.cc
    ${M130_CUSTOM_DIR}/src/logging/FdrReader.cc
    ${M130_CUSTOM_DIR}/src/logging/FlightDataRecorder.cc
    ${M130_CUSTOM_DIR}/src/logging/Hmac.cc
    ${M130_CUSTOM_DIR}/src/logging/ReplayEngine.cc
    ${M130_CUSTOM_DIR}/src/logging/Sha256.cc
    ${M130_CUSTOM_DIR}/src/access/UserManager.cc
    ${M130_CUSTOM_DIR}/src/access/SessionManager.cc
    ${M130_CUSTOM_DIR}/src/protocol/ProtocolVersion.cc
    ${M130_CUSTOM_DIR}/src/protocol/MessageRouter.cc
    ${M130_CUSTOM_DIR}/src/protocol/M130Dialect.cc
    ${M130_CUSTOM_DIR}/src/protocol/generated/M130Messages.generated.cc
    ${M130_CUSTOM_DIR}/src/protocol/generated/M130DialectTable.generated.cc
)
target_include_directories(m130_core PUBLIC ${M130_CUSTOM_DIR}/src)

if(MSVC)
    target_compile_options(m130_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(m130_core PRIVATE -Wall -Wextra -Wpedantic -Wshadow -Wconversion)
endif()

enable_testing()

function(m130_add_test test_name)
    add_executable(${test_name} ${test_name}.cc)
    target_link_libraries(${test_name} PRIVATE m130_core)
    target_include_directories(${test_name} PRIVATE ${M130_CUSTOM_DIR}/src)
    add_test(NAME ${test_name} COMMAND ${test_name})
    set_tests_properties(${test_name} PROPERTIES LABELS "Unit;M130")
endfunction()

m130_add_test(MissionStateMachineTest)
m130_add_test(WatchdogTest)
m130_add_test(AlertManagerTest)
m130_add_test(CommandAuthorizationTest)
m130_add_test(FlightSafetyEnvelopeTest)
m130_add_test(FlightTerminationServiceTest)
m130_add_test(AuditLoggerTest)
m130_add_test(ChainOfCustodyTest)
m130_add_test(FlightDataRecorderTest)
m130_add_test(UserManagerTest)
m130_add_test(SessionManagerTest)
m130_add_test(ProtocolVersionTest)
m130_add_test(MessageRouterTest)
m130_add_test(M130DialectTest)
m130_add_test(SafetyAggregatorTest)
m130_add_test(M130GeneratedCodecTest)
m130_add_test(Sha256Test)
m130_add_test(HmacSha256Test)
m130_add_test(FdrReaderTest)
m130_add_test(ReplayEngineTest)
m130_add_test(FdrExporterTest)
