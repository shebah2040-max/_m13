#pragma once

#include <cstdint>
#include <string>

namespace m130::logging {

/// Post-flight exporters for FDR files.
///
/// CSV and JSON-lines formats are chosen because they are consumable by
/// stdlib tooling (pandas, numpy, grep, jq) without requiring Parquet/Arrow.
/// The structured JSONL track is already human-readable; the exporter below
/// is used mainly to flatten the binary raw track.
class FdrExporter
{
public:
    /// Convert a `.m130raw` file to a CSV with header
    /// `timestamp_us,channel,msg_id,payload_len,payload_hex`.
    /// Returns the number of rows written (not counting the header). If the
    /// raw file is missing/corrupt, returns 0.
    static std::uint64_t rawToCsv(const std::string& raw_path, const std::string& csv_path);

    /// Convert a `.m130raw` file to a JSON-lines stream.
    static std::uint64_t rawToJsonLines(const std::string& raw_path, const std::string& jsonl_path);
};

} // namespace m130::logging
