# Модель даних 05

Дата:2026-11-06. Це дизайн, не наявна реалізація. Одиниці — мкс/цілі відсотки; public types strict C++11 без SDK/RTOS/GPIO/I/O. Одним екземпляром ControlCore володіє наявний CapturePolicy у rc-control owner. Нормативні переходи — [контракт](contracts/operational-control.md).

## Незмінна конфігурація

`ControlConfig`: core mode=`LegacyDisabled|Test|Fly`; policy_version=1; Test timeout_us=500000, valid_apply_limit_us=19000, physical_limit_us=1000000. Значення immutable, не зберігаються в NVS. Default core constructor — LegacyDisabled; ordinary app явно передає Test. Build family та output role є окремими composition-ознаками, не додатковими режимами ядра:

| Build family / image | Core mode | Output role |
|---|---|---|
| LegacyDisabled, включно з explicit rc03 diagnostics/accuracy | LegacyDisabled | LegacyDisabled |
| FiniteBench04 | LegacyDisabled | FiniteBench04 |
| Operational05, включно з explicit05 evidence | Test або явно Fly | Operational05 |

Finite04 використовує попередні OutputPolicy/BenchConfig, не operational deadline. Поєднання поза таблицею відхиляються під час configure; runtime перемикання осей немає.

`BuildIdentity`: full source/config/profile/SDK hashes у manifest; runtime fingerprint64, boot/session identity64. Boot ID informational, не security token/доказ нового живлення; жодна prior-boot command не завантажується. Перевірки доказів використовують повний manifest і фактичний reset/boot anchor, а не тільки 64bit ID.

## AcceptedMeasurement

Поля: width_us32, start_us64/end_us64, period_us32, capture_epoch32, edge_seq32, complete/period_known/association_verified, status. Validation: width900..2100,period18000..22000 inclusive, matched epoch/window, sane порядок часу, не pre-barrier, не duplicate. У operational accepted end має бути новішим за попередню valid; однаковий token/end — не нова команда. Legacy nondecreasing-time контракт 02 з рівністю лишається legacy.

Обробка candidate двофазна: parse/association/width/period → завершення processing-budget check → commit valid. До commit немає update freshness/calculated/output. ProcessingLate candidate не може коротко вмикати вихід перед повідомленням помилки. RMT width не замінюється end−start.

## ControlSnapshot05

- has_valid; last_valid_end_us64, last_width_us32/calculated_percent8 (окремі поля, validity через has_valid), last accepted epoch/seq; calculated відсутній при has_valid=false, а не вигадані 0%.
- mode; phase=`AwaitValid|Active|TestTimedOut|TechnicalRetry|AwaitPostRecoveryValid|TechnicalExhausted|ResetPending`.
- now_valid/age_valid, now_us64 і age_us64; за invalid clock час збережений тільки як last-trusted, не поточний. Age лише checked now−last_valid_end; Fly може мати великий age без cutoff.
- effective_percent8/work_permitted; request generation64/output_epoch64. Valid0% може мати permission=true, AwaitValid/timeoutLOW — false; це різні стани.
- current_capture_bits16/history_capture_bits16, capture_phase/current trustworthy availability. Stuck current bit завершується з достовірною зміною рівня/новим observation episode; loss/shape/RX current bits позначають resync/очікування нового trustworthy valid та очищуються після його повного post-barrier commit. History sticky до нового boot; поточна input-loss ознака не technical inhibit.
- current_technical_bits16/history_technical_bits16 (current очищується після успішного reinit, history лишається; AwaitFresh fence окремий), last_sdk_status32/operation8, retry_count8, recovery_episode_active, first_failure_at_us/validity, last_failure_completed_us/validity. D75: retry_count=кількість розпочатих reinit у поточному епізоді (0..3); reinit0 не закриває епізод і не скидає count. First failure зберігається до закриття епізоду; last failure completion задає лише spacing наступного retry, не новий бюджет. Відсутність current technical bits не означає завершення recovery episode.
- applied=`Unknown|Low|Pwm|High` — лише checked API fact; physical voltage не міститься у live snapshot без приладу.

`Active` у Fly з input unavailable означає hold; у Test до T означає hold. Діагностика розрізняє це mode+age+capture status, додатковий неоднозначний freshness timeout Fly не вводиться.

## OutputHealth та recovery epoch

Фізичні stages: low_primed, timer_configured, channel_configured, pad_route=`GPIO|LEDC|Unknown`; health=`Starting|Ready|LowConfirmedRetry|AwaitFresh|ExhaustedLow|LowUnconfirmed|ResetPending`. Відомий stage змінюється лише за checked success; помилка cleanup не стирає ownership.

Після technical failure generation інвалідовано, epoch збільшено. Successful reinit0/checked stop0 фіксує recovered_at_us і ще одну нову epoch. Nonzero дозволений тільки після accepted valid end>recovered_at з поточного trustworthy capture epoch/barrier. Valid, отримана під retry, може оновити calculation/history, але не обходить post-recovery fence. Після чистого startup окремої technical fence до першої валідної команди не додається: нормальна startup-послідовність має LOW і починає нове спостереження/ingress після Ready.

D75: recovery_episode_active очищується і retry_count скидається до0 лише після checked успішного застосування нової post-fence valid-команди, включно з valid0%, у її актуальних output epoch/generation. Самі reinit0, valid commit, Test timeoutLOW/fallback не закривають епізод. Apply failure до закриття використовує залишок того самого бюджету; якщо витрачено3 спроби — ExhaustedLow до reboot після checkedLOW. Успішна третя reinit лишається AwaitFresh і може завершити епізод успішним apply; історія відмов не стирається. Новий boot починає без епізоду/held duty, count0, LOW до нової valid.

SetupBlockedLOW — результат startup до handoff, не додатковий wire controlphase: AwaitValid/permissionfalse, currenttechnicalbit64 і startup diagnostic outcome; owner progress відсутній, watchdog reset не обіцяється без subscription.

## Час/лічильники та довга робота

Raw clock signed64 перевіряється до unsigned cast в усіх owner/ISR/SDK leaves. Негативне/регресія shared-now, неможлива арифметика → FatalClock, не підстановка now=0. Однаковий raw у межах короткої операції допустимий; watchdog feed вимагає now>last_feed_raw. Frozen raw або припинення notify не дає feed й веде до watchdog reset. Зміна частоти/невидимий oscillator fault не оголошується універсально виявлюваною: timebase accuracy і 1 с перевіряються фізично.

Порівняння elapsed через subtraction після order check; retry/deadline addition із overflow guard. Shared hardware-clock rollover/regression не продовжується вигаданим часом: fatal→LOW/reset, без відновлення duty після reboot. Немає periodic uptime reset. Seq/epoch32 лишають modulo2^32 з доведеним bounded span<2^31. FreeRTOS tick wrap не використовується як command freshness; у tests перевірити rollover helper bounded tick intervals, long uptime й age>UINT32_MAX. Request/output epochs64 не обертаються: overflow protocol-fatal. Statistics saturate зі sticky flag, але сама діагностична saturation не cutoff Fly; association-losing capture saturation означає input fault/resync.

## Нагляд і ресурси

ProgressCheckpoint: progress_seq64, trusted_raw64, core evaluated/request generation, output-known/LOW-status, checked watchdog feed outcome. Checkpoint тільки після bounded ingress/control/output циклу. IRQ/notification/diagnostic sequence не є progress. Owner і notify timer не належать повторно створюваним ingress resources; після handoff cleanup RMT/GPIO не видаляє worker й не повертає GPIO0 app.

Production додаткова portable state/snapshot RAM≤2048B без існуючих queues/stacks/SDK contexts; це design upper bound для static assertions/map, не actual map result. Capture queues64edges/8batches і 48symbols незмінні. SDK init allocations лише під setup/погодженим recovery; portable runtime без heap, LEDC context не recreate кожну спробу.

## Bench-only evidence

Окремі operational acceptance images: timing/stall/accuracy/stress, command source лишається RC. Нормальна збірка без injection/archives. Timing journal≤64 records×48B+header128B=3200B: token/epoch/operation/percent/generation/MCU brackets/status, overflow/frozen/identity/digest. Повний recorder припиняє запис і позначає evidence incomplete; не зупиняє operational Fly, не затримує TestLOW й не рекурсивно логгує overflow. Це відмінність від finite04, де його старий terminal evidence-overflow contract зберігається.

### Timing journal: журнал подій сценарію (D76 / U1)

- Один explicit bench-сценарій на boot має compile-time fixture з ідентичністю, умовою start до першої операції, що трасується, та очікуваною завершальною подією. Ці умови фіксуються в manifest/evidence header до run; USB/runtime-команди чи автоматичний повтор для добору доказів відсутні. Командою виходу залишається лише RC.
- Кожний fully accepted valid commit оновлює поточний accepted anchor у RAM: has_valid, capture epoch/seq, pulse_end_us, width/calculated. Саме оновлення не додає record. Invalid/old/pre-barrier samples не змінюють anchor. До першої valid has_valid=false, без вигаданих token/time.
- Під час активного сценарію журналюються значущі переходи control/output/permission/route/health, пов'язані API before/after/status, усі technical failures/operation-late, retry start/result і D75 episode closure. Перед відповідною подією фіксується точна копія або однозначне immutable посилання на accepted anchor; anchor records теж входять у ліміт64. Посилання веде на збережений immutable record, не на перезаписуваний RAM anchor. Кожний потрібний API call цієї події має власні brackets/result, без вибору лише успішного fallback. Повторні успіхи без зміни стану й звичайні polls не додають records; помилки/late/retry та D75 closure фільтрувати за незмінним duty заборонено. Зокрема post-fence valid0% closure записується навіть коли physical request лишається LOW.
- Normal freeze — тільки після завершальної події fixture і її checked result, повної обробки належних подій та перевірки pending fault/barrier; late/missing/failed результат не стає pass. До start і після freeze current anchor/control можуть оновлюватися, але frozen records/identity/digest незмінні. Sticky/live faults лишаються спостережуваними за чинним export contract; пізня належна пропущена подія робить доказ incomplete, а не переписує його для pass. Попередній anchor не можна перемаркувати новим now. Відсутній необхідний anchor/result означає incomplete.
- До run перевіряється верхня кількість усіх потрібних records сценарію, включно з anchors/retry/API results, у межах64. Переповнення припиняє запис і робить evidence incomplete; без wrap/витіснення/вибіркового видалення/рекурсивного запису overflow. Для наступного сценарію потрібен окремий явно виконаний bench run; freeze/full не зупиняють operational output/capture, не змінюють Test deadline/Fly hold.
- RAM межа лишається64×48B+header≤128B; extra bench≤12288B. Під час stall/reset volatile journal зникає: обов'язкові зовнішній запис/anchor, архів через reboot не відновлюється. Це event evidence, не per-pulse accuracy/600с loss доказ: чинні accuracy128-record і stress summary мають окремі незмінні правила.

Accuracy — чинні 128×48B+header≤256B=6400B без відбору добрих samples, frozen export після checked LOW/навмисної зупинки capture; такий explicit bench-only stop не є production Fly cutoff. Stress summary≤512B,601MCU-секунда від[t0+1 с,t0+602 с),t0 — first accepted valid end після startup; кандидат належить за end, late processing до closure≤17 мс за D62 методом. Capture/output не зупиняються після summary freeze, ніякого finite04 LOW на 605 с. Максимальна додаткова 05 bench RAM≤12288B включно з collectors/metadata/formatter, без існуючих stacks/queues; перевірити map та high-water при реалізації.

Watchdog reset стирає volatile journals; архів попереднього boot не відновлюється. Stall evidence потребує зовнішнього запису й детермінованої одноразової bench injection після відомої accepted/apply події; USB прихід рядка чи reset reason самі не timestamp reset. Retained duty/mode/breadcrumb не вводяться.
