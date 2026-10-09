#pragma once
#include "bounded_queue.hpp"
#include "types.hpp"
#include <atomic>
#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <thread>

namespace acquisition {
// Header-only text recorder. Images belong in a separate binary payload store.
class AsyncRecorder {
public:
    AsyncRecorder(const std::string& path, std::size_t capacity)
        : queue_(capacity), file_(path) {
        if (!file_) throw std::runtime_error("cannot open output");
        file_ << "snapshot,reference_ns,stream,sample_ns,arrival_ns,sample_sequence\n";
        worker_ = std::thread([this] { consume(); });
    }
    ~AsyncRecorder() { stop(); }
    AsyncRecorder(const AsyncRecorder&) = delete;
    AsyncRecorder& operator=(const AsyncRecorder&) = delete;
    bool submit(Snapshot snapshot) {
        if (!queue_.try_push(std::move(snapshot))) { ++dropped_; return false; }
        return true;
    }
    void stop() {
        queue_.close();
        if (worker_.joinable()) worker_.join();
    }
    std::size_t dropped() const { return dropped_.load(); }
    bool failed() const { return failed_.load(); }
private:
    void consume() {
        while (auto snapshot = queue_.pop()) {
            for (const auto& entry : snapshot->observations) {
                const auto& sample = entry.second;
                file_ << snapshot->sequence << ',' << snapshot->reference_ns << ','
                      << std::quoted(entry.first) << ',' << sample.sample_ns << ','
                      << sample.arrival_ns << ',' << sample.sequence << '\n';
            }
            if (!file_) failed_ = true;
        }
        file_.flush();
        if (!file_) failed_ = true;
    }
    BoundedQueue<Snapshot> queue_;
    std::ofstream file_;
    std::thread worker_;
    std::atomic<std::size_t> dropped_{0};
    std::atomic<bool> failed_{false};
};
}  // namespace acquisition
