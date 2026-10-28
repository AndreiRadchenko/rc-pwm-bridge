# Target build 02

2026-10-28. T023 environment checker exit 0. `python3 scripts/build.py target --board-profile firmware/boards/c3-supermini-pinout-01.json --build-dir build/c3-02-final` — exit 0. ESP-IDF v5.5.6 commit `880a203ae603d08a94d06aa01e0d6761958e5e9e`, riscv32 esp-14.2.0_20260121, CMake 3.30.2, Ninja 1.12.1; manifest `tools/environment/environment.lock.json`. `python3 scripts/verify/config.py --build-dir build/c3-02-final` — exit 0; автоматичні config/graph/map/source checks пройдені, повідомлено потребу ручного огляду startup/watchdog/bootloader.

`compile_commands.json` підтверджує компіляцію `control.cpp` у компоненті з останнім стандартом `-std=c++11`, `-fno-exceptions`, `-fno-rtti`; `libbridge_core.a` створено. Поточний `app_main.cpp` не викликає ControlCore; symbols control не знайдено у фінальній map. Збірка підтверджує компіляцію компонента, не інтеграцію керування у виконуваний застосунок. Це відповідає межам 02 і не є фізичною перевіркою.

SHA-256:

```text
d04d2c23a525c8fc4f8d0c2f41b2cf4fb79696452fac0389eadaa90d90bf45d6  firmware/components/bridge_core/control.cpp
fc74bbf42caf8367ad4870cd2ed5ff1732bee89c4f7ccc5f37438f50396b7f0c  firmware/components/bridge_core/include/bridge/control.hpp
965f8f24e5b9163449ff1a4688b35173f109a3421e681a0727d78b54a3610513  build/c3-02-final/rc_pwm_bridge.bin
734f50b39df339b8705120a2f500b4e967b5cb19c05ff4ce2eadd42c7bde7d31  build/c3-02-final/esp-idf/bridge_core/libbridge_core.a
```

Плата не підключалась, flash/reset/load не виконувались. Команда `shasum` з початково помилковим шляхом `bridge.bin` повернула exit 1; правильний `rc_pwm_bridge.bin` перевірено окремо, як наведено вище.
