#include "logging/FdrExporter.h"
#include "logging/FlightDataRecorder.h"
#include "test_support.h"

#include <filesystem>
#include <fstream>
#include <sstream>

using namespace m130::logging;

namespace {
std::string tmpBase(const char* tag)
{
    return (std::filesystem::temp_directory_path() /
            (std::string("m130_export_") + tag)).string();
}

void cleanup(const std::string& base, const std::string& extra = {})
{
    for (const char* ext : { ".m130raw", ".m130events", ".m130structured", ".m130session.json" }) {
        std::filesystem::remove(base + ext);
    }
    if (!extra.empty()) std::filesystem::remove(extra);
}

std::string slurp(const std::string& path)
{
    std::ifstream in(path);
    std::ostringstream os; os << in.rdbuf();
    return os.str();
}
} // namespace

int exportsCsv()
{
    const auto base = tmpBase("csv");
    const auto csv  = base + ".csv";
    cleanup(base, csv);

    {
        FlightDataRecorder fdr([n = std::uint64_t{0}]() mutable { return ++n; });
        fdr.open(base);
        RawFrame f;
        f.timestamp_us = 42; f.channel = 1; f.msg_id = 42001;
        f.payload = { 0xde, 0xad, 0xbe, 0xef };
        fdr.appendRaw(f);
        fdr.close();
    }

    const auto rows = FdrExporter::rawToCsv(base + ".m130raw", csv);
    M130_REQUIRE_EQ(rows, std::uint64_t(1));

    const auto text = slurp(csv);
    M130_REQUIRE(text.find("timestamp_us,channel,msg_id,payload_len,payload_hex") != std::string::npos);
    M130_REQUIRE(text.find("42,1,42001,4,deadbeef") != std::string::npos);

    cleanup(base, csv);
    return 0;
}

int exportsJsonLines()
{
    const auto base = tmpBase("jsonl");
    const auto out  = base + ".jsonl";
    cleanup(base, out);

    {
        FlightDataRecorder fdr([n = std::uint64_t{0}]() mutable { return ++n; });
        fdr.open(base);
        for (std::uint16_t i = 0; i < 3; ++i) {
            RawFrame f;
            f.timestamp_us = 1'000 + i;
            f.channel = 0;
            f.msg_id  = static_cast<std::uint16_t>(42000 + i);
            f.payload = { std::uint8_t(i) };
            fdr.appendRaw(f);
        }
        fdr.close();
    }

    const auto rows = FdrExporter::rawToJsonLines(base + ".m130raw", out);
    M130_REQUIRE_EQ(rows, std::uint64_t(3));

    const auto text = slurp(out);
    M130_REQUIRE(text.find("\"msg_id\":42000") != std::string::npos);
    M130_REQUIRE(text.find("\"msg_id\":42002") != std::string::npos);
    M130_REQUIRE(text.find("\"payload_hex\":\"02\"") != std::string::npos);

    cleanup(base, out);
    return 0;
}

int run()
{
    M130_RUN(exportsCsv);
    M130_RUN(exportsJsonLines);
    return 0;
}

M130_TEST_MAIN()
