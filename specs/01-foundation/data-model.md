# Модель даних — 01-foundation

Дата: 2026-10-28. Джерела: [специфікація](spec.md), [дослідження](research.md).
Це дизайн майбутньої реалізації; [контактний профіль](../../hardware/esp32-c3-supermini-profile.md)
визначено за D18–D20, firmware JSON/config ще не створено. Окремі результати
ідентифікації після Plan наведено в [board review](verification/board-review.md).

## Профіль конкретного апаратного варіанта

`BoardProfile` належить платформі, не переносимому ядру. Його джерело — майбутній
`firmware/boards/<variant_id>.json`; JSON читається інструментом підготовки збірки,
а не firmware. Після перевірки він перетворюється на незмінний опис у build-каталозі.

| Поле | Правило |
|---|---|
| schema_version, variant_id | Версія 1; непорожній локально унікальний ID апаратного варіанта. |
| manufacturer, factory_revision | Підтверджений текст або явне `unknown` за D13; не назва, вигадана з фото. |
| chip_model, chip_revision, flash_bytes | Факти конкретного екземпляра з джерелом і статусом перевірки. |
| flash_mode, flash_frequency_hz, partition_layout | Вибрати й записати з джерелом до залежного target configure; зараз не вибрані. Розмір flash із ROM не доводить її mode/frequency. Не підставляти мовчазні defaults. |
| evidence | ROM-дані, hash pinout/фото й погодження користувача D18; окрема схема/continuity не обов'язкова для design. Вимірювання мають окремий тип доказу. |
| rc_pin, pwm_pin | За D19/D20: 3 та 0; джерело — погоджений pinout/SoC review. Фізичний результат має окремий статус. |
| direction_mode | `hardwired_low`: перемичка DIR–GND на MD20A за D20. `dir_pin`/програмного `dir_level` немає; GPIO1 не використовується, жодної DIR GPIO-операції. Фізична перевірка перемички — 04. |
| power_path, usb_route, console | Дизайн живлення 3V3/GND за D18, ROM-перевірений native USB та локальна діагностика; виміряні/невиміряні режими позначені окремо. |
| boot_conflicts, onboard_loads, bias | Розгляд strap/flash/USB/UART/JTAG, кнопок/LED та зовнішнього bias. |
| rc_interface, md20a_interface | Електричні умови й межі дозволеного підключення; відомі/відкладені частини Q03/Q06. |
| approval | Окремі design/bench statuses, джерело/дата, область дозволеної роботи та fingerprint вмісту. |

`draft`/`blocked` не дозволяють target-конфігурацію з GPIO. `design_approved`
дозволяє підготовку/компіляцію профілю D18–D20 без вимоги вже виконаних вимірювань.
`bench_ready_01_gpio_only` додатково вимагає H02/H03 й дозволяє лише окремо запитані
спостереження SuperMini з від'єднаними MD20A/насосом. Зміна критичного поля потребує
повторного design review; design approval не є physical acceptance або дозволом навантаження.
Докладний контракт: [board-profile.md](contracts/board-profile.md).
Фізичне прийняття `01_gpio_only` охоплює SuperMini за D21. У полях RC/MD20A
допустимий явний `pending` із документальним джерелом, власником і gate: 03 для
RC до підключення, 04 для ревізії/інтерфейсу/перемички драйвера, 06 для силового
тракту. Такі поля не блокують прийняття профілю в області 01 й не є прийняттям
відкладених інтерфейсів. Невиконані H02/H03 самої SuperMini залишають 01 відкритою.

## Переносимі типи startup

C++11, фіксовані enum/структури; без GPIO, ESP-IDF, FreeRTOS, heap, винятків і RTTI.
Це мінімальний startup supervisor, а не control core наступних етапів.

| Тип/поле | Семантика |
|---|---|
| MonotonicUs | `uint64_t`, мікросекунди поточного boot; час передається аргументом. |
| StartupPhase | `AwaitingAttempt`, `Attempting`, `RetryWait`, `ReadyDisabled`, `Exhausted`, `ClockFault`. |
| RetryPolicy | 5 спроб загалом; пауза 1 000 000 мкс від завершення невдалої спроби. |
| InitOutcome | Успіх лише після успіху всіх обов'язкових кроків; інакше код причини. |
| attempts_started | 0–5, зростає рівно при видачі нового запиту спроби. |
| last_now_us, next_attempt_us | Регресія часу або переповнення обчислення deadline ведуть у `ClockFault`. |
| work_permitted | Завжди `false` у всіх станах 01; немає події, яка встановлює `true`. |
| last_error | Остання помилка спроби; не підміняє поточний phase або успішність усієї сесії. |

Перехідний контракт: [startup.md](contracts/startup.md). Reset створює нову volatile
сесію з нульовим лічильником; NVS/збережених endpoints/відновлення дозволу немає.
RetryPolicy задає мінімальну паузу; номінальний platform poll 10 мс за D23 не є
полем core-політики або верхньою межею фактичного target lateness. Запізнілий tick
видає одну спробу, без catch-up і без зміни permission=false.
`Exhausted` та `ClockFault` не запускають автоматичний software reset.
Clock-порт повертає validity/error signed raw до перетворення на MonotonicUs;
помилка/від’ємне значення дає terminal ClockFault без вигаданого часу та init/GPIO/setup.
Checked conversion є спільною SDK-незалежною політикою для target leaf і fake-clock tests.

## Платформний результат і локальний snapshot

`OutputApplyResult` відокремлює успіх GPIO API від фізичного вимірювання.
`pwm_low_api_applied=true` означає лише успішне застосування API; це не доказ LOW на
контакті. Помилка assert LOW не дозволяє решту init-кроків у цій спробі.

`StartupSnapshot`: sequence, now_us, now_valid, phase, attempts_started, next_attempt_us,
work_permitted, last_error, pwm_low_api_applied, variant_id/fingerprint reference.
`now_valid=true` означає валідний clock sample поточної події. Якщо sample відсутній,
від'ємний або регресивний, now_valid=false, а now_us — останній прийнятий час
або 0 до першого валідного sample, не поточне вимірювання. Невдалу операцію не
підміняють цим значенням для init/deadline; сесія переходить у ClockFault.
До першого запиту clock початковий snapshot може мати now_valid=false без fault:
прапорець описує час, а ClockFault викликається саме невдалою/невалідною clock-операцією.
На межі транспорту — fixed-size запис не більше 96 байтів, без динамічних рядків;
ідентифікатор варіанта може передаватися як фіксований числовий fingerprint.

Producer публікує snapshot без очікування у статичний mailbox на один запис.
Новий snapshot замінює старий. За D22 новий sequence відповідає початковому стану
або зміні phase/attempt/deadline/error/output-result; зміна лише now_us не породжує
публікацію. Consumer зберігає `last_attempted_sequence`, форматує не більше 256 байтів
лише для нового sequence та робить одну спробу передачі. Маркер оновлюється навіть
після zero/partial write; старий snapshot не повторюється. Consumer працює окремо від startup. Повнота історії не
гарантується; останній стан/лічильник і причина зберігаються у snapshot. Локальний
вивід можливий за готового transport і читання host; setup failure transport є
init failure, відсутність host — ні. USB reconnect не створює нову подію й не
спричиняє replay: після втрати запису останній стан може бути невидимим до нової
події або окремо дозволеного reset. Періодичного повторення поточного стану немає. Жодного
парсера команд керування через консоль у 01 немає.

## Середовище й докази

`EnvironmentIdentity` містить SDK tag/commit/submodule state, tools manifest hash,
реальні версії compiler/Python/CMake/Ninja, hash Python-lock/wheelhouse, host CLT/SDK,
конфігурацію й source fingerprint. Для непроініціалізованої Git-історії використовують
manifest SHA-256 джерел, не вигаданий commit ID.

`VerificationRecord` має `kind` (`host`, `target_build`, `config_review`, `board_review`,
`physical`), R/FR/SC/scenario IDs, процедуру/команду, умови, очікування, фактичний
результат, attachments та `not_run`/`blocked`/`pass`/`fail`. Відсутність запису не є pass.
Методи й gates: [build-and-evidence.md](contracts/build-and-evidence.md).
