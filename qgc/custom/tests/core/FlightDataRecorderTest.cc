#include "logging/FlightDataRecorder.h"
#include "test_support.h"

#include <filesystem>

using namespace m130::logging;

int writesRawAndEvents()
{
    FlightDataRecorder fdr;
    auto base = (std::filesystem::temp_directory_path() / "m130_fdr_test").string();
    std::filesystem::remove(base + ".m130raw");
    std::filesystem::remove(base + ".m130events");
    M130_REQUIRE(fdr.open(base));

    RawFrame f;
    f.timestamp_us = 1;
    f.channel = 0;
    f.msg_id  = 42001;
    f.payload = { 0x01, 0x02, 0x03, 0x04 };
    fdr.appendRaw(f);

    EventRecord e;
    e.timestamp_ms = 1;
    e.level = "info";
    e.source = "safety.watchdog";
    e.message = "heartbeat stale";
    fdr.appendEvent(e);

    M130_REQUIRE_EQ(fdr.rawFrameCount(), std::uint64_t(1));
    M130_REQUIRE_EQ(fdr.eventCount(),    std::uint64_t(1));
    M130_REQUIRE(fdr.bytesWritten() >= sizeof(uint64_t) + sizeof(uint8_t) + sizeof(uint16_t) + sizeof(uint32_t) + 4);

    fdr.close();
    M130_REQUIRE(std::filesystem::exists(base + ".m130raw"));
    M130_REQUIRE(std::filesystem::exists(base + ".m130events"));
    std::filesystem::remove(base + ".m130raw");
    std::filesystem::remove(base + ".m130events");
    return 0;
}

int run()
{
    M130_RUN(writesRawAndEvents);
    return 0;
}

M130_TEST_MAIN()
