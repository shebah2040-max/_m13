#include "logging/FlightDataRecorder.h"
#include "logging/ReplayEngine.h"
#include "test_support.h"

#include <filesystem>
#include <vector>

using namespace m130::logging;

namespace {
std::string tmpBase(const char* tag)
{
    return (std::filesystem::temp_directory_path() /
            (std::string("m130_replay_") + tag)).string();
}

void cleanup(const std::string& base)
{
    for (const char* ext : { ".m130raw", ".m130events", ".m130structured", ".m130session.json" }) {
        std::filesystem::remove(base + ext);
    }
}

void writeFixture(const std::string& base)
{
    FlightDataRecorder fdr([n = std::uint64_t{0}]() mutable { return ++n; });
    fdr.open(base);
    // 5 frames at 1 ms spacing.
    for (std::uint16_t i = 0; i < 5; ++i) {
        RawFrame f;
        f.timestamp_us = 1'000'000ULL + 1'000ULL * i;
        f.channel = 0;
        f.msg_id  = static_cast<std::uint16_t>(42000 + i);
        f.payload = { std::uint8_t(i) };
        fdr.appendRaw(f);
    }
    fdr.close();
}
} // namespace

int replaysAtRealTime()
{
    const auto base = tmpBase("rt");
    cleanup(base);
    writeFixture(base);

    ReplayEngine eng;
    M130_REQUIRE(eng.open(base + ".m130raw"));
    std::vector<std::uint16_t> seen;
    eng.setSink([&](const RawFrame& f) { seen.push_back(f.msg_id); });
    eng.setSpeed(1.0f);

    // Wall clock starts at t=10_000_000 us. Stream starts at 1_000_000 us.
    // At speed 1×, scaled_dt = stream_dt. So frame i is due at wall = 10M + i*1000.
    const std::uint64_t anchor = 10'000'000;
    eng.startAt(anchor);
    M130_REQUIRE_EQ(eng.step(anchor),            std::size_t(1));                    // frame 0
    M130_REQUIRE_EQ(eng.step(anchor +   999),    std::size_t(0));                    // not yet
    M130_REQUIRE_EQ(eng.step(anchor + 1'000),    std::size_t(1));                    // frame 1
    M130_REQUIRE_EQ(eng.step(anchor + 5'000),    std::size_t(3));                    // frames 2,3,4
    M130_REQUIRE(eng.isEof());
    M130_REQUIRE_EQ(seen.size(), std::size_t(5));
    M130_REQUIRE_EQ(seen.front(), std::uint16_t(42000));
    M130_REQUIRE_EQ(seen.back(),  std::uint16_t(42004));
    cleanup(base);
    return 0;
}

int replaysAtTenX()
{
    const auto base = tmpBase("10x");
    cleanup(base);
    writeFixture(base);

    ReplayEngine eng;
    M130_REQUIRE(eng.open(base + ".m130raw"));
    std::size_t count = 0;
    eng.setSink([&](const RawFrame&) { ++count; });
    eng.setSpeed(10.0f);

    const std::uint64_t anchor = 0;
    eng.startAt(anchor);
    // 5 frames span 4000 us real, → 400 us scaled. Advancing 1000 us wall
    // time flushes everything.
    const std::size_t n = eng.step(anchor + 1000);
    M130_REQUIRE_EQ(n, std::size_t(5));
    M130_REQUIRE_EQ(count, std::size_t(5));
    M130_REQUIRE(eng.isEof());
    cleanup(base);
    return 0;
}

int replaysAtSlowSpeed()
{
    const auto base = tmpBase("slow");
    cleanup(base);
    writeFixture(base);

    ReplayEngine eng;
    M130_REQUIRE(eng.open(base + ".m130raw"));
    std::size_t count = 0;
    eng.setSink([&](const RawFrame&) { ++count; });
    eng.setSpeed(0.5f);  // half speed

    const std::uint64_t anchor = 0;
    eng.startAt(anchor);
    // Real stream dt for last frame = 4000 us → scaled dt = 8000 us at 0.5×.
    M130_REQUIRE_EQ(eng.step(anchor + 7'999), std::size_t(4));  // frames 0..3
    M130_REQUIRE_EQ(eng.step(anchor + 8'000), std::size_t(1));  // frame 4
    M130_REQUIRE(eng.isEof());
    M130_REQUIRE_EQ(count, std::size_t(5));
    cleanup(base);
    return 0;
}

int clampsExtremeSpeeds()
{
    ReplayEngine eng;
    eng.setSpeed(-1.0f);
    M130_REQUIRE(eng.speed() >= 0.1f);
    eng.setSpeed(1000.0f);
    M130_REQUIRE(eng.speed() <= 100.0f);
    eng.setSpeed(0.0f);
    M130_REQUIRE_EQ(eng.speed(), 0.1f);
    return 0;
}

int rewindRestartsStream()
{
    const auto base = tmpBase("rw");
    cleanup(base);
    writeFixture(base);

    ReplayEngine eng;
    M130_REQUIRE(eng.open(base + ".m130raw"));
    std::size_t count = 0;
    eng.setSink([&](const RawFrame&) { ++count; });
    eng.setSpeed(10.0f);
    eng.startAt(0);
    eng.step(10'000);
    M130_REQUIRE(eng.isEof());
    M130_REQUIRE(eng.rewind());
    M130_REQUIRE(!eng.isEof());
    eng.startAt(100'000);
    const auto after = count;
    eng.step(200'000);
    M130_REQUIRE(count > after);
    cleanup(base);
    return 0;
}

int run()
{
    M130_RUN(replaysAtRealTime);
    M130_RUN(replaysAtTenX);
    M130_RUN(replaysAtSlowSpeed);
    M130_RUN(clampsExtremeSpeeds);
    M130_RUN(rewindRestartsStream);
    return 0;
}

M130_TEST_MAIN()
