#include "acquisition/bounded_queue.hpp"
#include "acquisition/frame_decoder.hpp"
#include "acquisition/synchronizer.hpp"
#include <cassert>

int main() {
    using namespace acquisition;
    SnapshotAssembler sync({{"imu", "height"}, 5, 8});
    sync.ingest({"imu", 1, 100, 110, true, {}});
    sync.ingest({"height", 1, 120, 125, true, {}});
    assert(!sync.assemble(100, 1).complete()); // Stale/future data cannot masquerade as synced.
    sync.ingest({"height", 2, 102, 112, true, {}});
    assert(sync.assemble(100, 2).complete());
    BoundedQueue<int> queue(1);
    assert(queue.try_push(7)); assert(!queue.try_push(8));
    queue.close(); assert(queue.pop().value() == 7); assert(!queue.pop());
    FrameDecoder decoder;
    int received=0;
    const std::uint8_t part1[]{0x00,0xA5};
    const std::uint8_t part2[]{0x5A,0x00,0x02,0x11,0x22};
    auto accept=[&](std::vector<std::uint8_t> p){ assert(p.size()==2 && p[1]==0x22); ++received; };
    decoder.feed(part1,2,accept); decoder.feed(part2,5,accept);
    assert(received==1);
}
