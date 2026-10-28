#include "test_support.hpp"
#include "bridge/startup.hpp"
int main() { Checks c; bridge::StartupSupervisor s; CHECK(c, s.snapshot().work_permitted); return c.failures ? 1 : 0; }
