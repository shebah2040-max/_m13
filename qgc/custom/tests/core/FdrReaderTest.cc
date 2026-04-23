#include "logging/FdrReader.h"
#include "logging/FlightDataRecorder.h"
#include "test_support.h"

#include <filesystem>

using namespace m130::logging;

namespace {
std::string tmpBase(const char* tag)
{
    return (std::filesystem::temp_directory_path() /
            (std::string("m130_fdrrd_") + tag)).string();
}

void cleanup(const std::string& base)
{
    for (const char* ext : { ".m130raw", ".m130events", ".m130structured", ".m130session.json" }) {
        std::filesystem::remove(base + ext);
    }
}
} // namespace

int rawRoundTrip()
{
    const auto base = tmpBase("raw");
    cleanup(base);

    {
        FlightDataRecorder fdr([n = std::uint64_t{0}]() mutable { return ++n; });
        M130_REQUIRE(fdr.open(base));
        for (std::uint16_t i = 0; i < 5; ++i) {
            RawFrame f;
            f.timestamp_us = 1000u + 100u * i;
            f.channel = static_cast<std::uint8_t>(i % 2);
            f.msg_id  = static_cast<std::uint16_t>(42000 + i);
            f.payload = { std::uint8_t(i), std::uint8_t(i + 1), std::uint8_t(i + 2) };
            fdr.appendRaw(f);
        }
        fdr.close();
    }

    FdrRawReader rd;
    M130_REQUIRE(rd.open(base + ".m130raw"));
    const auto frames = rd.readAll();
    M130_REQUIRE_EQ(frames.size(), std::size_t(5));
    for (std::size_t i = 0; i < frames.size(); ++i) {
        const auto& f = frames[i];
        M130_REQUIRE_EQ(f.timestamp_us, std::uint64_t(1000u + 100u * i));
        M130_REQUIRE_EQ(static_cast<int>(f.channel), static_cast<int>(i % 2));
        M130_REQUIRE_EQ(f.msg_id, std::uint16_t(42000 + i));
        M130_REQUIRE_EQ(f.payload.size(), std::size_t(3));
    }

    cleanup(base);
    return 0;
}

int eventRoundTrip()
{
    const auto base = tmpBase("evt");
    cleanup(base);
    {
        FlightDataRecorder fdr;
        M130_REQUIRE(fdr.open(base));
        for (int i = 0; i < 3; ++i) {
            EventRecord e;
            e.timestamp_ms = 1000 + i;
            e.level = (i == 0 ? "info" : (i == 1 ? "warn" : "alert"));
            e.source = "safety.watchdog";
            e.message = "line ";
            e.message += char('A' + i);
            fdr.appendEvent(e);
        }
        fdr.close();
    }

    FdrEventReader er;
    M130_REQUIRE(er.open(base + ".m130events"));
    const auto events = er.readAll();
    M130_REQUIRE_EQ(events.size(), std::size_t(3));
    M130_REQUIRE_EQ(events[0].level,   std::string("info"));
    M130_REQUIRE_EQ(events[1].level,   std::string("warn"));
    M130_REQUIRE_EQ(events[2].level,   std::string("alert"));
    M130_REQUIRE_EQ(events[0].source,  std::string("safety.watchdog"));
    M130_REQUIRE_EQ(events[2].message, std::string("line C"));
    cleanup(base);
    return 0;
}

int rejectsTruncatedTail()
{
    const auto base = tmpBase("trunc");
    cleanup(base);
    {
        FlightDataRecorder fdr([n = std::uint64_t{0}]() mutable { return ++n; });
        M130_REQUIRE(fdr.open(base));
        RawFrame f; f.timestamp_us = 1; f.msg_id = 7; f.payload = { 1, 2, 3, 4 };
        fdr.appendRaw(f);
        fdr.close();
    }

    // Append garbage header without full payload.
    {
        std::ofstream out(base + ".m130raw", std::ios::binary | std::ios::app);
        const std::uint64_t ts = 2;
        const std::uint8_t  ch = 0;
        const std::uint16_t id = 9;
        const std::uint32_t len = 8; // promise 8 bytes
        out.write(reinterpret_cast<const char*>(&ts), sizeof(ts));
        out.write(reinterpret_cast<const char*>(&ch), sizeof(ch));
        out.write(reinterpret_cast<const char*>(&id), sizeof(id));
        out.write(reinterpret_cast<const char*>(&len), sizeof(len));
        const std::uint8_t partial[3] = { 1, 2, 3 };
        out.write(reinterpret_cast<const char*>(partial), sizeof(partial));
    }

    FdrRawReader rd;
    M130_REQUIRE(rd.open(base + ".m130raw"));
    const auto frames = rd.readAll();
    M130_REQUIRE_EQ(frames.size(), std::size_t(1)); // only the good one
    cleanup(base);
    return 0;
}

int run()
{
    M130_RUN(rawRoundTrip);
    M130_RUN(eventRoundTrip);
    M130_RUN(rejectsTruncatedTail);
    return 0;
}

M130_TEST_MAIN()
