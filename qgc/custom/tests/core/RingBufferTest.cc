#include "analysis/RingBuffer.h"
#include "test_support.h"

using m130::analysis::RingBuffer;

static int testCapacityAndInvariants()
{
    RingBuffer<int> rb(4);
    M130_REQUIRE_EQ(rb.capacity(), std::size_t{4});
    M130_REQUIRE_EQ(rb.size(), std::size_t{0});
    M130_REQUIRE(rb.empty());
    M130_REQUIRE(!rb.full());
    return 0;
}

static int testFillThenOverwrite()
{
    RingBuffer<int> rb(3);
    rb.push(1);
    rb.push(2);
    rb.push(3);
    M130_REQUIRE(rb.full());
    M130_REQUIRE_EQ(rb.at(0), 1);
    M130_REQUIRE_EQ(rb.at(2), 3);
    M130_REQUIRE_EQ(rb.oldest(), 1);
    M130_REQUIRE_EQ(rb.newest(), 3);

    rb.push(4);  // evict 1
    M130_REQUIRE_EQ(rb.size(), std::size_t{3});
    M130_REQUIRE_EQ(rb.at(0), 2);
    M130_REQUIRE_EQ(rb.at(2), 4);
    M130_REQUIRE_EQ(rb.newest(), 4);

    rb.push(5);
    rb.push(6);
    M130_REQUIRE_EQ(rb.at(0), 4);
    M130_REQUIRE_EQ(rb.at(2), 6);
    return 0;
}

static int testSnapshotChronological()
{
    RingBuffer<int> rb(4);
    for (int i = 0; i < 7; ++i) {
        rb.push(i);
    }
    auto snap = rb.snapshot();
    M130_REQUIRE_EQ(snap.size(), std::size_t{4});
    M130_REQUIRE_EQ(snap[0], 3);
    M130_REQUIRE_EQ(snap[1], 4);
    M130_REQUIRE_EQ(snap[2], 5);
    M130_REQUIRE_EQ(snap[3], 6);
    return 0;
}

static int testClear()
{
    RingBuffer<int> rb(2);
    rb.push(1);
    rb.push(2);
    rb.clear();
    M130_REQUIRE(rb.empty());
    rb.push(9);
    M130_REQUIRE_EQ(rb.newest(), 9);
    return 0;
}

static int testOutOfRangeThrows()
{
    RingBuffer<int> rb(2);
    bool threw = false;
    try {
        (void)rb.at(0);
    } catch (const std::out_of_range&) {
        threw = true;
    }
    M130_REQUIRE(threw);
    return 0;
}

static int run()
{
    M130_RUN(testCapacityAndInvariants);
    M130_RUN(testFillThenOverwrite);
    M130_RUN(testSnapshotChronological);
    M130_RUN(testClear);
    M130_RUN(testOutOfRangeThrows);
    return 0;
}

M130_TEST_MAIN()
