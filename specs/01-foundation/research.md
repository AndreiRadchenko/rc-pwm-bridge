# Дослідження — 01-foundation

Дата перевірки джерел: 2026-10-28. Це Phase 0 Plan, не результат збірки/вимірювань.
Вхід: [spec.md](spec.md), D09,D11–D13, конституція v1.0.0. Джерела нижче первинні;
рішення щодо архітектури/retry є вибором цього Plan, а не твердженнями виробника.

## RCH-01 — SDK і цільовий toolchain

**Рішення**: ESP-IDF **v5.5.6**, commit
`880a203ae603d08a94d06aa01e0d6761958e5e9e`; target лише `esp32c3`.
Клон SDK має використовувати цей commit і відповідні зафіксовані submodules,
а не `stable`, `master` чи плаваючу release-гілку.

**Підстава**: [офіційний реліз](https://github.com/espressif/esp-idf/releases/tag/v5.5.6)
визначає v5.5.6 як bugfix для 5.5; є
[точний commit](https://github.com/espressif/esp-idf/commit/880a203ae603d08a94d06aa01e0d6761958e5e9e)
і [версійна C3 документація](https://docs.espressif.com/projects/esp-idf/en/v5.5.6/esp32c3/get-started/index.html).

**Альтернативи**: плаваючий stable не відтворюваний; перехід на іншу major-лінію не
потрібний для одного C3. Вибір 5.5.6 не є твердженням, що це найновіша SDK усіх гілок.

Pins із [tools/tools.json цього tag](https://raw.githubusercontent.com/espressif/esp-idf/v5.5.6/tools/tools.json):

| Інструмент | Вибрана версія | SHA-256 архіву для обраного macOS-arm64 host |
|---|---|---|
| riscv32-esp-elf GCC | esp-14.2.0_20260121 | caa7103e014c61939eea4656cd4effdc34930da0b76cd59ad1adb7bfaeef923c |
| CMake | 3.30.2 | c6fdda745f9ce69bca048e91955c7d043ba905d6388a62e0ff52b681ac17183c |
| Ninja | 1.12.1 | 89a287444b5b3e98f88a945afa50ce937b8ffd1dcc59c555ad9b1baf855298c9 |

Це hashes офіційного manifest, не результат локального скачування/перевірки архівів.
Інші tools, якщо потрібні SDK bootstrap, беруться лише з manifest pinned commit;
їхні реально встановлені версії й архіви також входять у environment-lock.

## RCH-02 — Python, host та межі відтворюваності

**Рішення**: CPython **3.11.7**; переносима частина — strict **C++11**,
app/platform adapters — **C++17**. Host baseline цього етапу — macOS arm64 із
Apple clang **21.0.0**, build **clang-2100.1.1.101**, CLTools
**26.6.0.0.1781586589**, macOS SDK **26.5**. Версії перевіряються до команд збірки.
Не покладатися на вищий default C++ стандарт SDK.

Read-only inventory саме цього середовища підтвердив наведені Python/Clang/CLTools/SDK;
CMake, Ninja, idf.py і riscv32-esp-elf-g++ у PATH не знайдено. SHA-256 наявного
`/usr/bin/clang`: `f30550eab15fdf5ab8c0dc54c52679711241e5d4b636b027e18c09fef531775d`.
Це inventory, не host-тест і не доказ працездатного SDK environment.

[SDK Python checker](https://raw.githubusercontent.com/espressif/esp-idf/v5.5.6/tools/python_version_checker.py)
вимагає Python не нижче 3.9; 3.11.7 відповідає цій умові. [Clang C++ status](https://clang.llvm.org/cxx_status.html)
підтверджує підтримку C++11. Переносимість на ще не обрану Nano не підтверджено:
обмеження стандарту зменшує залежності, але не доводить timer/ABI/resource compatibility.

**Альтернативи**: сторонній test framework або container додають bootstrap-залежності,
які не потрібні для мінімального supervisor. Інший host compiler/OS можливий лише
після окремого environment pin і перевірки, не як заявлена підтримка зараз.

### E01 — обов'язкове замикання Python залежностей

[requirements.core.txt](https://raw.githubusercontent.com/espressif/esp-idf/v5.5.6/tools/requirements/requirements.core.txt)
та [офіційні constraints v5.5](https://dl.espressif.com/dl/esp-idf/espidf.constraints.v5.5.txt)
містять діапазони; самі вони не є повним immutable lock. Тому до відтворюваної
цільової збірки потрібний окремий bootstrap-крок:

1. Зберегти точні bytes requirements/constraints і SHA-256, SDK commit/submodule list
   та tools manifest. SDK submodules мають відповідати pinned commit.
2. На CPython 3.11.7 / macOS arm64 у чистому staging venv розв'язати повний набір
   bootstrap/runtime Python пакетів, включно з pip/build tooling.
3. Зберегти exact versions, distribution hashes і придатний wheelhouse для всіх
   транзитивних залежностей; згенерувати майбутні
   `tools/environment/requirements.lock.txt`, `tools/environment/wheels.sha256`
   та `tools/environment/environment.lock.json`.
4. Відтворити другий чистий venv із wheelhouse через `--no-index --require-hashes`,
   перевірити SDK dependencies. Зберегти результат і фактичні version outputs.

**E01 зараз blocked/not performed**: не було install/resolution/download wheelhouse.
Готового lock, його hash або успішної rebuild-перевірки немає. Технічні версії SDK,
compiler і стандарту обрані; Q01 не оголошується повністю закритим до E01.
Підготовка bootstrap/lock tooling для замикання E01 дозволена до закриття самого
gate; залежна target-реалізація/збірка — після нього. Незалежна host-частина має
власні перевірені pins і не потребує target Python closure.
Самого `pip freeze` без distribution hashes недостатньо. `dependencies.lock` component
manager не видається за SDK/Python lock і не створюється штучно за відсутності managed components.

## RCH-03 — доступність потрібних периферій

**Рішення**: зберегти RMT RX для 03, LEDC для 04. У 01 використовується лише
статичний PWM GPIO LOW; DIR постійно з’єднується з GND на MD20A (D20); драйвери RMT/LEDC не ініціалізуються.

[Версійний RMT C3](https://docs.espressif.com/projects/esp-idf/en/v5.5.6/esp32c3/api-reference/peripherals/rmt.html)
підтверджує RX та `rmt_receive`. [Офіційний LEDC source цієї версії](https://raw.githubusercontent.com/espressif/esp-idf/v5.5.6/docs/en/api-reference/peripherals/ledc.rst)
підтверджує low-speed mode для C3 й залежність частоти від resolution/source clock.
C3 має потрібну периферію для погоджених 1000 Гц; це не вимірювання PWM і не
затвердження конкретної resolution. Для максимуму resolution є caveat переповнення
при duty `2^resolution`; steady LOW/HIGH у 04 приймаються окремо, не blind max-count.

**Альтернативи**: Arduino framework суперечить v1; MCPWM з WROOM-прикладу не
переноситься на C3; програмний PWM/capture не потрібні.

## RCH-04 — GPIO, профіль і фізичні gates

**Рішення**: жодних pin defaults. Профіль прив'язується до перевіреного варіанта за
D13; unknown factory metadata не підміняє електричних доказів. Подальше пряме
рішення D18 прийняло наданий pinout як достатнє джерело контактів та зовнішні
3,3 В на 3V3/GND. D19/D20 визначили RC=GPIO3, PWM=GPIO0 і перемичку DIR–GND на MD20A без GPIO1 у
[конкретному профілі](../../hardware/esp32-c3-supermini-profile.md).

[GPIO v5.5.6](https://docs.espressif.com/projects/esp-idf/en/v5.5.6/esp32c3/api-reference/peripherals/gpio.html)
попереджає, що startup/bootloader можуть змінити IO до app_main; GPIO reset не є
гарантією LOW. Обмеження strap/flash/USB враховуються разом із перевіркою конкретної
розводки. [Datasheet v2.4](https://documentation.espressif.com/esp32-c3_datasheet_en.html)
містить startup-glitch та boot restrictions. Для програмного дизайну за D18
не вимагається окрема схема/continuity; вимірювання startup залишаються окремими.

**Альтернативи**: призначити «типові SuperMini pins» або затвердити flash із назви
не відповідає FR-004/FR-005. Очікування офіційного vendor revision не є єдиним
шляхом після D13: допускається власний ID із належними доказами.

H01/H02/H03 і межі Q03/Q06 описано в [контракті профілю](contracts/board-profile.md).
H01 design визначено D17–D19. Для H02/H03 бракує фізичних доказів; це блокує
лише відповідні стендові дії/прийняття, не програмну підготовку/компіляцію профілю.
Рішення D21 (guided checklist CHK007, A) обмежило фізичне SC-004 у 01 профілем
SuperMini; RC/MD20A у 01 мають документальні вимоги й gates, фактичні дані —
у 03/04/06. Це не знімає H02/H03 SuperMini й не доводить придатність драйвера/насоса.

## RCH-05 — startup та bounded retries

**Рішення**: 5 спроб загалом; 1 000 000 мкс від завершення кожної невдалої спроби;
після успіху ReadyDisabled, після п'ятої невдачі Exhausted без application auto reset.
Це допустимий технічний вибір у Plan за FR-008/D12, не приховане рішення Q05.

**Підстава**: скінченний ліміт робить fault history і host-тести визначеними;
1 с не створює busy retry loop. Константи є design parameters, не виміряними
затримками і не доказом фізичного cutoff. Подальшу зміну записувати як зміну контракту.
Подальший вибір A у guided Analyze (C1, D23) уточнив: poll 10 мс номінальний,
а 1 с — мінімальна пауза. Верхньої межі фактичного target lateness у 01 немає;
окремий target-timing режим не додається. Перевіряються host deadline/late-tick
логіка та nominal config/source, без заяви про виміряний target jitter або Q05 cutoff.

**Альтернативи**: reset-only recovery суперечить обраному B; нескінченні retries або
автоматичний reboot без обмежень не потрібні для 01. Точні переходи/error ordering:
[startup.md](contracts/startup.md). Після будь-якої помилки дозвіл роботи false;
за помилки GPIO API фізичний LOW не вважається доведеним.

## RCH-06 — локальна діагностика без впливу на startup

**Рішення**: USB Serial/JTAG console після перевірки board routing, без UART fallback
за замовчуванням. Snapshot у статичному mailbox, окремий consumer; SDK application
logs не виконуються в init/retry path. Логіка не приймає команд керування з консолі.

[USB console v5.5.6](https://docs.espressif.com/projects/esp-idf/en/v5.5.6/esp32c3/api-guides/usb-serial-jtag-console.html#data-buffering)
описує можливе очікування 50 мс при заповненому буфері без читання host; тому
звичайний stdout/ESP_LOG не можна назвати неблокувальним startup-path transport.
[Версійний USB driver header](https://raw.githubusercontent.com/espressif/esp-idf/v5.5.6/components/esp_driver_usb_serial_jtag/include/driver/usb_serial_jtag.h)
визначає timeout для write_bytes. Consumer обирає timeout 0 і drop при неповній
передачі, без flush/wait; TX/RX buffers 512/64 байти є design parameters.
[Kconfig.level](https://raw.githubusercontent.com/espressif/esp-idf/v5.5.6/components/log/Kconfig.level)
підтверджує `CONFIG_LOG_DEFAULT_LEVEL_NONE` та `CONFIG_LOG_MAXIMUM_EQUALS_DEFAULT`.
[esp_system/Kconfig](https://raw.githubusercontent.com/espressif/esp-idf/v5.5.6/components/esp_system/Kconfig)
підтверджує `CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG` та secondary none.

**Альтернатива**: синхронні повідомлення всередині retry змінюють його часові умови.
Подальше рішення D22 (CHK016, B) обрало лише нові події одноразово: consumer poll
100 мс не означає heartbeat. Zero/partial write відкидає спробу, sequence більше
не повторюється; USB reconnect не спричиняє replay. Поточний стан після втрати
може не бути видимим до нової події чи окремо дозволеного reset. Альтернативу
періодичної передачі останнього snapshot власник не обрав.
Bootloader/ROM та panic/watchdog messages — окрема startup-відповідальність;
їхні GPIO/стани не доводяться вимкненням application logs.

## RCH-07 — вимкнені radios і перевірка конфігурації

**Рішення**: перед підключенням project.cmake обмежити `COMPONENTS` до `main`,
далі явні REQUIRES/PRIV_REQUIRES. Не додавати esp_wifi, bt, esp_netif або lwip.
Без runtime Wi-Fi/BT/network init, без NVS-сервісу й endpoint storage.

[Build system v5.5.6](https://docs.espressif.com/projects/esp-idf/en/v5.5.6/esp32c3/api-guides/build-system.html)
підтверджує dependency trimming. [Wi-Fi Kconfig](https://raw.githubusercontent.com/espressif/esp-idf/v5.5.6/components/esp_wifi/Kconfig)
має invisible `ESP_WIFI_ENABLED` з default y; простий `CONFIG_ESP_WIFI_ENABLED=n`
не видається за дієвий switch. Перевірка має охопити реальні sdkconfig, component graph,
linker map і startup call graph; не тільки запропонований defaults файл.

**Альтернативи**: залишити автоматичне включення всіх компонентів і покладатися
на один Kconfig рядок недостатньо. MINIMAL_BUILD є альтернативним механізмом,
але цей Plan обирає явний `COMPONENTS main` для однозначного review.

## RCH-08 — тести та закриття досліджень

**Рішення**: standalone host executable без стороннього framework, explicit checks
із ненульовим exit при помилці та CTest. Основний suite має проходити; окремий
negative probe викликається напряму й повинен повертати ненульовий код. Його fail
не підміняє успіх suite й не приховується як нібито firmware failure.

Визначено SDK/мови/toolchain, dependency-lock protocol, retry states, component graph,
console isolation і схему профілю. Flash mode/frequency/partition layout ще не
вибрані: Q01/Q02, ітерація 01, до залежного target configure. Вибір має спиратися
на pinned SDK та апаратні джерела, записувати значення/підставу в профілі й конфігурації;
невідомі параметри не замінюються SDK defaults. Вибір не є фізичним boot acceptance.
Відкриті E01 та H02–H05 — явні execution/evidence gates, а не відповіді, вигадані
під час research. Джерела й локальний inventory не є host/build/physical acceptance.
