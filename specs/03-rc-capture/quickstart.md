# Перевірка 03-rc-capture після реалізації

Програмну реалізацію виконано за окремим Implement. Host18/18 і три target variants перевірено; фактичні результати — [acceptance](verification/acceptance.md). Фізичні перевірки not_run, flash/load потребують окремого доручення. Цей документ описує повторний запуск. Деталі — [plan](plan.md), [contract](contracts/rc-capture.md), [модель](data-model.md).

## Передумови

З кореня репозиторію перевірити environment pins, наявність нових capture sources/host suites та профіль firmware/boards/c3-supermini-pinout-01.json. Програмна основа 01/02 потрібна; фізичний стенд додатково потребує H02/H03/Q03, приладу й окремого hardware завдання. За D48 receiver не блокує HJ acceptance03, але має власний gate до використання.

## Host без ESP32 headers

У новому build directory:

~~~sh
python3 scripts/build.py host --build-dir build/host-03-validation
/private/tmp/scopa01-host/cmake/cmake-3.30.2-macos-universal/CMake.app/Contents/bin/ctest --test-dir build/host-03-validation -N
/private/tmp/scopa01-host/cmake/cmake-3.30.2-macos-universal/CMake.app/Contents/bin/ctest --test-dir build/host-03-validation --output-on-failure
~~~

Відсутні pins/tools/suites або лише попередні 8 tests означають, що 03 не перевірена. Не видавати pass02 за нове evidence. Наявні suites:

- H1: шість точок карти, width 899/900/2100/2101, period 17999/18000/22000/22001, перший period unknown, допустимий перехід 19→21 мс, invalid-to-high regression, last-valid.
- H2: startup HIGH/LOW, missing rising/falling, HIGH≥5 мс без штучного end, stuck threshold±1 мкс, delivery до 110000, late poll як deadline failure, latch/valid-under-fault/resync/effective 0.
- H3: sequence wrap, timestamp через 2³² мкс, regression, duration extremes/zero/levels, epoch mismatch, queue overflow/sticky error, saturated 48 symbols, delayed/reordered callback, rearm error, guard 12 мс, processing 10 мс, fault barrier, concurrent edge/monitor/arm/recovery checks, guard reset після final HIGH-overrange.
- H4: diagnostic spacing99999/100000 мкс, max10/с, overwrite/drop/насичення counters, busy/absent reader/reconnect, один in-flight record/без історії, zero/partial mock write, максимальні поля≤384 B. Portable header/core compile без IDF/FreeRTOS.

Fake clock задає час без wall-clock sleeps. Test list/команди/версії/факти/невиконані cases записати у verification/host.md при виконанні.

## Target build без плати

~~~sh
python3 scripts/environment/check.py --manifest tools/environment/environment.lock.json
python3 scripts/build.py target --board-profile firmware/boards/c3-supermini-pinout-01.json --build-dir build/c3-03-validation
python3 scripts/verify/config.py --build-dir build/c3-03-validation
~~~

Build directory має бути новим, старі artifacts не видаляти. B1: SDK/toolchain, C3 target, native RMT/GPIO graph, нові capture/core objects, effective cache-safe/IRAM config і ISR call graph/data placement у map; radios/PM/sleep off, arming відсутній, profile/output LOW. Записати static RAM/stack sizes/hashes у verification/build.md. verify/config перевіряє також effective RMT/GPIO settings, actual capture objects, trace graph та core flags. Build не доводить runtime latency чи фізичної безпеки.

## Операторський стенд після окремого завдання

Не переходити сюди автоматично. MD20A/насос від’єднані; перевірена схема пониження HJ signal до 3,3 В, спільна земля, analyzer на GPIO3 після conditioning. H02/H03/Q03 потрібні для цього стенда. Firmware flashing — окремо. 125H/250H не підтримуються 03.

До запуску записати analyzer model/timebase accuracy/threshold/sample rate. Вибрати не нижче 10 MS/s, крок≤0,1 мкс; невизначеність U_width та U_interval кожна ≤1 мкс; для interval — на всьому перевіреному діапазоні до 22 мс. Якщо прилад цього не забезпечує — blocked. Absolute clock sync не потрібна. За D53/CHK025-A точні межі 18/22 мс, точні граничні ширини й складні malformed streams покривають host fixtures; окремий фізичний генератор не блокує 03. Не приписувати HJ відсутні можливості.

1. P1: короткі accuracy windows128 candidates за D55, 50 Гц, доступні width points/краї/зміни; для кожного вікна зберегти повний frozen RAM archive після зупинки capture та відповідний analyzer trace. Порівняти кожну complete width з trace: E_width=abs(width_RMT−width_analyzer) ≤5 мкс. Для кожної відповідної пари rising: E_interval=abs(interval_GPIO−interval_analyzer) ≤10 мкс. За D54/B порівнювати E без додаткового запасу U; U_width/U_interval ≤1 мкс записуються окремо. Недоступні точні boundary stimuli — host-only за D53, без заяви physical pass для них і без блокування 03 через окремий генератор. Пройти доступний діапазон HJ, записуючи фактичні width/period та дані поблизу valid меж.
2. На чистому valid потоці перевірити RAM timing instrumentation без per-edge serial: stop=falling+5000, rearm≤1000 мкс післяstop, processing≤10000 мкс післяcompletion. Додатковий markerGPIO потребує окремо прийнятого профілю, GPIO не вигадувати.
3. Фізично обов’язкові startup HIGH/LOW, відсутність імпульсів і тривалий сталий HIGH/LOW після перевірки електричного fixture; складні malformed-послідовності — host за D53: capture status/resync/latch. Незмінний level threshold 100 мс, core fault до 110 мс; записати threshold_at/reported_at/age, не час появи тексту. Operational cutoff05 тут не перевіряється.
4. P2: accuracy trace off; окреме стабільне 600 с вікно для кожного worst-case diagnostic режиму: max 384 B/100 мс з активним читачем, повільним читачем і читач недоступний; reconnect окремо. Expected correct count analyzer має дорівнювати matched/accepted valid count, losses0. Не припускати рівно 30000 pulses; не виключати rearm loss усередині вікна. Source invalid рахувати окремо.
5. У всіх режимах GPIO0 LOW, effective 0, permission false. LOW MD20A означає brake, не ізоляцію живлення/нуль витрати. Receiver має окрему перевірку до використання; одночасні два канали —07.

verification/hardware.md: firmware hash/config, source/analyzer/conditioning/напруги, expected/measured width/period/timestamps, максимальна похибка кожного pulse, counts/missing/duplicates, duration/load/counters, rearm/processing/fault latency, raw trace provenance. Провал — failed; tolerance не підганяти під результат. Невиконане — not_run/blocked із конкретною залежністю. Plan не закриває фізичних gates.


## Уточнення сценаріїв checklist 2026-10-29

- H2/H3: спостережений HIGH/LOW триває 105 мс, новий edge до owner poll109 мс — попередній stuck episode має бути повідомлений до 110 мс. Poll після 110 мс фіксує deadline_missed/lateness і failed deadline, а не pass із виправленим timestamp. Окремо masking gap: continuity unknown, без вигаданих edges/freshness, resync під збереженим latch.
- H4: overwrite/version ack при concurrent publish, повторне читання тієї самої version, busy slot без write та zero/partial write рахуються у визначених різних одиницях.
- P2 max load: у bench build актуальний record доповнюється пробілами до 384 B включно з framing; ≥100000 мкс між спробами. За available reader —384 B/100 мс; за slow/absent — та сама частота спроб, фактичні bytes/attempts/skip/drop записуються. Fake capture values не підставляються.

## Правило прийняття CHK026-B / D54

D54 / CHK026-B (2026-10-29): pass визначається прямим порівнянням виміряної абсолютної похибки E з допуском: E_width ≤5 мкс для кожного коректного імпульсу, E_interval ≤10 мкс для кожного відповідного rising interval. Невизначеність порівняння U_width/U_interval документується окремо і має бути ≤1 мкс для кожної метрики; U не додається до E й не віднімається від допуску. Невідоме U або U >1 мкс — blocked для фізичного прийняття цієї метрики; перевищення E за придатної методики — fail. Без averaging чи округлення E перед рішенням; фізичні результати ще не отримані.

До запуску записати analyzer model/timebase specification, sample interval, пороги й спосіб зіставлення фронтів та обґрунтування U_width/U_interval для застосованого setup. Крок дискретизації сам по собі не є повною оцінкою U. Не припускати фактичного U на підставі цього документа.

Приклад правила: E_width=4,6 мкс і U_width=0,8 мкс — pass за D54, бо E≤5 та U≤1; E_width=5,1 мкс при тому самому U — fail. Формула E+U не використовується. Не компенсувати похибку постфактум середнім offset/drift для отримання pass. Зберігати raw measurements, per-sample E, max E та U окремо; середнє не замінює per-pulse criterion.


## Коротке accuracy window — C1-A / D55

Для T052–T054 використати accuracy-trace+diagnostics-stress build. Analyzer записує сигнал до початку RC; verified LOW і firmware waiting-for-signal phase встановлюють відому межу першого rising. Reader condition (active/slow/absent) задається до подачі RC, після підтвердження готовності; production arming тут немає.

Collector збирає 128 послідовних candidates у bounded RAM без USB records на імпульс, normal diagnostics працює під max load. Не виключати invalid/unknown/error candidates; перший unknown period не заважає width comparison. Заповнення завершує closed window і freeze, потім owner зупиняє capture. Не включати dump/restart до capture timing/loss window.

Після freeze відновити reader, якщо він був відсутній, і зберегти повний HEADER/128 indexed records/END cycle з marker rc03-trace. Export повторює незмінний bounded archive з ≤10 frames/с і max384 B/frame; пропущені/partial cycles не приймати. Створити окремі raw files для кожної bench session, не змішувати однакові session labels різних boots. Для наступного вікна — окремо доручений operator restart, не command/arming.

Нормалізувати analyzer export у CSV pulse_start_ns,pulse_end_ns, зберігши raw export/hash та опис нормалізації. Offline tool реалізовано; виконати з кореня:

~~~sh
python3 scripts/verify/rc_capture_trace.py --trace frozen-export.txt --analyzer normalized-edges.csv --u-width-us 1 --u-interval-us 1 --report comparison.json
~~~

Це приклад offline команди; числові U замінити фактично обґрунтованими значеннями ≤1 мкс. Невідоме U — blocked, не довільне 1. Tool перевіряє повноту/count/order/відповідність, кожну RMT width та всі відповідні rising intervals, D54 без округлення E/average/offset/drift adjustment. Перший record дає width; interval comparisons починаються з другої відповідної rising-пари. Зберегти всі per-record errors, maxima та source hashes.

Повторити короткі вікна по доступному HJ діапазону та при active/slow/absent reader з max diagnostics. P2 окремо триває 600 с на режим без collector: analyzer counts, cumulative counter deltas, duplicates/drops і max rearm/processing/fault latency. P2 сам не доводить accuracy кожного імпульсу за 600 с; P1 не доводить zero loss за 600 с. Обидва набори evidence обов’язкові.

## Фактичні build variants і suites

Для нового запуску кожен build directory має бути відсутнім (наявні artifacts не видаляти):

```sh
python3 scripts/build.py target --board-profile firmware/boards/c3-supermini-pinout-01.json --build-dir build/c3-03-normal-new
python3 scripts/build.py target --board-profile firmware/boards/c3-supermini-pinout-01.json --build-dir build/c3-03-stress-new --rc03-bench diagnostics
python3 scripts/build.py target --board-profile firmware/boards/c3-supermini-pinout-01.json --build-dir build/c3-03-trace-new --rc03-bench accuracy
python3 scripts/verify/config.py --build-dir build/c3-03-normal-new
python3 scripts/verify/config.py --build-dir build/c3-03-stress-new
python3 scripts/verify/config.py --build-dir build/c3-03-trace-new
python3 -m unittest discover -s tests/tooling -v
```

Normal — trace/stress off; diagnostics —384B/max10с, trace off; accuracy — stress+trace. Trace object відсутній у normal/stress compile graph. Джерела, назви18 CTest suites, версії та actual outcomes — [host.md](verification/host.md), [build.md](verification/build.md). Протокол нижче не є командним інтерфейсом.

## Діагностичний текст і архів

Кожен normal record має newline + `rc03 ` + key=value + newline, ≤384B. Bench доповнює його пробілами до384B включно з framing. Поля `w` (RMT width us), `p` (period us), `v` (status0None/1Valid/2PeriodUnknown/3InvalidWidth/4InvalidPeriod/5Incomplete/6Error), `c` (capture phase0WaitingRise/1WaitingLow/2Receiving/3Resync), `d` (calculated percent), `o=0`, `a=0`, `sat` — десяткові. Решта чисел — hex без0x:

| Поле | Значення |
|---|---|
| q,id,sid | Snapshot version, profile fingerprint, capture observation start us (boot-relative session label, не глобальний UUID) |
| s,e | Estimated rising/falling us; incomplete end0, width окремо від e−s |
| f | Sticky CaptureFault bit mask із rc_capture.hpp |
| n | candidates,valid,invalid,dropped,fault events,duplicate edges (доступні software observations; universal loss detection не обіцяється) |
| k | overwritten snapshots,busy slots,write drops,total slots/attempts,actual bytes accepted by USB driver |
| m | max rearm gap us,max completion→processing-end us,max fault lateness us |
| ft | Latest stuck threshold_at/reported_at us; час повідомлення ядру, не USB тексту |
| b | Startup phase,attempts,error,pwm_low_api_applied; enum values із startup.hpp |

Counter saturation sticky; production reset/clear/arm commands немає. k.bytes означає driver acceptance, не доказ прочитаних host bytes. One-in-flight gate — wait_tx_done(0); zero/partial write counts як один drop, remainder не повторюється. Pending latest замінюється, заміна рахується один раз. Busy slot не дублюється write-drop. `k` у поточному тексті передує результату саме цього write; наступний snapshot показує оновлений total.

Bench archive після freeze/stop (усі числа десяткові):

```text
rc03-trace H 1 count failed start_us end_us 0 base_drops end_drops base_faults end_faults max_rearm_us max_processing_us max_fault_lateness_us
rc03-trace R index seq epoch start_us end_us width_us period_us flags fault_bits accepted_at_us
rc03-trace E count
```

Version1,count128, literal0=capture_active false. flags: status low8bits,complete256,period_known512. `accepted_at_us` — час завершення owner processing, не edge timestamp. Header start/end визначають закрите вікно; counters baseline/end і max timings не скидають реальний latch. Record48B, trace6208B на C3, static assertions record≤48/header≤256/total≤6400. Нормалізований CSV має лише `pulse_start_ns,pulse_end_ns` (цілі ns). Tool відхиляє incomplete/failed/mixed epoch/gaps; невідомеU або неповний export — blocked. Raw files однієї boot session не змішувати з іншою.

Повна ручна процедура й gates — [hardware-procedure.md](verification/hardware-procedure.md); поточний physical status — [hardware.md](verification/hardware.md).

## Поточні артефакти після D56

Після створення RMT каналів firmware вимикає внутрішні підтяжки GPIO3 (GPIO_FLOATING), bias задає зовнішній conditioning. Нові перевірені directories: build/c3-03-bias-normal,build/c3-03-bias-stress,build/c3-03-bias-trace. Старі build/c3-03-final/stress/trace не містять D56 і збережені як попередні artifacts; для ручного прошивання після окремого доручення використовувати нові bias directories. Hashes/build results — verification/build.md; фізична придатність дільника pending.

## Поточні артефакти після виправлення USB RX

Останні перевірені builds: build/c3-03-usb-normal,build/c3-03-usb-stress,build/c3-03-usb-trace. Вони містять D56 та RX128 замість несумісних із SDK64; TX512 незмінний. Старі bias builds збережені, але не містять виправлення USB RX. Для моніторингу підходить `pio device monitor --port /dev/cu.usbmodem1101 --baud 115200 --dtr 0 --rts 0` (порт звірити зі своєю платою); очікувані application records починаються rc03, ROM boot log їх не замінює. Сам запуск монітора не є повним acceptance.
