#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <vector>

namespace acquisition {
// Demonstration protocol only: 0xA5 0x5A | u16 big-endian length | payload.
// It is intentionally unrelated to any device-specific frame format.
class FrameDecoder {
public:
    explicit FrameDecoder(std::size_t maximum_payload = 4096) : maximum_(maximum_payload) {
        if (maximum_ > 65535) throw std::invalid_argument("u16 length limit");
    }
    void feed(const std::uint8_t* bytes, std::size_t count,
              const std::function<void(std::vector<std::uint8_t>)>& on_frame) {
        for (std::size_t i = 0; i < count; ++i) {
            buffer_.push_back(bytes[i]);
            parse(on_frame);
        }
    }
private:
    void parse(const std::function<void(std::vector<std::uint8_t>)>& on_frame) {
        while (buffer_.size() >= 2) {
            if (buffer_[0] != 0xA5 || buffer_[1] != 0x5A) {
                buffer_.erase(buffer_.begin()); continue;
            }
            if (buffer_.size() < 4) return;
            const auto length = (std::size_t(buffer_[2]) << 8) | buffer_[3];
            if (length > maximum_) { buffer_.erase(buffer_.begin()); continue; }
            if (buffer_.size() < length + 4) return;
            std::vector<std::uint8_t> payload(buffer_.begin()+4, buffer_.begin()+4+length);
            buffer_.erase(buffer_.begin(), buffer_.begin()+4+length);
            on_frame(std::move(payload));
        }
    }
    std::size_t maximum_;
    std::vector<std::uint8_t> buffer_;
};
}  // namespace acquisition
