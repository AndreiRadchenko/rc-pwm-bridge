#include "bridge/pump_output.hpp"
static_assert(sizeof(bridge::OutputEvidence)+sizeof(bridge::CaptureWindowEvidence)<=3200,"stress collectors");
static_assert(sizeof(bridge::OutputEvidence)+6400<=9088,"accuracy collectors");
int main() { bridge::OutputSnapshot snapshot={};return snapshot.bench_permission?1:0; }
