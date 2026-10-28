# firmware

Нативний ESP-IDF каркас ітерації 01: переносиме C++11 ядро та адаптери ESP32-C3. Профіль `boards/c3-supermini-pinout-01.json`: RC GPIO3 зарезервовано, PWM GPIO0 статично LOW, DIR апаратно з’єднується з GND на MD20A; firmware DIR не керує. Дозвіл роботи завжди false. Немає RC capture, PWM генерації, arming або мережевих сервісів.

Чиста target-збірка та окремий config review описані у [build.md](../specs/01-foundation/verification/build.md) і [config-review.md](../specs/01-foundation/verification/config-review.md); фізичні H02/H03 і прийняття 01 відкриті. LOW MD20A означає brake, а не відключення живлення. Прошивання й робота з навантаженням потребують окремого явного завдання.

Команди й середовище: [quickstart](../specs/01-foundation/quickstart.md).
