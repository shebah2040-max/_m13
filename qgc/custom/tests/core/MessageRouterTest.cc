#include "protocol/MessageRouter.h"
#include "test_support.h"

using namespace m130::protocol;

int dispatchesToRegisteredHandler()
{
    MessageRouter r;
    int calls = 0;
    r.on(42001, [&](uint32_t id, const void*, std::size_t) {
        if (id == 42001) ++calls;
    });
    M130_REQUIRE_EQ(r.dispatch(42001, nullptr, 0), std::size_t(1));
    M130_REQUIRE_EQ(calls, 1);
    return 0;
}

int teeObservesAllMessages()
{
    MessageRouter r;
    int tee_calls = 0;
    r.setTee([&](uint32_t, const void*, std::size_t) { ++tee_calls; });
    r.on(42001, [](uint32_t, const void*, std::size_t){});
    r.dispatch(42001, nullptr, 0);
    r.dispatch(42999, nullptr, 0); // no handler
    M130_REQUIRE_EQ(tee_calls, 2);
    M130_REQUIRE_EQ(r.totalUnhandled(), std::uint64_t(1));
    return 0;
}

int multipleHandlers()
{
    MessageRouter r;
    int a = 0, b = 0;
    r.on(10, [&](uint32_t, const void*, std::size_t) { ++a; });
    r.on(10, [&](uint32_t, const void*, std::size_t) { ++b; });
    M130_REQUIRE_EQ(r.dispatch(10, nullptr, 0), std::size_t(2));
    M130_REQUIRE_EQ(a, 1);
    M130_REQUIRE_EQ(b, 1);
    return 0;
}

int offRemovesHandlers()
{
    MessageRouter r;
    int a = 0;
    r.on(10, [&](uint32_t, const void*, std::size_t) { ++a; });
    r.off(10);
    M130_REQUIRE_EQ(r.dispatch(10, nullptr, 0), std::size_t(0));
    M130_REQUIRE_EQ(a, 0);
    return 0;
}

int run()
{
    M130_RUN(dispatchesToRegisteredHandler);
    M130_RUN(teeObservesAllMessages);
    M130_RUN(multipleHandlers);
    M130_RUN(offRemovesHandlers);
    return 0;
}

M130_TEST_MAIN()
