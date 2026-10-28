#ifndef TEST_SUPPORT_HPP
#define TEST_SUPPORT_HPP
#include <cstdio>
struct Checks { int failures = 0; void check(bool ok, const char* expr, int line) { if (!ok) { ++failures; std::fprintf(stderr, "line %d: %s\n", line, expr); } } };
#define CHECK(c, expr) (c).check((expr), #expr, __LINE__)
#endif
