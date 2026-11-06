# Специфікація: прозоре керування насосом, Test/Fly та автоматичне відновлення

**Ітерація**: `05-failsafe-arming`\
**Каталог**: `SPECIFY_FEATURE_DIRECTORY=specs/05-failsafe-arming`\
**Гілка**: `05-failsafe-arming`\
**Створено**: 2026-11-06\
**Стан**: Specify/Clarify завершені; Plan визначив технічний дизайн D69–D73. G05 закрито конституцієюv2.0.0/D74; фізичні gates відкриті, реалізація й прийняття05 не виконані.\
**Конституція**: v2.0.0, поправка2026-11-06/D74; первинна ратифікація2026-10-28. Історію конфліктуv1.0.0 збережено нижче.\
**Вхід**: поточне доручення користувача, [промпт05](../../speckit-iteration/05-failsafe-arming/specify.md), обов'язкова [поточна основа](../../speckit-iteration/current-baseline.md) та відповіді користувача в цій сесії.

## Обсяг і межі рішень

Мета05 — завершити прозоре перетворення одного перевіреного RC-PWM сигналу на PWM MD20A, режимну поведінку Test/Fly та автоматичне відновлення перед контрольованим прийняттям насоса 06. Насос дозує пальне й керує тягою; конвертор лише перевіряє та транслює команду. Власна логіка запуску, горіння, керування й відновлення двигуна поза його відповідальністю (D63).

- **Залежності**: 01-foundation,02-control-core,03-rc-capture,04-pump-output. Простежувані вимоги: R01,R03,R04,R05,R06,R07,R08,R09,R10,R11,R12 із [реєстру](../../docs/requirements.md). R02 успадковується як обмеження середовища, без переобрання залежностей.
- **Входить**: перша повністю валідна команда поточного boot, окремі Test/Fly збірки, свіжість від accepted pulse_end_us, missing/invalid/stuck/capture loss, автоматичне повернення прямо до високої команди, технічні відмови й watchdog, локальна діагностика та прийняття без насоса.
- **Не входить**: рішення автопілота/двигуна, реверс/MCU DIR, радіозв'язок, runtime/USB/persistent mode, збереження duty через reboot, фільтрація/автокалібрування, силове прийняття/струми/тепло/витрата насоса 06, запалювання/інші ESP32/Nano07–09. Назва каталогу історична: окремий operational arming у05 не вводиться.
- **Профіль**: `c3-supermini-pinout-01`, одиниця `c3-supermini-188b0e93032c`, RC=GPIO3, PWM=GPIO0, постійна DIR–GND перемичка на MD20A без MCU GPIO; зовнішні номінальні3,3 В на3V3/GND. [Профіль](../../hardware/esp32-c3-supermini-profile.md) та [JSON](../../firmware/boards/c3-supermini-pinout-01.json) не підтверджують фізичну придатність.
- **Інваріанти**: native ESP-IDF, переносиме ядро, один локальний канал, Wi-Fi/Bluetooth/мережеві сервіси вимкнені; вихід1000 Гц при проміжному duty,0% — сталий LOW,100% — підтримуваний сталий HIGH. LOW MD20A означає brake, не coast, ізоляцію живлення чи доведений нуль витрати.

### Джерела та фактичний стан залежностей

Нормативні джерела: [рішення](../../docs/decisions.md), [вимоги](../../docs/requirements.md), [архітектура](../../docs/architecture.md), [roadmap](../../docs/roadmap.md), [відкриті питання](../../docs/open-questions.md), [AGENTS.md](../../AGENTS.md), [апаратні джерела](../../hardware/README.md) і [конституція](../../.specify/memory/constitution.md). Старі статусні абзаци не переважають фактичні evidence records. `PROJECT_FOUNDATION.md` — історія; двоканальний старт, WROOM, довільні GPIO й low-side flyback-схема не переносяться.

| Залежність і прочитані контракти | Фактичний стан за acceptance | Наслідок для05 |
|---|---|---|
| 01: [spec](../01-foundation/spec.md), [startup](../01-foundation/contracts/startup.md), [профіль](../01-foundation/contracts/board-profile.md), [build/evidence](../01-foundation/contracts/build-and-evidence.md), [acceptance](../01-foundation/verification/acceptance.md) | Є host/build/config докази; E01/Q01 закрито для pinned environment; H02/H03, SC-004/SC-006 фізично відкриті | Перевикористати основу; повторно перевіряти середовище перед збіркою; software LOW не доводить startup/bias |
| 02: [spec](../02-control-core/spec.md), [контракт ядра](../02-control-core/contracts/control-core.md), [acceptance](../02-control-core/verification/acceptance.md) | Власні програмні критерії виконані за D38; історична модель завжди disarmed/effective0 | Розширити ядро, зберігши карту/validation/invalid hold/time-order; model-only clear_fault не є production recovery |
| 03: [spec](../03-rc-capture/spec.md), [plan](../03-rc-capture/plan.md), [контракт](../03-rc-capture/contracts/rc-capture.md), [acceptance](../03-rc-capture/verification/acceptance.md) |18/18 host,17/17 tooling,3 build/config; фізичні T043–T047/T051 відкриті. Часткові USB/HJ/sweep логи не приймають точність/600 с | Успадкувати асоціацію, бар'єри/resync, облік втрат і бюджети; відокремити sticky history від operational inhibition |
| 04: [spec](../04-pump-output/spec.md), [plan](../04-pump-output/plan.md), [контракт виходу](../04-pump-output/contracts/pump-output.md), [acceptance](../04-pump-output/verification/acceptance.md) |22/22 host,28/28 tooling,9/9 build/config;46/53 задач, фізичні T044–T050 відкриті | Перевикористати вихід/єдиного власника; явно мігрувати terminal adapter для operational05; finite bench04 зберігається окремо |

Ці результати належать попереднім ітераціям, не новим перевіркам05. [Environment lock](../../tools/environment/environment.lock.json) фіксує ESP-IDF v5.5.6 commit `880a203ae603d08a94d06aa01e0d6761958e5e9e`, strict C++11 core/C++17 platform і pinned tools. Локальні тимчасові шляхи перевіряються перед reuse; їхня наявність зараз не припускається. Новий build system або SDK не обираються.

### Успадкована валідація та вхідні часові межі

1. Width900–2100 мкс і period18000–22000 мкс включно; підтримка входу лише50 Гц. Period між послідовними trustworthy rising; unknown period не є valid. Invalid period сам не latch fault; останній trustworthy rising лишається origin наступного period (D41/D45/D49).
2. Width/period/association перевіряються до clamp.1000/1250/1400/1500/1800/2000 мкс →0/25/40/50/80/100%; nearest whole percent із0,5% вгору, без фільтрації/learning. Валідні900–999/2001–2100 обмежуються endpoint тільки після валідації (D36).
3. RMT width незалежна від різниці estimated GPIO ISR timestamps. Last-valid — falling pulse_end_us accepted complete valid у спільному монотонному домені; не callback/worker/print time. Invalid/unknown/incomplete/old/ambiguous не створюють команду й не оновлюють freshness (D51).
4. Overflow/drop/malformed/association error зберігають status/fault, discard пошкодженого фрагмента, epoch/sequence/time-order barriers та автоматичний resync. Після втрати меж потрібні два нові trustworthy rising. Resync сам не є valid; старий sample не перемарковується часом now (D46/D52).
5. StuckHIGH/LOW:100000 мкс незмінного рівня від останньої спостереженої зміни або observation start; додатковий бюджет повідомлення10000 мкс. Завершений довгий епізод не стирається новим edge; masked interval не є доказом безперервного спостереження.
6. Зберігаються RMT1 МГц/filter0/stop5 мс/48symbols, rearm≤1 мс після stop, processing≤10 мс після completion, active guard12 мс плюс10 мс доставки. D56 вимикає GPIO3 internal pulls після створення RMT із checked cleanup/fault/retry. Це Q04/capture параметри, не Test timeout. Старі hardware-latch timestamps/3 мс stuckHIGH не відновлюються.

### Погоджена operational політика

| Умова за справних часу й виходу | Test — звичайна/default збірка | Fly — лише явно обрана збірка |
|---|---|---|
| Немає valid поточного boot | LOW; last-valid відсутній | LOW; утримувати нічого |
| Перша повністю valid, включно з високою | Автоматично транслювати | Автоматично транслювати |
| Немає нових valid; попередня команда є | Hold до віку T, при age≥T вимагати LOW; фізичний LOW не пізніше1 с від last-valid end | Hold last-valid незалежно від тривалості відсутності входу, включно з0/100% |
| Invalid/missing/stuck/capture loss | Last-valid незмінний, Test deadline не переноситься | Discard недостовірних даних; status/history не примушують LOW лише через input loss |
| Нова trustworthy valid після належного resync | Автоматично виконати, включно з high | Автоматично виконати, включно з high |
| Reset будь-якої причини | Prior-boot duty втрачається; LOW до нової valid | Те саме; hold через reboot не обіцяється |
| Output/shared-clock failure | Класи/реакція D70/D71, retry episode D75; контракт§3–5 | Те саме; Fly не гарантує контроль несправного виходу |

Немає zero-handshake, окремого arming,10 додаткових startup frames або ручного clear після input loss (D64/D65). Перша valid має пройти всі чинні capture/period/resync перевірки. USB reconnect не створює команду й не перемикає режим. Звичайна operational збірка05 стає Test; historical normal04 лишається LOW/disarmed baseline. Test/Fly не є finite waveform/stress/accuracy04.

### Уточнення користувача,2026-11-06

- **C05-01 / D66**: максимальна загальна затримка від останнього accepted valid pulse_end_us до фізичного LOW у Test — **1 с (1000000 мкс)**. Користувач окремо доручив Plan обрати timeout T у цій загальній межі; спрацювання при **age≥T**.1 с не дорівнює автоматично T: у загальну межу входять обробка, планування й фізичне застосування. T і розподіл запасу Plan визначає до реалізації, не підставляючи історичні100 мс.
- **C05-02 / D67**: користувач доручив Plan обрати класи technical output/clock failures, реакцію й автоматичний retry з обґрунтуванням. Політика не визначена мовчазним default. Plan має встановити recoverable/terminal класи, запити LOW/fallback/cleanup, строки/ліміти retry, поведінку після вичерпання, умови довіри часу/виходу та нового nonzero. Це не перевибір автоматичного input-loss recovery D64.
- **C05-03 / D68**: погоджено watchdog automatic reset при зависанні критичного capture→control→output циклу; після reboot LOW до першої нової valid-команди. Покриття, числові строки й метод перевірки визначає окремий Plan. Відсутність valid у Fly або нечитання USB самі не є stall і не повинні приховано скасовувати hold.

- **C05-04 / D75 / CHK017**: користувач обрав поновлення бюджету retry лише після checked успішного застосування нової trustworthy post-fence valid-команди, включно з valid0%. Кожний старт reinit витрачає одну з3 додаткових спроб; самого reinit на0% недостатньо. До такого apply усі output failures належать тому самому епізоду; після витрачених3 спроб нова failure → checkedLOW/ExhaustedLow до reboot. Успішна третя reinit може чекати valid й завершитися успішним apply. Sticky history зберігається; input-loss recovery D64/D65 не змінюється. Числа3/100мс не переобрані.

### Відкриті залежності та строки їх закриття

| Питання / власник | Потрібне рішення або доказ | Межа блокування |
|---|---|---|
| Q05 /05 | Plan D69–D73 обрав T500мс/age≥T/загальнийphysicalLOW≤1с, apply20мс, outputretry3/100мс/fence, clock/protocol reset, TWDT100мс/IWDT500мс/ROM hook, rc05/migration/validation | Technical design визначено; actual tests/build/physical not_run. G05 закрито D74, залежні physical gates відкриті |
| G05 / governance | D74: конституціюv2.0.0 та залежні шаблони/настанови синхронізовано під D63–D65/D10 окремим явним Constitution-завданням | Закрито2026-11-06; не є реалізацією або фізичним прийняттям |
| Q02/H02/H03 /01 | Реальні живлення/bias/startup waveform/прилад; USB+external power не прийняті | До залежних підключень/живлення й фізичного прийняття, без блокування опису незалежного ядра |
| Q03 /03 та джерело05 | Conditioning/ground/protection/рівні й timing/loss; фактичні Matek H743-Wing revision/output/PWM config/ArduPilot version | До використання джерела. Конкретний receiver/radio-loss приймається до його використання; HJ-only цього не доводить і від receiver gate не залежить |
| Q06 /04,06 | Actual MD20A revision/interface/DIR–GND без насоса; силовий тракт, незалежний emergency cutoff, напрямок/струми/тепло06 | До powered-driver tests; перед навантаженням — accepted05 та перевірений стенд06 |
| Q04 /02,03 | Програмні параметри вибрані; accuracy/loss/runtime бюджети фізично не прийняті | Зберегти застосовні регресійні критерії, без переобрання параметрів |
| Q07–Q09 /07–09 | Відкладені розширення | Не блокують05 |

### Governance та межі Specify

На момент Specify/Plan принципII конституціїv1.0.0 вимагав «LOW before explicit arming» та універсального missing/invalid fail-off, тому D64/D65 були явно записані як user overrides, а G05 блокував affected implementation. Старий текст не оголошено сумісним заднім числом. Окреме явне Constitution-завдання2026-11-06 ухвалилоv2.0.0/D74, синхронізувало шаблони/настанови й закрилоG05. Міграція02–04 latch/observer/terminal adapter та перевірки лишаються запланованою реалізацією, не результатом поправки.

Гілку05 створено штатним create-new-feature.sh із явно заданим SPECIFY_FEATURE_DIRECTORY=specs/05-failsafe-arming. FEATURE_DIR і SPEC_FILE повернули абсолютні шляхи саме цієї ітерації; .specify/feature.json вказує на05. D10/AGENTS.md і пряме поточне доручення є явним винятком до старої заборони branch mutation принципуV та шаблону. Він не дозволяє commit/push/device access/flash/load. Виконано лише Specify та його quality checklist. Plan/Tasks/Implement, hardware дії й публікація Git не виконуються; prompts та попередні артефакти/докази зберігаються.

## Сценарії користувача й перевірки

### US1 — Прозора команда після startup/reset (P1)

Оператор отримує LOW до першої повністю valid поточного boot, потім команду без додаткового дозволу. Це основа конвертора та передумова решти історій.

**Незалежна перевірка**: host модель команд/часу, окремий build/config review і фізичний сигнал без насоса після electrical gates.

1. **US1.AC1**: Дано новий boot Test/Fly без valid; коли вхід відсутній, invalid, початковоHIGH або unknown period; тоді LOW, last-valid відсутній, prior-boot duty не поновлюється.
2. **US1.AC2**: Дано справні час/вихід і належний resync; коли прийнято першу fully-valid2000 мкс; тоді автоматично запитується100%, без попереднього0%, arming або додаткових10кадрів. Шість nominal points дають погоджену карту.
3. **US1.AC3**: Дано held80/100%; коли стається cold/manual/software/brownout/watchdog reset або bootloader/upload→новий boot; тоді стара команда втрачена, LOW до нової valid цього boot. Інтервал до firmware покриває окремо перевірений апаратний bias.
4. **US1.AC4**: Дано ordinary build без явного Fly; коли перевірено effective config/build identity і поведінку; тоді це Test. Явна Fly збірка ідентифікується як Fly; runtime/USB/persistent mode відсутні, finite04 не підміняє operational05.

### US2 — Test відсікає несвіжу команду й відновлюється (P1)

Оператор Test має передбачувану відсічку при відсутності valid та автоматичне повернення актуальної команди.

**Незалежна перевірка**: керований час на T−1/T/T+1 мкс і1 с, затримки обробки/виходу; окреме пряме фізичне last-valid-end→LOW без насоса.

1. **US2.AC1**: Дано valid80% з end=t_v; коли немає нових valid й вік менший T; тоді hold80%. Рівно при age=T і після нього вимагається LOW; фізичний сталий LOW має настати не пізніше t_v+1000000 мкс. T і технічний запас задає Plan за D66.
2. **US2.AC2**: Дано valid80%, потім missing, invalid-width, invalid-period, incomplete, stuckHIGH/LOW або capture overflow окремими сценаріями; коли нових valid немає; тоді t_v не змінюється, deadline не переноситься на fault/worker/print/retry. Invalid активність не утримує Test нескінченно.
3. **US2.AC3**: Дано cutoff і належний capture resync; коли прийнято нову trustworthy valid2000 мкс; тоді автоматично100%, без zero-handshake/manual clear; sticky history зберігає факт втрати.
4. **US2.AC4**: Дано late queued valid; коли її end вже прострочений для Test або вона pre-barrier/old; тоді доставка не створює свіжої ненульової команди. Прострочення записано фактичним lateness, не backdated pass.
5. **US2.AC5**: Дано одночасні deadline/new-valid/output events; коли оцінюється вихід; тоді враховується вже прийнятий актуальний last-valid, stale apply не скасовує пріоритетне рішення LOW. Plan однозначно задає точку прийняття/серіалізації та правила гонок до реалізації.

### US3 — Fly утримує й приймає нову команду після resync (P1)

Оператор явно обраного Fly очікує hold при input loss без прихованого Test cutoff.

**Незалежна перевірка**: host великий вік last-valid і faults/resync, окремі тривалі фізичні hold/recovery записи без насоса.

1. **US3.AC1**: Дано last-valid0/25/80/100%; коли missing/invalid/stuck тривають довше T,1 с та capture100+10 мс; тоді за справних часу/виходу held duty не змінюється. Status/counters фіксують втрату;0/100% лишаються сталими endpoints.
2. **US3.AC2**: Дано hold80% і malformed/overflow із втратою меж; коли capture відновлюється; тоді пошкоджені/старі samples відкинуто, потрібні два нові trustworthy rising; resync сам не змінює duty. Наступна fully-valid2000 мкс автоматично дає100%, нова valid1000 мкс дає0%.
3. **US3.AC3**: Дано Fly без valid поточного boot; коли проходить довгий час або завершується resync; тоді LOW до прийнятої valid. Reset втрачає стару held-команду.
4. **US3.AC4**: Дано hold100%; коли реально відмовляє output/shared clock або зависає критичний цикл; тоді діє окрема D67/D68 technical policy, відмова видима. Fly не заявляє справний вихід/hold через reboot.

### US4 — Технічна відмова й watchdog спостережувані (P1)

Оператор відрізняє запит LOW від успішного застосування та фізичного LOW; відновлення не відтворює stale command.

**Незалежна перевірка**: host/adapter failure injection, окремі no-pump stall/watchdog/clock/output вимірювання за майбутньою затвердженою процедурою.

1. **US4.AC1**: Дано setup/partial setup/apply failure на LOW↔PWM↔HIGH; коли виконуються погоджені failure/cleanup/retry; тоді nonzero не виникає до виконання обраних Plan умов відновлення. Старий request не replay; кожна невдача видима навіть після успішного fallback.
2. **US4.AC2**: Дано невдалий stopLOW та невдалий повний fallback; коли формується результат; тоді LOW непідтверджений, physical cutoff не заявлено, перевірка fail. API success теж не замінює вимірювання напруги.
3. **US4.AC3**: Дано unavailable/negative/regressive/overflow clock; коли trustworthy age неможливий; тоді час не виправляється нулем/callback timestamp/unsigned wrap, false valid не створюється. Plan визначає technical reaction/recovery за D67 й розрізняє старий sample та несправний shared clock.
4. **US4.AC4**: Дано deliberate stall критичного циклу при nonzero; коли досягнуто обраної watchdog межі; тоді automatic reset, після reboot LOW до першої нової fully-valid, включно з високою. Окремо виміряно stall→reset і stall→physicalLOW за критеріями Plan. Жива diagnostics task не маскує stalled control.
5. **US4.AC5**: Дано справний Fly цикл без RC/USB reader; коли control/hold продовжують виконуватися; тоді сама відсутність valid/читача не запускає watchdog reset. Reset не використовується як прихований signal-loss cutoff.

### US5 — Актуальна діагностика без впливу на керування (P2)

Оператор бачить різницю між станом входу, розрахунком, effective request і apply result навіть під максимальним diagnostic load.

**Незалежна перевірка**: snapshot/transport boundary fixtures; фізичні max-load/slow/absent reader/reconnect записи.

1. **US5.AC1**: Дано Test cutoff або Fly hold після loss; коли переглядається snapshot; тоді видно mode/build/boot, input validity/status/resync, last-valid/end/age-validity, calculated duty, effective request/permission, apply status, current technical failure і sticky history/counters. Test calculated80 може співіснувати з effective0; Fly input-fault history — з effective80.
2. **US5.AC2**: Дано maximum fields/counters, busy/zero/partial write/concurrent publish; коли writer працює; тоді≤384 B/frame, spacing≥100 мс/≤10записів/с, correct drop/overwrite/version ack. Один committed record може завершитися після reconnect; далі актуальний стан без pending history/burst.
3. **US5.AC3**: Дано max diagnostics з active/slow/absent reader/reconnect; коли повторено US1–US4 і застосовну03/04 регресію; тоді capture/Test deadline/Fly hold/recovery/watchdog не очікують transport/formatting. Drop не стирає історію помилок й не створює команду.
4. **US5.AC4**: Дано брак методу/приладу/доказу; коли сформовано звіт; тоді blocked/not_run з власником і наступним доказом. Host/build не стають physical pass; специфікація не дозволяє роботу насоса 06.

### Граничні випадки

- Width899/900/2100/2101; period17999/18000/22000/22001 мкс;1004/1005/1006→0/1/1%,1994/1995/1996→99/100/100%; unknown period/startupHIGH без rising.
- T−1/T/T+1 мкс; загальна межа1 с безпосередньо до/рівно/після; late worker/output, repeated same-time polls, invalid edges без freshness, no prior-valid.
- Epoch/seq wrap у bounded window, queue saturation/drop, association ambiguity, pre/post-barrier arrival, regressive end, duplicate/old sample; коректно розширений rollover не fault сам по собі.
- StuckHIGH/LOW поріг±1 мкс, ended episode до monitor, masking/re-enable;100+10 мс input accounting і112 мс M04 не стають T або budget05.
- Fly довгий hold0/100/intermediate; high reconnect після resync; reset без відновлення prior duty; числове переповнення віку/лічильника не створює команду/режим.
- Fault між перевіркою й apply/між складовими apply; stale nonzero після LOW/fallback/reinit; повторна невдача не перебазовує початок reaction budget.
- Watchdog heldHIGH/stalled control за живої diagnostics task; no-reader/no-valid Fly не stall. Hardware bias, softwareLOW, watchdog і emergency power cutoff різні.
- Actual ArduPilot/receiver може продовжувати valid PWM після radio loss: конвертор не визначає здоров'я радіолінії з freshness. Джерело/його failsafe приймається окремо до використання.

## Вимоги

### Функціональні вимоги

- **FR-001 (R01,R03,R10)**: Розширити наявні02/03/04 у межах одного локального каналу, переносимої логіки й погодженого профілю; public core без SDK/GPIO/FreeRTOS, без нових pins/мережі/MCU DIR.
- **FR-002 (R07,R11)**: Окремі immutable build configurations Test/Fly; ordinary build — Test, Fly явно; build identity та diagnostics однозначні. Runtime/USB/persistent switch відсутні; finite04 зберігається окремо.
- **FR-003 (R07,R08)**: Кожний boot/reset починає без prior-valid/held duty з LOW; перша fully-valid поточного boot автоматично транслюється, включно з high, без zero-handshake/arming/extra10frames/manual input-loss clear.
- **FR-004 (R04,R05,R06)**: Зберегти width/period/association-before-clamp, карту/rounding без фільтрації/learning,1000 Гц/staticLOW/staticHIGH. Invalid утримує calculated last-valid без оновлення часу; без previous valid значення відсутнє.
- **FR-005 (R05,R07,R09)**: Freshness походить від accepted pulse_end_us у shared monotonic domain з урахуванням processing/output latency; old/pre-barrier/ambiguous/time-order rejected samples не є новою operational командою й не перемарковуються now.
- **FR-006 (R07,R08,R09)**: Test hold до age<T, при age≥T вимагає сталий LOW; за відсутності нових valid загальний last-valid-end→physicalLOW≤1000000 мкс (D66). Invalid/missing/stuck/capture loss не перезапускають deadline. D69 визначив T=500000 мкс і технічний запас у Plan; deadline miss видимий, без backdating.
- **FR-007 (R07,R09)**: Fly за справних часу/виходу hold last-valid незалежно від тривалості input loss, включно з0/100%; signal/capture status/history не створюють прихованого cutoff. No-prior-valid — LOW; hold через output failure/reboot не гарантується.
- **FR-008 (R05,R07,R08)**: Обидва режими автоматично виконують наступну нову trustworthy valid після належного resync, включно з high. Зберегти discard/epoch/time-order/two-rising boundaries; model-only clear_fault не є production recovery.
- **FR-009 (R05,R07,R09,R11)**: Зберегти stuck100+10 мс, rearm/processing/guard03 і loss accounting. Розділити current capture status, sticky history, technical fault та output inhibition; явно мігрувати latch02/03, observer і terminal adapter04. Capture-loss recovery за справних часу/виходу не підмінюється output-failure policy.
- **FR-010 (R06,R08,R09,R11)**: Один узгоджений output owner; malformed duty/stale requests відхиляються. Checked apply/LOW/fallback/cleanup та failures видимі, LOW failure непідтверджений. D70/D75 і контракт§3–4 визначають класи/реакції,3 додаткові reinit спроби на епізод з інтервалом≥100 мс, поновлення бюджету лише після checked new post-fence valid apply (включно з0%), exhaustionLOW і post-recovery valid fence; stale nonzero не replay після recovery.
- **FR-011 (R05,R08,R11)**: Invalid/unavailable/negative/regressive/overflow clock не створює false timestamp/freshness/unsigned wrap. D70/D71 і контракт§3,§5 визначають LOW/reset для fatal shared-clock failure та discard/resync для старого measurement за справного clock; невідомий age так і позначений.
- **FR-012 (R08,R09,R11)**: Watchdog виконує automatic reset stalled критичного capture/control/output циклу (D68); coverage/feed/trigger/строки/метод визначені D71, контрактом§4–5 і quickstart P7; фактична перевірка ще not_run. Diagnostics activity не підміняє progress; no-valid Fly/no-reader самі не stall. D69/D71 узгоджують stall/watchdog/LOW allocations із загальною Test межею1 с: окремий watchdog timeout не є дозволом перевищити її. Якщо потрібну реакцію довести неможливо, відповідне прийняття blocked/fail, без заяви cutoff. Після reboot FR-003; watchdog не є незалежним power cutoff.
- **FR-013 (R09,R11)**: Diagnostics показує поля US5.AC1, зберігає latest-only/drop/counters,≤384 B/frame,≥100 мс/≤10записів/с, максимум один committed/in-flight record і no-command semantics03. Не блокує режими/recovery/watchdog, не накопичує production per-pulse history. Frozen export лише explicit bench evidence.
- **FR-014 (R01,R08,R12)**: Physical transitions05 приймаються без насоса після застосовних Q02/Q03/Q06 gates: Test last-valid→LOW, Fly hold/recovery, startup/reset/stall/watchdog. Actual Matek/ArduPilot/receiver levels/config/radio-loss приймаються до використання; HJ-only не доводить receiver suitability.
- **FR-015 (R04,R05,R06,R09,R12)**: До вимірювань задати бюджети05, допуски/тривалості/повтори/U/придатність приладу/source matching; зберегти застосовні D53–D55/M04 регресії. MCU API timings і analyzer physical intervals окремі; невідомий метод blocked, перевищення межі fail без послаблення заднім числом.
- **FR-016 (R12)**: Meaningful host/adapter тести поведінки/меж/відмов/recovery, окремі pinned target builds/config і physical records; кожний R/FR пов'язаний із US/SC/evidence або owner/blocker. Історичні докази не переписуються як05; test-first не обов'язковий.
- **FR-017 (R07,R08,R12)**: Дотримуватися принципівII/V конституціїv2.0.0/D74, що формалізували D64/D65 та D10; зберегти історію попередніх overrides, technical selections D69–D73 і статус закритогоG05. Поправка не є реалізацією й не запускає інші фази/Git publication/hardware.

### Ключові сутності

- **Valid-команда поточного boot**: width/period, calculated duty, accepted pulse_end_us, boot/epoch/seq provenance. Відсутня команда відрізняється від valid0%.
- **Build mode**: immutable Test або явно Fly з build identity; окремий від finite bench scenario04.
- **Freshness/deadline**: t_v, validity age/clock, T, порігage≥T і загальна межа1 с Test. Fly signal-loss cutoff deadline відсутній.
- **Input status/history**: current valid/invalid/incomplete/stuck/unavailable/resync/barrier окремо від sticky counters/reasons; не тотожні output/shared-clock failure.
- **Control/output state**: очікування першої valid, трансляція, hold, Test timeoutLOW, technical fault/recovery. Calculated duty, effective request/permission, API-applied status та physical voltage різні факти.
- **Failure episode**: class/reason, first failure і last failure completion/validity, витрачені reinit starts0..3, LOW/fallback/cleanup/retry outcomes; D75 закриває епізод і поновлює бюджет лише checked new post-fence valid apply, включно з valid0%. Reinit0 не закриває епізод; sticky failure history зберігається.
- **Evidence**: R/FR/US/SC, build/boot/source identity, метод/числові межі/U, expected/actual і pass/fail/blocked/not_run; категорії host/build/physical розділені.

## Критерії успіху

### Вимірювані результати

- **SC-001**: Обидва режими проходять шість nominal points і погоджені width/period/rounding/association boundaries; жодний invalid/old/ambiguous не створює valid100% або новий t_v. Переносима межа перевіряється без MCU.
- **SC-002**: Ordinary/default та explicitTest мають Test поведінку, explicitFly — Fly. У всіх startup/reset fixtures prior-boot duty не поновлюється; перша fully-valid high виконується без zero-handshake/arming/додаткових10кадрів. Результат підтверджений окремими build/config і no-pump scenarios.
- **SC-003**: Test на T−1/T/T+1 мкс відповідає age≥T; за відсутності нових valid фізичний сталий LOW не пізніше1 с від останнього accepted end, включно з processing/output latency, при missing/invalid/stuck/capture loss і max diagnostics. Boundary fixtures також перевіряють1 с−/0/+; T=500000 мкс та внутрішні budgets визначені D69/контрактом§4; фактичні докази not_run.
- **SC-004**: Fly утримує0/intermediate/100% під час усіх заявлених тривалих input-loss runs, включно з перетином capture100+10 мс, T і1 с, за справних часу/виходу. Обидва режими автоматично виконують new-high після належного resync без permanent signal-loss lock. D69/D73 задають≥600 фізичних секунд loss для0/80/100% та active/slow/absent reader, new-valid→physical output≤20 мс; фізична нескінченність не заявляється.
- **SC-005**: Кожний обраний D67 technical failure/retry/cleanup case відповідає затвердженим reaction/recovery limits; deliberate stalled cycle дає watchdog reset за D68 у визначений Plan строк, після boot LOW до нової fully-valid. Невдалий LOW не зараховується як physical cutoff. До SC-005 входять епізоди D75: reinit0-success/apply-failure не поновлює count; третій reinit success→valid0/high apply success закриває епізод; failure після третього reinit без такого success→ExhaustedLow; наступна незалежна failure після закриття має новий бюджет3. Обрані D70/D71/D75 критерії ще не виконані; числова таблиця — контракт§4, coverage/метод — quickstart P6/P7.
- **SC-006**: SC-001–SC-005 та застосовні capture/output regressions виконані при max diagnostics/slow/absent reader/reconnect без очікування USB;≤384 B/frame, spacing≥100 мс/≤10записів/с. Capture evidence зберігає D53–D55: кожний correct pulse коротких RAM-вікон E_width≤5 мкс, відповідний rising interval E_interval≤10 мкс, U≤1 мкс окремо без E+U/averaging; окремі600 с zero-loss/timing runs із доведеним фізичним покриттям.
- **SC-007**: Physical intermediate duty відповідають1000 Гц/M04 waveform/voltage,0/100% сталі LOW/HIGH на GPIO0 та вході accepted MD20A. Test cutoff означає початок безперервного LOW до наступної навмисної valid-команди у заданому observation interval, не LOW частину PWM. Fly hold/recovery виміряні окремо; static endpoints потребують initial levels/gapless evidence, не лише порожнього CSV.
- **SC-008**: Для100% R/FR є US/SC та власний evidence/status. Host/build/config/physical05 розділені; full acceptance05 і готовність до насосного прийняття 06 не заявляються за невиконаних обов'язкових критеріїв/gates. Специфікація не є firmware або вимірюванням.

### Простежуваність прийняття й докази

Наведені verification шляхи плановані всередині05; результатів у Specify немає, файли evidence до виконання перевірок не створюються. H=host, B=target build/config, P=physical без насоса. Усі перевірки05 not_run; gates позначають залежне blocked.

| Вимога R/FR | Сценарій / SC | Метод | Майбутній доказ / стан |
|---|---|---|---|
| R01,R03,R10 /FR-001 | US1.AC4,US5.AC4 /SC-001,002,008 | H public core isolation; B profile/graph/startup/no radio/no DIR | verification/host.md, verification/build.md; not_run |
| R07,R11 /FR-002 | US1.AC4 /SC-002 | B ordinary/default/explicitTest/explicitFly identity; H/P corresponding behavior | verification/build.md, verification/host.md, verification/hardware.md; not_run |
| R07,R08 /FR-003 | US1.AC1–3,US3.AC3 /SC-002 | H startup/high-first/reset; P startup/reset/bootloader/upload/brownout/watchdog | verification/host.md, verification/hardware.md; physical gates відкриті |
| R04,R05,R06 /FR-004 | US1.AC2,US2.AC2,US3.AC1–2 /SC-001,007 | H map/validation/rounding; P M04 waveform/endpoints/voltage | verification/host.md, verification/hardware.md; not_run |
| R05,R07,R09 /FR-005,006 | US2.AC1–5 /SC-001,003 | H T±1 мкс/exact/late/old/invalid/1 с±; P accepted-end→LOW≤1 с | verification/host.md, verification/hardware.md; D69/контракт§4; not_run |
| R07,R09 /FR-007 | US3.AC1,3–4 /SC-004,007 | H довільно великий age/0/100/stuck; P long Fly hold | verification/host.md, verification/hardware.md; D73/quickstart P4; not_run |
| R05,R07,R08 /FR-008 | US2.AC3–5,US3.AC2 /SC-001,004 | H barrier/epoch/two-rising/new-high/rejectold; P Test/Fly recovery-high | verification/host.md, verification/hardware.md; not_run |
| R05,R07,R09,R11 /FR-009 | US2.AC2,US3.AC1–2,US5.AC1 /SC-004,006 | H stuck±1 мкс/ended/masked/overflow/history; P100+10 мс та режимна реакція | verification/host.md, verification/hardware.md; not_run |
| R06,R08,R09,R11 /FR-010 | US4.AC1–2 /SC-005 | H actual adapter boundary/failure/fallback/cleanup/retry/stale; P reaction | verification/host.md, verification/hardware.md; D70/контракт§3–4; not_run |
| R05,R08,R11 /FR-011 | US4.AC3 /SC-005 | H clock invalid/regression/overflow vs old sample; B/P selected fault cases | verification/host.md, verification/build.md, verification/hardware.md; D70/D71; not_run |
| R08,R09,R11 /FR-012 | US1.AC3,US4.AC4–5 /SC-002,005 | H progress/reset; B watchdog config; P stall→reset/LOW/new-high після boot | verification/host.md, verification/build.md, verification/hardware.md; D71/quickstart P7; not_run |
| R09,R11 /FR-013 | US5.AC1–3 /SC-006 | H payload/drop/versionack/current-history; P reader/maxload | verification/host.md, verification/hardware.md; not_run |
| R01,R08,R12 /FR-014 | US1.AC3,US2.AC1,US3,US4.AC4,US5.AC4 /SC-003–008 | P readiness/no-pump/source/interface/Test/Fly/watchdog, receiver до use | verification/hardware.md, verification/acceptance.md; Q02/Q03/Q06 blocked/not_run |
| R04,R05,R06,R09,R12 /FR-015 | US2.AC1,US3.AC1–2,US5.AC3–4 /SC-003–008 | Review limits/U/anchors; P D53–D55/M04 affected regressions | verification/hardware.md; D69–D73; readiness/U перед run, not_run |
| R12 /FR-016 | US5.AC4 /SC-008 | H/B/P окремо; commands/config/hashes/expected-actual/negative cases | verification/host.md, verification/build.md, verification/hardware.md, verification/acceptance.md; not_run |
| R07,R08,R12 /FR-017 | US5.AC4 /SC-008 | Review user overrides/branch exception/technical selections/formal amendment | Ця spec, checklists/requirements.md, D74/конституціяv2.0.0; G05 закрито |

Фізичний Test interval вимірюється від falling однозначно зіставленої останньої accepted valid до початку сталого LOW. Provenance останньої valid, U, initial levels і gaplessness обов'язкові; інакше blocked. MCU API brackets та analyzer waveform time різні докази: offset не калібрується з самих PWM transitions за припущеною затримкою; USB timestamps не є фізичним end. Загальна1 с не доводиться лише API success.

Застосовні M04 межі зберігаються: per-cycle E_f≤1 Гц/U_f≤0,1 Гц, E_d≤0,1відсоткового пункту/U_d≤0,02; LOW+U≤0,8 В, HIGH−U≥2,4 В/HIGH+U≤3,6 В, U_V≤0,05 В; виявлення протилежного рівня≥1 мкс за придатної методики. Наявність цих меж у контракті не є physical pass. Startup/reset/bootloader/upload наслідують застосовний M04 observation/repetition scope; нові watchdog/brownout сценарії05 методично визначає Plan до run.

D55 короткі128-record RAM accuracy windows не доводять600 с loss/timing. D62 stress04 має601MCU-секунду й окремий lower-bound доказ≥600фізичних секунд; finite04 API1/2 мс,112 мс stuck→LOW та65/605 с sequence не стають operational05 budgets або Fly cutoff. Production05 не успадковує bench archive history; affected instrumentation needs окремий bounded evidence design до run. Жодні unrelated hardware tests не повторюються лише через створення документа.

## Припущення

- Intended source — ArduPilot/Matek H743-Wing (D63); actual revision/contact/config/version/електрична придатність ще не прийняті. HJ50 Гц має власні незалежні gates; receiver/radio-loss до його використання обов'язкові.
- Насос від'єднаний у всіх майбутніх tests05; фактичний стан стенда в цій документальній сесії не перевірявся. Firmware не визначає ізоляцію;06 потребує accepted05 і перевіреного силового стенда.
- Прилад/conditioning/bias/power suitability/fault injection можливості не припускаються; до кроку власник фіксує readiness/blocked reason. Відсутні докази не скасовують критерії прийняття.
- Код/контракти й environment01–04 перевикористовуються. Майбутній Plan має описати міграцію historical inhibition та production transitions, а не лише виклик model clear або стирання всіх status flags.
- D66–D68 зафіксували делегування технічного вибору окремому Plan. Це делегування вже виконано D69–D73 із обґрунтуванням і шістьма Constitution Check gates до research/після design; тести/реалізація/фізичне прийняття залишаються окремими фазами.
- Hardware bias, softwareLOW, watchdog і незалежний emergency power cutoff — різні відповідальності. Low-side MOSFET flyback-схема не переноситься на H-bridge. Нуль витрати насоса й сумісність невипробуваних плат не заявляються.


## Технічне доповнення Plan,2026-11-06

Цей розділ фіксує виконане делегування D66–D68, не змінюючи продуктову політику D63–D65. Датовані відповіді користувача/Specify records зберігають історію делегування; поточні FR/SC і матриця простежуваності узгоджені з Plan. Актуальні technical selections — D69–D73 у [реєстрі](../../docs/decisions.md), [plan](plan.md), [research](research.md), [embedded контракті](contracts/operational-control.md), [моделі](data-model.md) та [quickstart](quickstart.md).

- **D69**: Test T=500000мкс, age≥T; межі499999/500000/500001мкс. Загальна physical last-valid-end→LOW≤1с незмінна. Нормальний end→checkedAPI≤19мс/directphysicalstate≤20мс; healthy deadline→checkedLOW≤2мс. Строки resetpath350мс+100мс разом ізT дають950мс design allocation; кожна physical метрика потребує доказу, не source-only arithmetic. Maximum latency приймається заL+U≤limit,U_latency≤1мс, без зміни D54/M04 directE/U.
- **D70**: Input loss/errors не output-terminal і не обхід Fly. Recoverable output failure→checkedLOW/fallback, потім до3 додаткових reinit спроб/≥100мс від failure completion. Успіх reinit0→LOW до нової valid end>recovered_at; exhaustedconfirmedLOW→inhibit доreboot без appautorestart. Runtime fatalclock/protocol/LowUnconfirmed за armed watchdog→best-effortLOW/ResetPending/nofeed. Pre-registration watchdog init/add failure→SetupBlockedLOW/nohandoff без недоведеного reset; post-handoff feed API error→explicit emergency ROM reset післяbest-effortLOW. Повна таблиця/стадії/1мсSDKcall/20мсattempt у контракті; цифри — вимоги до майбутньої перевірки.
- **D71**: RuntimeTWDT100мс, тількиtask subscription owner, qualifiedfeed; IWDT500мс. EmergencyIRAMhook→ROMsystemreset доstockpanic, бездругогоGPIOwriter; actual map/IRAM/log0 closure йreset/bias фізичні докази обов'язкові. Feed приTestnonzero тількидоT, на/післяT післяLOW; missingRCFly/noUSB не stall. Freezeclock неfeed. Reset стираєheldduty йможе матиsoftware rawreason, не обіцяє watchdoghistory наступногоboot.
- **D72**: Один extendedcore/CapturePolicy/owner; accepted-end таservice-now окремі, candidatecommit післяprocessingcheck; operational currentinput/history/technical inhibition розділені, legacy02/03/finite04 збережені. Persistentowner/monitor vsretryRMT/GPIO, ordinarydefaultTest/Flyexplicit, separatelegacyflags. rc05v1 grammar≤347B/384B padded іlatest-only transport.
- **D73**: Explicitbench evidence позаproduction/finite04, longFlyloss≥600physс у0/80/100% ізreaderconditions іrecoveryhigh/low; captureaccuracy/600с окремо заD53–D55. Recorderoverflow не cutoff Fly; actualpayload/RAM/stack/physicalinputs потребуютьперевірки.

Для FR-006/SC-003 T уже500мс; для FR-010–012/SC-004–005 обрані technical reactions/limits наведено вище, не залишено мовчазних defaults. US4.AC4 reset/LOW partialmetrics вимагають independent reset anchor ізвідомою похибкою; безнього blocked, навіть коли direct1с waveform придатний. Наявністьsource/design неclaimpass. G05 principleII/template synchronization закрито D74; Q02/Q03/Q06 лишаються physical gates. У Plan змінено лише документацію; Tasks/Implement/Gitpublication/device/flash/load не виконано.


## Конституційна синхронізація2026-11-06

Окремим прямим завданням користувача ухвалено [конституціюv2.0.0](../../.specify/memory/constitution.md), D74. G05 закрито: Test/Fly/startup/automatic recovery й Specify-only D10 формально включено до принципів, шаблони spec/plan/tasks та поточні настанови синхронізовано. Це оновлення governance status, а не повторний Specify/Plan/Implement. Числа/механізми D66–D73, user scenarios, validation contracts і методи прийняття не змінено. Жодні firmware/host/build/physical результати не додано, Q02/Q03/Q06 не прийнято; наступні фази — окреме доручення.
