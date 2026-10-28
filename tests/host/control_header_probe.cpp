#include "bridge/control.hpp"
int main() { bridge::ControlCore core; return core.snapshot().work_permitted ? 1 : 0; }
