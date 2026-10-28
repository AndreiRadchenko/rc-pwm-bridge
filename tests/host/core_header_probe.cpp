#include "bridge/startup.hpp"
int main() { bridge::StartupSupervisor s; return s.snapshot().work_permitted ? 1 : 0; }
