# Host-перевірки 03

2026-10-29. Результат: **18/18 CTest pass**, включно з усіма 8 suites 01/02; **17/17 Python tooling tests pass**. Це синтетичні перевірки, фізичних результатів немає.

## Команди та середовище

З кореня репозиторію:

```sh
python3 scripts/build.py host --build-dir build/host-03-verified
/private/tmp/scopa01-host/cmake/cmake-3.30.2-macos-universal/CMake.app/Contents/bin/cmake --build build/host-03-verified
/private/tmp/scopa01-host/cmake/cmake-3.30.2-macos-universal/CMake.app/Contents/bin/ctest --test-dir build/host-03-verified --output-on-failure
python3 -m unittest discover -s tests/tooling -v
```

Перший виклик створив новий directory; наступний — інкрементальна перевірка після фінальних виправлень. Apple clang21.0.0, CMake3.30.2, Ninja1.12.1 із environment.lock.json. Tooling запускався системним Python3; target та pinned host-tool gate перевіряє Python3.11.7. Версії конкретного запуску — environment.md / software-evidence.json. Переносимі sources та probes збираються strict C++11, platform policy C++17, без ESP32/FreeRTOS headers. Fake clock задає час, wall-clock sleeps не використовуються.

Логи: host-build.txt, host-rebuild.txt, ctest.txt, tooling.txt, negative-probe.txt. Negative probe навмисно повертає 1, підтверджуючи працездатність assertions; він не зареєстрований як позитивний CTest.

## Покриття

| Suites | Перевірена поведінка |
|---|---|
| startup_tests, core_header_probe | Startup/retry/LOW 01 і portable header |
| control_header_probe, control_mapping_tests, control_validation_tests, control_fault_tests, control_state_tests, control_time_tests | Незмінні карта, округлення, invalid-before-clamp, latch/zero/час 02 |
| rc_capture_header_probe | Capture/trace headers без SDK за C++11 |
| rc_capture_validation_tests | width899/900/2100/2101; period17999/18000/22000/22001; first unknown;19→21мс; invalid origin D49; одночасні width/period failures не маскуються |
| rc_capture_association_tests | Незалежні RMT width / estimated edges; levels/terminator; epoch, sanity50±1; відкладена обробка |
| rc_capture_core_tests | Шість map points, rounding, last-valid=end тільки valid, invalid-to-high regression, disarmed0 |
| rc_capture_fault_tests | Saturation48, incomplete HIGH, malformed runs, edge-buffer loss, ingress drop count/saturation, valid під latch, missing terminator/future metadata |
| rc_capture_time_tests | HIGH/LOW100мс±1, delivery110мс±1, episode105мс→109мс; guard12мс; processing10мс включно з кінцем core update; rearm1мс;64-bit time/sequence wrap/regression з фактичним core latch, startup HIGH Incomplete/WaitingLow та first/last fault times |
| rc_capture_adapter_tests | Fake lifecycle: помилка кожного init кроку/cleanup/retry100мс/idempotence; owner interleavings, old barrier, masking gap, sequence gap, poll перед queued edge |
| rc_capture_trace_tests |128 records/freeze/no overwrite, unknown збережено, faults/drops між candidates не приховані, max timing у header, compile-time RAM bounds |
| rc_diagnostics_policy_tests |100мс spacing, version-specific ack/concurrent publication, overwrite/busy/zero-partial drops різних одиниць, attempts/bytes, saturation |
| rc_diagnostics_transport_tests | Bounded formatting/marker/newline, великі поля,384B padding, маленький buffer відхиляється; busy admission |
| Python tooling | Profile/CLI failure gates, bench defaults routing/host rejection, effective configs/actual graph, missing capture object/IRAM config, trace object/config match та core flags; frozen archive count/order/partial cycles/mixed epoch/failed collector, direct E/U, per-interval10.001us failure |

Fake boundary tests не виконують ESP-IDF driver або реальний USB. One-in-flight підтверджено review викликів wait_tx_done(0)→одне write(...,0) та portable slot/write accounting; реальні backpressure, reconnect і partial transmission лишаються T047. Host не доводить IRAM runtime timing, точність5/10мкс, GPIO0 waveform чи нуль втрат600с.
