#include "FdrExporter.h"

#include "FdrReader.h"

#include <fstream>
#include <sstream>
#include <iomanip>

namespace m130::logging {

namespace {
std::string hexOf(const std::vector<std::uint8_t>& bytes)
{
    std::ostringstream os;
    os << std::hex << std::setfill('0');
    for (auto b : bytes) os << std::setw(2) << static_cast<unsigned>(b);
    return os.str();
}
} // namespace

std::uint64_t FdrExporter::rawToCsv(const std::string& raw_path, const std::string& csv_path)
{
    FdrRawReader reader;
    if (!reader.open(raw_path)) return 0;

    std::ofstream out(csv_path, std::ios::trunc);
    if (!out.is_open()) return 0;

    out << "timestamp_us,channel,msg_id,payload_len,payload_hex\n";
    std::uint64_t rows = 0;
    while (true) {
        auto f = reader.next();
        if (!f) break;
        out << f->timestamp_us << ','
            << static_cast<unsigned>(f->channel) << ','
            << f->msg_id << ','
            << f->payload.size() << ','
            << hexOf(f->payload) << '\n';
        ++rows;
    }
    return rows;
}

std::uint64_t FdrExporter::rawToJsonLines(const std::string& raw_path, const std::string& jsonl_path)
{
    FdrRawReader reader;
    if (!reader.open(raw_path)) return 0;

    std::ofstream out(jsonl_path, std::ios::trunc);
    if (!out.is_open()) return 0;

    std::uint64_t rows = 0;
    while (true) {
        auto f = reader.next();
        if (!f) break;
        out << R"({"ts_us":)" << f->timestamp_us
            << R"(,"channel":)" << static_cast<unsigned>(f->channel)
            << R"(,"msg_id":)" << f->msg_id
            << R"(,"payload_len":)" << f->payload.size()
            << R"(,"payload_hex":")" << hexOf(f->payload) << "\"}\n";
        ++rows;
    }
    return rows;
}

} // namespace m130::logging
