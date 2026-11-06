# Перевірка 05: майбутній порядок виконання

Дата:2026-11-06. Це validation guide після окремо дозволеної реалізації. У Plan наведені команди не запускалися; нові прапорці/перевірки 05 ще не реалізовані. Компіляція не доводить фізичну поведінку. Доступ до плати, flash/reset і будь-які operator дії потребують окремого явного завдання. Насос у 05 фізично від'єднаний.

## Передумови

1. Перевірити чинну конституціюv2.0.0/D74 та узгоджені шаблони/настанови: G05 вже закрито окремим явним Constitution-завданням2026-11-06. Не повторювати цю поправку як незакриту передумову. Technical D69–D73 та physical gates зберігаються; реалізація й прийняття не виконані.
2. Завершити окремо доручену реалізацію. Підтвердити source/config/profile/SDK identity; відновити й перевірити саме pinned tools із tools/environment/environment.lock.json, без довільної заміни версій. Наявність локального SDK у research не є environment pass.
3. Host/build виконувати без пристрою. Для фізичного кроку окремо прийняти power/bias/levels/conditioning/MD20A revision/interface/DIR–GND, від'єднаний насос, probe loading/ground/інструмент/U та безпечний режим живлення. USB+external power не припускається прийнятим.
4. Власні results05 записувати окремо у verification/host.md,build.md,config-review.md,hardware.md,acceptance.md з командами/exit/config/hash/expected-actual/status. Old acceptance01–04 зберегти. Невиконані сценарії — not_run/blocked із причиною.

## Host, лише після реалізації

Із root репозиторію, у нові вільні каталоги; наявні build directories не видаляти:

```sh
SPECIFY_FEATURE_DIRECTORY=specs/05-failsafe-arming python3 scripts/environment/check.py --manifest tools/environment/environment.lock.json
SPECIFY_FEATURE_DIRECTORY=specs/05-failsafe-arming python3 scripts/build.py host --build-dir build/host-05-review
python3 -m unittest discover -s tests/tooling -v
```

CTest запускати pinned executable з каталогу CMake у manifest; negative_probe — окремо й очікувати ненульовий exit. Зберегти discovery count, suite output/exit та hashes. Тут не задається вигаданий майбутній count tests.

Перевірити H1–H8 з plan.md: old02/03/04 regression; six map points/rounding/validation; defaultTest/explicitFly; T499999/500000/500001 мкс, physical-bound comparator999999/1000000/1000001 мкс із правилом U; no prior-valid; input faults/old/pre-barrier/processing-late; recovery-high; output stages/3retry/100 мс/fence/exhaustion; D75: кожний reinit start витрачає спробу, reinit0-success/apply-failure не скидає count, third-success→post-fence valid0/high success закриває епізод, нова failure до такого success при count3→ExhaustedLow, незалежний епізод після закриття має бюджет3; clock negative/regression/freeze/overflow і long uptime; watchdog feed тільки checkpoint; failed init/add без armed timer→SetupBlockedLOW/nohandoff, failed feed→explicit emergency reset; payload bound≤347/384B/drop/versionack/reconnect. API success і synthetic traces не зараховувати як physical pass.

## Native target builds без плати

Після implementation wrapper має підтримувати ці команди; зараз це запланований CLI:

```sh
SPECIFY_FEATURE_DIRECTORY=specs/05-failsafe-arming python3 scripts/build.py target --board-profile firmware/boards/c3-supermini-pinout-01.json --build-dir build/c3-05-default
SPECIFY_FEATURE_DIRECTORY=specs/05-failsafe-arming python3 scripts/build.py target --operational-mode test --board-profile firmware/boards/c3-supermini-pinout-01.json --build-dir build/c3-05-test
SPECIFY_FEATURE_DIRECTORY=specs/05-failsafe-arming python3 scripts/build.py target --operational-mode fly --board-profile firmware/boards/c3-supermini-pinout-01.json --build-dir build/c3-05-fly
SPECIFY_FEATURE_DIRECTORY=specs/05-failsafe-arming python3 scripts/build.py target --legacy-disabled --board-profile firmware/boards/c3-supermini-pinout-01.json --build-dir build/c3-05-legacy
```

Default і explicitTest мають однакову operational policy Test, Fly має explicit provenance. Actual sdkconfig/manifest/compile graph/map/disassembly важливіші за stdout або назву каталогу. Перевірити native C3/no radio/no DIR/fade, strict core headers/flags, unchanged legacy constructor semantics та finite04 sequence. Зібрати наявні 03 diagnostics/accuracy,04 waveform/stress/accuracy0..5 окремо; нова ordinary збірка навмисно вже не historical normal04.

Operational evidence images після implementation обираються, наприклад:

```sh
SPECIFY_FEATURE_DIRECTORY=specs/05-failsafe-arming python3 scripts/build.py target --operational-mode test --pump05-evidence timing --board-profile firmware/boards/c3-supermini-pinout-01.json --build-dir build/c3-05-test-timing
SPECIFY_FEATURE_DIRECTORY=specs/05-failsafe-arming python3 scripts/build.py target --operational-mode fly --pump05-evidence stall --board-profile firmware/boards/c3-supermini-pinout-01.json --build-dir build/c3-05-fly-stall
```

`--pump05-evidence accuracy|stress` — інші explicit no-pump instruments; звичайні Test/Fly без injection/archives. Explicit05 mode з 03/04 bench/legacy, host+target flags, unknown modes чи incompatible trace options повинні відхилятися до configure. Evidence instrumentation не додає нового джерела команд, GPIO marker або runtime mode switch.

Config/ELF review має довести runtime TWDT100 мс/add(owner)/only-owner feed, IWDT500 мс, сильний C-linkage ISR hook у IRAM з ROM reset target і no-return path, окремий reset-hook TU без успадкованої weak declaration, no default panic у declared watchdog path, LOG max/default0 без pre-hook output, debugger/GDB/coredump/apptrace off. Не припускати cache safety всього SDK за самим IRAM hook. Stack high-water/map і RAM limits перевірити окремо.

## План операторських сценаріїв без насоса

Кожний рядок виконується тільки після readiness та окремого явного operator доручення. До run записати mode/image/boot/source identity, fixture, точку вимірювання, U, initial levels, acquisition bounds/gaps, trigger/pretrigger, repetitions і raw provenance. Flash commands тут навмисно не генеруються.

| Сценарій | Дія/очікування | Доказ |
|---|---|---|
| P1 Startup/reset | Test/Fly: no prior-valid→LOW; first fully-valid high→100%; reset від 80/100 втрачає held duty; cold/manual/software/watchdog/brownout/bootloader/upload | Окремі GPIO0 та driver-input waveform; applicable M04 startup pretrigger≥10 мс,≥5 повторів/тип; current source levels прийняті; до first valid жодного небажаного HIGH≥1 мкс |
| P2 Mapping/output | Шість nominal points; 0/100 сталі; intermediate1000 Гц; high↔PWM↔LOW | M04 per-cycle frequency/duty/voltage/detection, output settling≤20 мс від corresponding valid falling |
| P3 Test loss/recovery | Від 80/100% забрати нові valid; missing/invalid/stuckHIGH/LOW; підтвердитиостанній accepted token; повернути новий high без zero | Hold80/100% при age<T за справних clock/output; Direct corresponding GPIO3fall→сталий GPIO0/driverLOW, L+U≤1 с, потім new-high≤20 мс. Достовірний prematureLOW — fail; upper-bound1с сам не pass. Невідоме age/physical зіставлення — blocked hold-part; у host T±1 мкс окремо |
| P4 Fly hold |0/80/100% при missing/invalid/stuck довше T/1 с;≥600 фізичних секунд loss для active/slow/absent reader; потім належний resync/new high/new low | Без cutoff тількичерез input loss; physical lower-bound duration≥600 с, no gaps; static endpoints і повернення duty |
| P5 Capture/diagnostics | Healthy50 Гц в Test/Fly при max384B/100 мс та reader active/slow/absent/reconnect; short accuracy windows окремо від 600 с stress | D53–D55 E_width≤5 мкс/E_interval≤10 мкс,U≤1 мкс окремо без E+U; zero-loss/rearm/processing/stuck100+10; actual bytes/skips/drops |
| P6 Technical output/clock | Explicit bench-only, по одній погодженій ін'єкції; перевірити fallback/cleanup/reinit3/100 мс/new-valid fence і D75 episode closure лише після checked post-fence valid apply, включно з valid0%; LowUnconfirmed чи fatalclock→reset | MCU API brackets окремо, physical LOW/route/state окремо; APIfailure лишається failure history; непідтверджений LOW не pass |
| P7 Task stall/watchdog | One-shot compiled fixture після відомої accepted/apply події: owner block, busyloop, IRQ/cache stall; diagnostics alive не feed; Test/Fly окремо | Lastqualified feed→reset≤350 мс, reset→physical LOW≤100 мс за незалежним anchor; direct Test last valid→LOW≤1 с. Немає anchor — partialmetricblocked, не timestampUSB |

Test cutoff — початок безперервного LOW у gapless записі, який триває≥100 мс без нової команди; після цього окремо recovery-high. Звичайна LOW частина PWM не cutoff. Для 0% перевіряється відомий staticLOW, а не неіснуючий transition. Fly600 с перевіряє тривале утримання, не фізичну нескінченність або несправний output.

P3 окремо оцінює hold-before-T та максимальний cutoff latency. До run методика має визначити незалежне зіставлення accepted anchor/MCU age із physical waveform, включно з clock/timebase uncertainty; API decision age≥T перевіряється journal-ом, physical hold — приладом. Не прирівнювати фізичні500мс до MCU age без такого методу й не підбирати offset за output transition. Якщо method/uncertainty не дозволяє однозначно розрізнити early LOW і допустиму boundary, hold-part blocked й combined P3 не pass, навіть якщо upper-bound1с пройдено. Technical fault LOW належить P6, не підміняє healthy input-loss P3. Нові числові допуски не задаються.

Timing limits05: U_latency≤1 мс і L+U≤maximum. Це окреме правило для maximum latency; D54/M04 directE/U не змінюються. Не підбирати MCU/analyzer offset з output transitions. DirectGPIO3/GPIO0 часи мають одну analyzer timebase і однозначне зіставлення accepted pulse. Для API timing потрібен повний immutable bench journal; для reset measurements — незалежна придатна reset observation, не USB/OS arrival. Якщо reset anchor запізнюється від actual reset, його відома похибка/latency входить у консервативні bounds; інакше blocked. Зовнішня методика не потребує нового application GPIO.

Timing journal D76 — event-based, один compile-time fixture/scenario на boot. До run записати manifest/scenario identity, start до першої traced operation, final event/checked result, очікувану верхню кількість records (anchors/API/retry включені)≤64. Accepted anchor оновлюється в RAM на кожній accepted valid, але record додається лише для transition/API event/failure/late/retry/D75 closure; незмінні успіхи не записуються. Перед подією зберігається саме її accepted anchor, без now-перемаркування. Post-fence valid0 closure записати навіть при незмінному LOW. Freeze після final checked result; overflow/missing anchor/result→incomplete без впливу на output. Повний immutable export потрібен для API claims; long hold physical waveform, accuracy і stress перевіряються окремо.

Stall reset стирає RAM. Fixture має створювати відомий accepted/apply anchor й one-shot stall без нових command inputs; pre-reset evidence зберігається зовнішнім приладом/host до події. Не видавати зниклий RAM journal за прочитаний після reboot. ROM reset reason може бути software/unknown; це не доказ типу watchdog. Якщо неможливо зіставитиостанню accepted команду, відповідний case blocked.

Accuracy image може навмисно checked LOW→stop capture→export тільки у explicit bench evidence phase; він не доказ production Fly hold. Stress summary601MCU секунда не вимикає вихід при freeze; export одним writer/слотом, capture/control тривають. D62 first/last matched physicalfallings lower-bound≥600 с для healthy regression; planned nominal50 Гц/30000 не source count. Mixed/partial/overflow archives або unknownU не pass.

## Коли зупинити прийняття

Нова невідповідність чинній конституції, невідомі levels/power/bias/interface, неправильний effective mode, missing reset anchor, unknownU/gaps, deadline miss, stale replay, Fly input-loss cutoff, unexpectedHIGH після reset або LowUnconfirmed — видимі blocked/fail, не привід послабити критерії. Q03 actual receiver/ArduPilot config/radio-loss приймається до його використання; HJ-only перевірка незалежна й цього не доводить. Завершення operator прийняття 05 документується окремо; робота насоса 06 не запускається автоматично.
