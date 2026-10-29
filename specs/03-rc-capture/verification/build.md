# Target builds 03

2026-10-29. **Три native ESP32-C3 builds pass**, плата не потрібна й не використовувалась. SDK ESP-IDF v5.5.6 commit `880a203ae603d08a94d06aa01e0d6761958e5e9e`, GCC14.2.0 esp-14.2.0_20260121, CPython3.11.7, CMake3.30.2, Ninja1.12.1. Нових зовнішніх залежностей немає. Pins01 збережено; environment checker пройшов.

## Команди та результати

```sh
python3 scripts/build.py target --board-profile firmware/boards/c3-supermini-pinout-01.json --build-dir build/c3-03-usb-normal
python3 scripts/build.py target --board-profile firmware/boards/c3-supermini-pinout-01.json --build-dir build/c3-03-usb-stress --rc03-bench diagnostics
python3 scripts/build.py target --board-profile firmware/boards/c3-supermini-pinout-01.json --build-dir build/c3-03-usb-trace --rc03-bench accuracy
python3 scripts/verify/config.py --build-dir build/c3-03-usb-normal
python3 scripts/verify/config.py --build-dir build/c3-03-usb-stress
python3 scripts/verify/config.py --build-dir build/c3-03-usb-trace
```

Після виправлення USB RX усі три directories створено заново через build.py; три clean target builds завершились exit0. Попередні build/c3-03-bias-* збережено. Records/hashes перед виправленням — usb-rx-before/. Поточні full logs — build-usb-normal/stress/trace.txt; config-*.txt, sections-*.txt і software-evidence.json відповідають новим usb builds. У логах є довідкові flash commands; агент їх не виконував.

USB RX capacity виправлено64→128B: pinned driver відхиляє rx_buffer_size≤64. TX лишився512B, zero-wait/latest-only/one-in-flight незмінні. D56 GPIO_FLOATING збережено й повторно перевірено в compiled ELF; gpio-bias-acquire-*.txt відповідають новим builds. Новий regression check у rc_diagnostics_transport_tests перевіряє SDK lower bound; у тимчасовій копії повернення64 змушує тест впасти з exit1 (usb-rx-regression.txt). Фізичний запуск виправленої прошивки агентом не перевірено.

| Варіант | Stress / trace | App bin B | App .dram0.bss B | Capture object B |
|---|---|---:|---:|---:|
| c3-03-usb-normal | off/off |173200|25336|16336|
| c3-03-usb-stress | on/off |173200|25336|16336|
| c3-03-usb-trace | on/on |173904|31544|22544|

У всіх: .iram0.text56438B, .dram0.data5096B. Різниця trace6208B, record48B/header64B, нижче6400B bound. Diagnostic object4752B. Capture object містить static8192B worker stack, queue storage/control, scratch192B, policy, metadata; diagnostics містить4096B stack. Main3584B, esp_timer3584B, ISR1536B — effective SDK defaults, не виміряні high-water marks. Static sections не є вимірюванням залишку heap/runtime stack.

Bin SHA-256:

- normal: `1f5ddcfc22b6f9e269db87ad43ec8a2b70ff35a4faa7ac2faf213548f45e7e57`
- stress: `31ce45ffe694b139350c1fc03fda164fbeff1202ff02f4d5f2c9f43ec1bd384e`
- trace: `c06c8fef754c4f37ca5d4e34412ecf9c56a5b3f6d34cb4b36ad5de4a3f37a4d1`

Повний source/config/graph/compile/map/ELF/bin/bootloader/partition hashes — [software-evidence.json](software-evidence.json), секції — sections-*.txt. Build artifacts лишаються у власних ignored directories; hashes не замінюють самі binaries перед flash. Старі01/02 artifacts не змінювались.

## Межі

Config/graph review — [config-review.md](config-review.md). Збірка не вимірює ISR timestamp accuracy, rearm/processing/deadline, zero loss600с, реальний USB backpressure, GPIO0 boot/reset чи H02/H03/Q03. Ці перевірки **not_run** у [hardware.md](hardware.md). IDF виводить довідкові flash commands у full build log; вони не виконувались.
