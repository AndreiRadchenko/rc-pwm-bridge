# Перевірка 04-pump-output після майбутньої реалізації

Дата: 2026-10-30. Plan створює лише дизайн. Код04/нові CLI options/tests/offline validator ще не реалізовані, host/build/physical результати04 відсутні. Наступні команди призначені для окремо дозволеної реалізації та її перевірки; у цьому Plan вони не запускались. [Контракт](contracts/pump-output.md), [модель](data-model.md), [Plan](plan.md) задають нормативні межі.

## Передумови

1. Реалізовані04 policy/adapter/fixtures та documented CLI. Змістовий probe: `firmware/components/bridge_core/pump_output.cpp` і відповідні host suites існують, `scripts/build.py --help` показує pump04 flags. Наявні сьогодні18host suites03 та build03 не є evidence04.
2. `SPECIFY_FEATURE_DIRECTORY=specs/04-pump-output` для всіх Spec Kit workflow скриптів. Plan/Tasks/Implement запускаються окремо за запитом; цей validation guide сам їх не викликає.
3. Pinned environment01 перевірено перед збіркою. З кореня репозиторію виконуються наведені нижче команди; нові build directories мають бути відсутніми, старі results не видаляються.
4. Для фізичної частини окремо потрібні виконані залежні H02/H03/Q03, прийнятий MD20A interface/Q06, придатна методика/прилад і явне апаратне завдання. Насос не підключений у всіх сценаріях04, включно з усіма reset.

## Host і tooling без плати

Після реалізації04:

~~~sh
python3 scripts/build.py host --build-dir build/host-04-validation
/private/tmp/scopa01-host/cmake/cmake-3.30.2-macos-universal/CMake.app/Contents/bin/ctest --test-dir build/host-04-validation -N
/private/tmp/scopa01-host/cmake/cmake-3.30.2-macos-universal/CMake.app/Contents/bin/ctest --test-dir build/host-04-validation --output-on-failure
python3 -m unittest discover -s tests/tooling -p 'test_*.py'
~~~

Перевірити presence pump_output_policy_tests, pump_output_adapter_tests, pump_output_evidence_tests і header probe, а також усі наявні01–03 suites. H1–H6 із Plan: всі1..99/counts та endpoints; percent>100; fixed5с/65с/605с deadlines±1мкс; lateness999/1000/1001мкс; fault перед/одночасно з nonzero, stale generation; reset як нова сесія; startup retry не відновлює boot permission; partial API/cleanup/LowUnconfirmed; formatter383/384B та no history; trace freeze/overflow/clock regression/неоднозначне зіставлення/архіви partial/mixed/digest. Тести не залежать від wall-clock sleeps або справжнього MCU.

Meaningful negative probes повинні виявляти невалідні команди/фальшивий pass, а не лише повторювати реалізаційні числа. Tooling також відхиляє pump04 на host, конфлікт із rc03-bench, trace-step поза accuracy/range0..5 і відсутній явний trace-step для accuracy. Expected failure exit codes явно фіксуються у `verification/host.md` при фактичному виконанні.

## Native target builds без hardware access

Наступні `--pump04-bench`, `--pump04-trace-step` і `--require-pump04` — контракт майбутнього CLI, не вже встановлені options. Після реалізації:

~~~sh
python3 scripts/environment/check.py --manifest tools/environment/environment.lock.json
python3 scripts/build.py target --board-profile firmware/boards/c3-supermini-pinout-01.json --build-dir build/c3-04-normal
python3 scripts/verify/config.py --build-dir build/c3-04-normal --require-pump04 normal
python3 scripts/build.py target --board-profile firmware/boards/c3-supermini-pinout-01.json --pump04-bench waveform --build-dir build/c3-04-waveform
python3 scripts/verify/config.py --build-dir build/c3-04-waveform --require-pump04 waveform
python3 scripts/build.py target --board-profile firmware/boards/c3-supermini-pinout-01.json --pump04-bench stress --build-dir build/c3-04-stress
python3 scripts/verify/config.py --build-dir build/c3-04-stress --require-pump04 stress
python3 scripts/build.py target --board-profile firmware/boards/c3-supermini-pinout-01.json --pump04-bench accuracy --pump04-trace-step 2 --build-dir build/c3-04-accuracy-40
python3 scripts/verify/config.py --build-dir build/c3-04-accuracy-40 --require-pump04 accuracy
~~~

Окремі accuracy builds для решти потрібних trace steps0..5 мають інші нові directory names та config hashes. Flag вибирає тільки момент збору в незмінній послідовності, не ручне керування duty.

B1–B4 review actual compile_commands/config/map/source graph: native C3/pins03/0/noDIR, core C++11, actual pump_output objects, normal bench off, правильний scenario/period/trace step, LEDC0/APB/13bit/no fade service/немає іншого writer, PM/radio/sleepoff, driver logs disabled у критичному path. Зберегти незмінні ISR IRAM closure/data03; нові output functions не викликаються з ISR. Actual RAM/sizeof payload/evidence/stack bounds та source/artifact SHA256 — у `verification/build.md` і config review. Наявний build/config pass03 не приймає04.

## Послідовність фізичного прийняття без насоса

Ці дії не виконуються автоматично з Plan чи наведення команд. Не наводимо flash/serial/device commands як частину автоматичного validation. Перед окремо дозволеним стендом:

- Зафіксувати образ normal/waveform/stress/accuracy, firmware/config/source hashes, profile/unit ID, реальну MD20A revision, wiring/ground/power/bias, стан «насос від'єднаний», відсутність сторонніх output clients.
- Перевірити живлення3,3В/якість/режимUSB та PWM bias за H02/H03; одночасний USB VBUS+external3V3 не стає прийнятим через наявність logging. HJ conditioning/routing/internalpullsoff D56 приймаються до його використання. Без чинного capture потоку його fault припинить тест; його не відключати для проходження04.
- До живлення драйвера прийняти ревізію/логічні рівні/землю/перемичку DIR–GND та вплив кнопок. Окремо записати disconnected-driver та powered-driver без мотора. Hardware suitability не виводиться зі схемного символу або табличного20A rating.
- Прилад: model/calibration/timebase/threshold/bandwidth/sampling/probe loading/trigger/storage gaps та U перевіряються за контрактом ДО вимірювання. Дані незрозумілого формату або без U — blocked. Digital timing не доводить voltage levels.

### P1 — Нормальний образ і startup

Normal завжди LOW/disarmed при довільному RC, slow/absent USB reader/reconnect та всіх reset. Після bootloader/upload нема nonzero. Мінімум5повторів cold-start/reset/bootloader/upload у кожному з двох driver conditions, full startup capture/pretrigger≥10мс; bootloaderLOW спостерігати≥10с. Підтверджувати LOW≤0,8В за U та відсутність≥1мкс протилежних pulses у declared method. Інтервал до firmware покриває фактичний апаратний bias, не GPIO initialization.

### P2 — Одноразова waveform сесія

Образ waveform: після handoff t0 початковийLOW5с, потім0/25/40/50/80/100% по10с, після65сLOW постійно. Фізично спостерігати до≥t0+75с, щоб перевірити відсутність повтору. У bench reset може запустити нову сесію за D59 — це очікувано й вимагає від'єднаного насоса. Попередній крок не resume. Capture fault протягом initialLOW або тесту означає terminalLOW, не silent skip.

На GPIO0 і driver PWM input перевірити кожну stable period у+1..9с window: частота1000±1Гц, duty nominal±0,1п.п.; U_f≤0,1Гц/U_d≤0,02п.п. окремо. Немає усереднення чи округлення E для pass. Endpoints сталі весь10с plateau, без повторних spikes≥1мкс; частота дляstatic HIGH/LOW не обчислюється. Перехідні області перевірити окремо: MCU API deadline≤2мс та незалежні physical plateau intervals: E≤0,1% тривалості+2мс, U≤0,01% тривалості+0,1мс; не видаляти їх як «погані samples». VoltageLOW/HIGH acceptance — за контрактом, з U≤0,05В і guard bands.

### P3 — Reset/upload у стендовому образі

Записати падіння/новий boot під час ненульового кроку. До firmware та протягом нового startup/initialLOW — LOW; після нового boot і успішних перевірок починається лише нова послідовність від0%, незалежно від reset reason. У bootloader/upload до app послідовності немає. Власного automatic reset firmware не створює. Повне фізичне прийняття brownout/watchdog належить05/06; тут перевіряється D59boot policy в доступних дозволених reset scenarios і записуються непроведені.

### P4 — Fault і deadlines

Для кожного ненульового plateau окремий boot/прогін: після перевірки fixture утворити supported stuckHIGH/LOW або explicit fault, перевірити terminalLOW без повтору/очищення latch. Обидва фізичніGPIO3/0 на одному analyzer дають source-level unchanged→сталий terminalLOW≤112мс; звичайні LOW-частини PWM не є cutoff. До T0 потрібний active nonzero plateau, після T_cut — gapless terminal спостереження≥10с без новогоHIGH≥1мкс; зберегти весь threshold/fault region, не час USB рядка. Malformed/queue/API cases програмно мають meaningful fixtures; фізичне втручання/коротке замикання або небезпечні API-fault fixtures не вигадуються.

Owner-fault→checked LOW API≤1мс та scheduled deadline→API result≤2мс перевіряються за frozen OutputEvidence у тому самому MCU-clock domain, із raw status кожної операції. Фізичні LOW/форма/plateau intervals та112мс перевіряються незалежно приладом. Не обчислювати clock offset із припущення про швидкий output та не називати це доказом owner-fault→physical LOW2мс. Cross-domain latency без незалежного calibration anchor — blocked, такого pass04 не заявляє. Тест зі спільним shift усіх фізичних transitions>2мс повинен відхилити або блокувати спробу cross-domain pass. Apply API-success або LowUnconfirmed не є proof фізичногоLOW. Стенд не підтверджує поведінку пошкодженого силового тракту.

У записах відрізняти ApplyApiError від LowUnconfirmed: помилка stop0 з повністю успішним GPIO fallback залишає Fault/failed run, але без нового LowUnconfirmed; неповний/невдалий fallback лишає LOW непідтвердженим. Fallback не починає новий часовий budget, раніше встановлені sticky reasons не очищуються. Підтвердження API-level LOW та вимірювання приладом — окремі докази.

### P5 — Навантаження діагностики та capture regression

Stress output за D60:5 с initialLOW, далі 6×100 с, кінцевий LOW на t0+605 с. Evidence за D62: window[t0+5 с,t0+606 с) має 601 MCU-секунду; остання секунда проходить із LOW та чинними capture/max diagnostics. Аналізатор окремо підтверджує≥600 фізичних секунд із невизначеністю за контрактом. Для active/slow/absent reader — окремі runs; record attempts≤10/с з min100 мс, available reader384B/slot, actual bytes/drops окремо. Capture candidates/valid/invalid/loss рахуються за raw analyzer у тих самих window bounds; не припускати рівно 30000 через номінальні 50 Гц. Нуль valid losses/reorder/duplicates, чинні rearm≤1 мс/processing≤10 мс/stuck delivery≤110 мс збережені. Accuracy collector у stress вимкнений; після завершення до≥t0+615 с перевірити terminalLOW/no repeat.

За D61/D62 підсумок цього вікна зберігається в CaptureWindowEvidence≤512B незалежно від USB; samples відносяться за estimated falling end_us до напіввідкритого вікна, навіть за обробки після end. Output Complete на 605 с не закриває collector і не запускає export. Після кінцевого LOW capture/max diagnostics тривають до 606 с та closing. Закриття summary після ingress barrier/drain має пройти guard≤17 мс за контрактом без розширення бюджетів 03. Поточні global counters після reconnect не використовувати замість window counts. Після завершення й повернення reader зберегти повний rc04-win H/B/W/T/E цикл разом із rc04-out, live fault summary та raw traces; partial/mixed цикл непридатний. Summary незмінний до нового boot, а пізня належна подія позначає окремий sticky invalidation і робить run failed. Протягом 601 MCU-секунди та closing export не виконується.

Для source/window matching протокол до запуску має задати однозначну відповідність boot/edge sequence та першого/останнього належного falling до індексів імпульсів raw analyzer за методом03, включно з обома boundary pulses. Не виводити її з припущення30000, довільного best-fit або підгону за PWM transitions. Невідомий anchor/граничний імпульс чи прогалини запису — blocked; невідповідні counts/order, missed budgets або втрати — fail. Global baseline/end snapshots є лише допоміжними: вони можуть включати події поза вікном.

Для coverage записати raw analyzer first/last matched falling indices, T_obs, duration_uncertainty_ns та T_lower=T_obs−U_duration. Pass вимагає T_lower≥600000000000 нс, gapless source trace й узгоджених counts/order; номінальні MCU-секунди не замінюють цей доказ. Невідомий U/association/gaps — blocked; повний придатний запис коротше 600 с за lower-bound — fail, без автоматичного продовження або повтору. Перевірити output LOW на 605 с і collector freeze тільки після 606 с.

Окремо Accuracy builds перевіряють width5мкс/interval10мкс, U≤1мкс окремо за D54, у128candidate RAM windows за D55. Collector стартує на+1с вибраного trace step, отже фактичний PWM вже активний для25–100%. На freeze спершу checkedLOW, потім capture stop/export; помилка LOW дає Fault/неприйнятне вікно, а LowUnconfirmed блокує навмисний normal freeze/stop; це очікуване закінчення evidence build, не доказ full65с sequence/600сloss. Один USB writer експортує незмінні rc03-trace та rc04-out cycles після freeze, no per-pulse USB під час capture. Для кожного вікна raw archive/analyzer identity/order/counters мають узгоджуватися; invalid/unknown не вилучаються для pass. Точні boundary/malformed stimuli лишаютьсяhost за D53; окремий генератор не додається як новий gate.

Для offline capture comparison використовується вже наявний `scripts/verify/rc_capture_trace.py` та формат03. Normalized analyzer CSV для output/fault: `time_ns,channel,level` (integer ns, channel `gpio0` або `gpio3`, level0/1), з monotonic edges кожного каналу та спільною часовою базою аналізатора; GPIO3 потрібний для112мс, без нього ця метрика blocked; raw vendor export і normalization provenance зберігаються. Майбутній `scripts/verify/pump_output_trace.py` приймає `--archive`, `--edges`, `--metadata`, `--report`; metadata version1 містить firmware/scenario/window identity, declared timebase/U/voltage method і physically measured voltage bounds, acquisition_start_ns/end_ns, initial_levels, observed_windows та gap/overflow flags. CSV містить лише edges: static trace без edges прийнятний тільки з цими boundary/initial-state доказами; порожній CSV сам не підтверджує LOW/HIGH. MCU API метрики та фізичні періоди/plateau/112мс порівнюються окремо, без clock-offset підгонки; cross-domain запит без незалежного calibration provenance не може дати pass.

~~~sh
python3 scripts/verify/pump_output_trace.py --archive evidence/rc04-out.txt --edges evidence/pwm-edges.csv --metadata evidence/method.json --report evidence/output-report.json
~~~

Цей validator теж ще не реалізований. Контракт exit0=pass усіх запитаних метрик,1=fail виміряного критерію,2=blocked/insufficient method/incomplete evidence. Немає voltage records — voltage blocked, а не inferredpass із0/1. Header/digest/count/order/errors/raw hashes/per-cycle errors і maxima записуються в report; не змішувати unrelated boot archives. Недостатнє U/clock association/storage — blocked без послаблення лімітів.

## Окремі результати та завершення

Після фактичного виконання: `verification/host.md` для host/tooling; `verification/build.md` і config review для target; `verification/hardware.md` для raw instruments/method/results; `verification/acceptance.md` для R/FR/SC→evidence та конституційного review. Вказати всі commands/config/hashes, умови/межі, expected/measured, failures і unperformed checks із owner. Unknown методика/revision/напруга/сценарій не стають pass. Завершення04 потребує всіх обов'язкових фізичних критеріїв, а не створення цих файлів.

LOW MD20A — brake; напрямок насоса, реальна витрата, power isolation, current/thermal та незалежне emergency cutoff приймаються в06 після05. Жодна наведена тут команда не прошиває плату, не активує навантаження й не запускає наступну фазу Spec Kit.
