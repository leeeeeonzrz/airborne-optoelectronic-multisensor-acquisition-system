#include "acquisition/async_recorder.hpp"
#include "acquisition/synchronizer.hpp"
#include <iostream>

int main(int argc, char** argv) {
    using namespace acquisition;
    try {
        SnapshotAssembler assembler({{"camera", "imu", "position", "height"},
                                      30'000'000, 32});
        AsyncRecorder recorder(argc > 1 ? argv[1] : "synthetic_session.csv", 16);
        for (std::uint64_t frame = 0; frame < 20; ++frame) {
            const auto time = static_cast<TimeNs>(frame) * 100'000'000;
            for (const auto& name : {"camera", "imu", "position", "height"}) {
                assembler.ingest({name, frame, time, time+2'000'000, true,
                                  {static_cast<double>(frame)}});
            }
            recorder.submit(assembler.assemble(time, frame));
        }
        recorder.stop();
        std::cout << "synthetic records only; dropped=" << recorder.dropped() << '\n';
        return recorder.failed() ? 1 : 0;
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
