# Host-перевірки 02

2026-10-28, US1 T009: `python3 scripts/build.py host --build-dir build/host-02-us1` — exit 0; pinned CTest `--test-dir build/host-02-us1 -R '^control_mapping_tests$' --output-on-failure` — exit 0, 1/1. Шість точок, валідні endpoints, усі 100 меж округлення із сусідами та повтори. Host evidence не доводить фізичний вихід. Повна suite ще не виконана.

## Повна suite

`python3 scripts/build.py host --build-dir build/host-02-final` — exit 0; pinned CTest `--test-dir build/host-02-final --output-on-failure` — exit 0, 8/8: startup і попередній probe, control header, карта, validation, fault, state, time. Покрито 899/900/2100/2101, 0/UINT32_MAX, valid → invalid → valid, fault → valid, clear/high, рівні та регресивні часові мітки. `clang++ -std=c++11 -MM -Ifirmware/components/bridge_core/include firmware/components/bridge_core/control.cpp` — exit 0, лише control.cpp та control.hpp як проєктні залежності. Результати не є фізичним вимірюванням.
