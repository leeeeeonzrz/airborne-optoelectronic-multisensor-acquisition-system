#pragma once
#include <opencv2/videoio.hpp>
#include <opencv2/core.hpp>
#include <mutex>
#include <cstdint>
#include <utility>
#include <string>

namespace acquisition {
// Call read_once from a dedicated worker; do not put blocking reads in a GUI slot.
class LatestVideo {
public:
    bool open(const std::string& uri) { return capture_.open(uri); }
    bool read_once() {
        cv::Mat frame;
        if (!capture_.read(frame) || frame.empty()) return false;
        std::lock_guard<std::mutex> lock(mutex_);
        latest_ = frame.clone(); ++sequence_;
        return true;
    }
    std::pair<std::uint64_t, cv::Mat> snapshot() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return {sequence_, latest_.clone()};
    }
private:
    cv::VideoCapture capture_;
    mutable std::mutex mutex_;
    cv::Mat latest_;
    std::uint64_t sequence_{0};
};
}
