#pragma once
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace acquisition {
using TimeNs = std::int64_t;
struct Sample {
    std::string stream;
    std::uint64_t sequence{};
    TimeNs sample_ns{};  // Same clock domain across streams, never a display string.
    TimeNs arrival_ns{};
    bool valid{true};
    std::vector<double> values;
};
struct Snapshot {
    std::uint64_t sequence{};
    TimeNs reference_ns{};
    std::map<std::string, Sample> observations;
    std::vector<std::string> missing;
    bool complete() const { return missing.empty(); }
};
struct SyncPolicy {
    std::vector<std::string> streams;
    TimeNs tolerance_ns{40'000'000};
    std::size_t history_limit{32};
};
}  // namespace acquisition
