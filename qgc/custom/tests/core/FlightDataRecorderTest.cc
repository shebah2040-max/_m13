#include "logging/FlightDataRecorder.h"
#include "test_support.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

using namespace m130::logging;

namespace {
std::string slurp(const std::string& path)
{
    std::ifstream in(path);
    std::ostringstream os;
    os << in.rdbuf();
    return os.str();
}
} // namespace

int writesAllThreeTracks()
{
    FlightDataRecorder fdr([n = std::uint64_t{0}]() mutable { return ++n; });
    auto base = (std::filesystem::temp_directory_path() / "m130_fdr_v2_test").string();
    for (const char* ext : { ".m130raw", ".m130events", ".m130structured", ".m130session.json" }) {
        std::filesystem::remove(base + ext);
    }
    M130_REQUIRE(fdr.open(base));

    RawFrame f;
    f.timestamp_us = 100;
    f.channel = 0;
    f.msg_id  = 42001;
    f.payload = { 0x01, 0x02, 0x03, 0x04 };
    fdr.appendRaw(f);

    EventRecord e;
    e.timestamp_ms = 1000;
    e.level = "info";
    e.source = "safety.watchdog";
    e.message = "heartbeat stale";
    fdr.appendEvent(e);

    StructuredSample s;
    s.timestamp_us = 110;
    s.msg_id = 42001;
    s.msg_name = "M130_GNC_STATE";
    s.numeric = {
        {"stage", "2"},
        {"q_dyn", "1234.5"},
        {"phi",   "0.125"},
    };
    s.text = { {"mode", "CRUISE"} };
    fdr.appendStructured(s);

    M130_REQUIRE_EQ(fdr.rawFrameCount(),   std::uint64_t(1));
    M130_REQUIRE_EQ(fdr.eventCount(),      std::uint64_t(1));
    M130_REQUIRE_EQ(fdr.structuredCount(), std::uint64_t(1));

    fdr.close();

    // All four artefacts must exist after close.
    M130_REQUIRE(std::filesystem::exists(base + ".m130raw"));
    M130_REQUIRE(std::filesystem::exists(base + ".m130events"));
    M130_REQUIRE(std::filesystem::exists(base + ".m130structured"));
    M130_REQUIRE(std::filesystem::exists(base + ".m130session.json"));

    // Structured line contains the decoded field bag.
    const std::string structured = slurp(base + ".m130structured");
    M130_REQUIRE(structured.find("\"msg_id\":42001") != std::string::npos);
    M130_REQUIRE(structured.find("\"M130_GNC_STATE\"") != std::string::npos);
    M130_REQUIRE(structured.find("\"q_dyn\":1234.5") != std::string::npos);
    M130_REQUIRE(structured.find("\"mode\":\"CRUISE\"") != std::string::npos);

    // Session manifest references all three tracks with SHA-256 hashes.
    const std::string manifest = slurp(base + ".m130session.json");
    M130_REQUIRE(manifest.find("\"schema_version\"") != std::string::npos);
    M130_REQUIRE(manifest.find("\"raw\":")           != std::string::npos);
    M130_REQUIRE(manifest.find("\"events\":")        != std::string::npos);
    M130_REQUIRE(manifest.find("\"structured\":")    != std::string::npos);
    M130_REQUIRE(manifest.find("\"sha256\"")         != std::string::npos);

    for (const char* ext : { ".m130raw", ".m130events", ".m130structured", ".m130session.json" }) {
        std::filesystem::remove(base + ext);
    }
    return 0;
}

int run()
{
    M130_RUN(writesAllThreeTracks);
    return 0;
}

M130_TEST_MAIN()
