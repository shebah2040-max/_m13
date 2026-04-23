#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

// Minimal assertion macros so core tests don't depend on Qt Test or GoogleTest.
// Exit code 0 on success; 1 on first failure.

namespace m130::testing {

inline int g_failures = 0;

inline void fail(const char* file, int line, const char* what)
{
    std::fprintf(stderr, "FAIL %s:%d  %s\n", file, line, what);
    ++g_failures;
}

} // namespace m130::testing

#define M130_REQUIRE(cond)                                                         \
    do {                                                                           \
        if (!(cond)) {                                                             \
            ::m130::testing::fail(__FILE__, __LINE__, "REQUIRE(" #cond ")");       \
            return 1;                                                              \
        }                                                                          \
    } while (0)

#define M130_EXPECT(cond)                                                          \
    do {                                                                           \
        if (!(cond)) {                                                             \
            ::m130::testing::fail(__FILE__, __LINE__, "EXPECT(" #cond ")");        \
        }                                                                          \
    } while (0)

#define M130_REQUIRE_EQ(a, b)                                                      \
    do {                                                                           \
        auto _av = (a); auto _bv = (b);                                            \
        if (!(_av == _bv)) {                                                       \
            std::fprintf(stderr, "FAIL %s:%d  REQUIRE_EQ(" #a ", " #b ")\n",       \
                         __FILE__, __LINE__);                                      \
            ++::m130::testing::g_failures;                                         \
            return 1;                                                              \
        }                                                                          \
    } while (0)

#define M130_RUN(fn)                                                               \
    do {                                                                           \
        int _rc = (fn)();                                                          \
        if (_rc != 0) return _rc;                                                  \
    } while (0)

#define M130_TEST_MAIN()                                                           \
    int main() { int rc = run(); return rc != 0 ? rc : (::m130::testing::g_failures ? 2 : 0); }
