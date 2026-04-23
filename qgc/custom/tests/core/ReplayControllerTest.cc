#include "logging/FlightDataRecorder.h"
#include "test_support.h"
#include "views/ReplayController.h"

#include <filesystem>
#include <vector>

using namespace m130::logging;
using namespace m130::views;

namespace {
std::string tmpBase(const char* tag)
{
    return (std::filesystem::temp_directory_path() /
            (std::string("m130_replayctl_") + tag)).string();
}

void cleanup(const std::string& base)
{
    for (const char* ext : { ".m130raw", ".m130events", ".m130structured", ".m130session.json" }) {
        std::filesystem::remove(base + ext);
    }
}

std::string writeTestSession(const char* tag, std::size_t n_frames = 4, std::uint64_t dt_us = 100000)
{
    const auto base = tmpBase(tag);
    cleanup(base);
    FlightDataRecorder fdr;
    if (!fdr.open(base)) return {};
    std::vector<std::uint8_t> payload{0xaa, 0xbb, 0xcc, 0xdd};
    for (std::size_t i = 0; i < n_frames; ++i) {
        RawFrame f;
        f.timestamp_us = static_cast<std::uint64_t>(i + 1) * dt_us;
        f.channel = 0;
        f.msg_id  = static_cast<std::uint16_t>(42000 + i);
        f.payload = payload;
        fdr.appendRaw(f);
    }
    fdr.close();
    return base + ".m130raw";
}
} // namespace

static int emptyStateBeforeLoad()
{
    ReplayController rc;
    M130_REQUIRE_EQ(rc.state(), ReplayState::Empty);
    M130_REQUIRE(!rc.isPlaying());
    M130_REQUIRE_EQ(rc.step(1000), static_cast<std::size_t>(0));
    return 0;
}

static int loadPausedByDefault()
{
    const auto path = writeTestSession("loadPaused");
    M130_REQUIRE(!path.empty());
    ReplayController rc;
    M130_REQUIRE(rc.loadFile(path));
    M130_REQUIRE_EQ(rc.state(), ReplayState::Paused);
    M130_REQUIRE(!rc.isPlaying());
    M130_REQUIRE_EQ(rc.step(1'000'000), static_cast<std::size_t>(0)); // paused ⇒ no emission
    return 0;
}

static int playEmitsFramesAndReachesEof()
{
    const auto path = writeTestSession("playEmits", 4, 100'000);
    ReplayController rc;
    std::size_t emitted = 0;
    rc.setSink([&](const RawFrame&) { ++emitted; });
    M130_REQUIRE(rc.loadFile(path));
    rc.play(0);
    rc.setSpeed(10.0f); // fast → all frames in one step
    const auto n = rc.step(10'000'000);
    M130_REQUIRE_EQ(n, static_cast<std::size_t>(4));
    M130_REQUIRE_EQ(emitted, static_cast<std::size_t>(4));
    M130_REQUIRE_EQ(rc.state(), ReplayState::Finished);
    return 0;
}

static int pauseHaltsEmission()
{
    const auto path = writeTestSession("pauseHalts", 10, 100'000);
    ReplayController rc;
    std::size_t emitted = 0;
    rc.setSink([&](const RawFrame&) { ++emitted; });
    M130_REQUIRE(rc.loadFile(path));
    rc.play(0);
    rc.setSpeed(1.0f);
    rc.step(100'000); // should emit first frame
    const auto after_first = emitted;
    M130_REQUIRE(after_first >= 1);

    rc.pause();
    M130_REQUIRE(!rc.isPlaying());
    rc.step(10'000'000); // ignored while paused
    M130_REQUIRE_EQ(emitted, after_first);
    return 0;
}

static int rewindRestartsPlayback()
{
    const auto path = writeTestSession("rewindReplay", 3, 100'000);
    ReplayController rc;
    std::size_t emitted = 0;
    rc.setSink([&](const RawFrame&) { ++emitted; });
    M130_REQUIRE(rc.loadFile(path));
    rc.play(0);
    rc.setSpeed(100.0f);
    rc.step(10'000'000);
    M130_REQUIRE_EQ(rc.state(), ReplayState::Finished);
    M130_REQUIRE_EQ(emitted, static_cast<std::size_t>(3));

    M130_REQUIRE(rc.rewind());
    M130_REQUIRE_EQ(rc.state(), ReplayState::Paused);
    rc.play(0);
    rc.step(10'000'000);
    M130_REQUIRE_EQ(emitted, static_cast<std::size_t>(6));
    return 0;
}

static int run()
{
    M130_RUN(emptyStateBeforeLoad);
    M130_RUN(loadPausedByDefault);
    M130_RUN(playEmitsFramesAndReachesEof);
    M130_RUN(pauseHaltsEmission);
    M130_RUN(rewindRestartsPlayback);
    return 0;
}

M130_TEST_MAIN()
