#pragma once
#include "types.hpp"
#include <algorithm>
#include <cmath>
#include <deque>
#include <mutex>
#include <stdexcept>
#include <unordered_map>

namespace acquisition {
class SnapshotAssembler {
public:
    explicit SnapshotAssembler(SyncPolicy policy) : policy_(std::move(policy)) {
        if (!policy_.history_limit || policy_.tolerance_ns < 0 || policy_.streams.empty())
            throw std::invalid_argument("invalid synchronization policy");
    }
    void ingest(Sample sample) {
        if (sample.sample_ns < 0 || sample.arrival_ns < 0)
            throw std::invalid_argument("timestamps must be nonnegative");
        if (std::find(policy_.streams.begin(), policy_.streams.end(), sample.stream)
            == policy_.streams.end()) return;
        std::lock_guard<std::mutex> lock(mutex_);
        auto& history = histories_[sample.stream];
        auto pos = std::upper_bound(history.begin(), history.end(), sample.sample_ns,
            [](TimeNs time, const Sample& s) { return time < s.sample_ns; });
        history.insert(pos, std::move(sample));
        while (history.size() > policy_.history_limit) history.pop_front();
    }
    Snapshot assemble(TimeNs reference_ns, std::uint64_t sequence) const {
        if (reference_ns < 0) throw std::invalid_argument("negative reference time");
        std::lock_guard<std::mutex> lock(mutex_);
        Snapshot result{sequence, reference_ns, {}, {}};
        for (const auto& name : policy_.streams) {
            auto found = histories_.find(name);
            const Sample* best = nullptr;
            long double distance = static_cast<long double>(policy_.tolerance_ns) + 1;
            if (found != histories_.end()) {
                for (const auto& sample : found->second) {
                    if (!sample.valid) continue;
                    auto delta = std::abs(static_cast<long double>(sample.sample_ns)
                                          - reference_ns);
                    if (delta < distance) { best = &sample; distance = delta; }
                }
            }
            if (best && distance <= policy_.tolerance_ns)
                result.observations.emplace(name, *best);
            else result.missing.push_back(name);
        }
        return result;
    }
private:
    SyncPolicy policy_;
    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::deque<Sample>> histories_;
};
}  // namespace acquisition
