# Контракт operational керування 05

Дата:2026-11-06; D63–D73, уточнення retry D75/CHK017. Локальні embedded boundaries, без web/network/команд USB. Це плановані інтерфейси та acceptance limits, не firmware API, що вже існує. Семантика нормативна; C++ назви нижче — обраний дизайн.

## 1. Розширення 02 та міграція 03/04

`ControlCore()` лишається LegacyDisabled із незмінною поведінкою 02, а `ControlCore(ControlConfig)` додає Test/Fly. Один core усередині CapturePolicy; constructor config immutable. Build family/core mode/output role мають окрему узгоджену [таблицю composition](../data-model.md#незмінна-конфігурація); FiniteBench04 не є mode ControlCore. Старі update_measurement/update_capture_fault/clear_fault зберігають legacy контракт у core mode LegacyDisabled. У operational немає production clear/arm входу; неприпустимий legacy mutation там дає protocol error, не зняття inhibition.

Operational boundary:

| Виклик тільки з owner | Семантика |
|---|---|
| accept_valid(AcceptedMeasurement,now) | Після повної association/processing validation; повторна width/map перевірка, checked end≤now, current epoch/barrier і new token/end. Оновлює calculated/last-valid тільки accepted valid |
| observe_input(status,current_bits,history,barrier) | Облік faults/invalid/resync без переносу last-valid/end і без пересування measurement ordering fault-now |
| evaluate(now,OutputHealth) | Shared-now ordering, technical priority, no-prior, Testage≥500000/Flyhold; повертає effective duty/permission/generation |
| output_failure(class,status)/output_recovered(end_fence,epoch) | Technical inhibit/recovery fence; history окремо; не використовується для звичайного input loss |
| snapshot() | Узгоджена [модель даних](../data-model.md), без I/O |

CapturePolicy не змінює Q04 thresholds/parser/epoch rules. У operational fault(now) більше не викликає legacy update_capture_fault у measurement stream; current status/history й core evaluate окремі. Parse/on_batch candidate не стає командою до processing_complete і повторної ingress barrier перевірки. Core TimeOrderError при старому sample — input discard/resync; реальна регресія shared raw — fatal clock, не підміна одним іншим.

У platform output_faults/output_service для operational прибрати blanket OR усіх sticky capture bits у terminal04. Перед кожною nonzero SDK operation, включно зі stopIdle1, зняти ingress under existing short lock, відкинути pending-invalid candidate й recompute mode; повторити після API та між set/update. Lock не охоплює SDK. Новий ingress після точки серіалізації не обіцяється фізично атомарним із поточним write. Input loss у Fly не інвалідовує held command output generation; technical error — інвалідовує. Legacy/finite04 зберігають старий fault-priority/terminal path, startup_had_failure й finite timing.

## 2. Пріоритет і автоматичні переходи

1. Unapproved profile/config не дають GPIO/nonzero. Після handoff з armed watchdog raw-clock invalid/protocol-fatal/LowUnconfirmed →ResetPending,best-effortLOW/no feed; до armed setup діє SetupBlockedLOW без claim watchdog reset.
2. Recoverable output failure → technicalLOW/retry; ExhaustedLow →LOW до reboot. Input samples можуть рахуватися/оновлювати calculation, але не дають effective nonzero до technical fence.
3. Після успішного reinit — AwaitPostRecoveryValid/LOW, нова valid end>recovered_at дозволяє наступну команду за mode. Old queued/cached high не replay.
4. Без valid поточного boot — AwaitValid/LOW/permissionfalse.
5. Test age≥500000 → TestTimedOut/LOW/permissionfalse, calculated зберігається. До межі — last-valid. Fly не cutoff за віком/input loss.
6. Нова fully-valid після потрібного resync → Active/calc duty/permissiontrue, включно з high та 0%. Sticky history не забороняє перехід; resync сам не команда.

Одночасні події: owner спочатку consumes pending fault/barrier, commit придатних нових candidates, потім evaluate з fresh now перед кожним hardware apply. Якщо нова valid вже committed до точки рішення, deadline рахується від її end. Якщо cutoff уже застосований, відновлює тільки нова valid; stale generation не скасовує LOW. Порушення association/processing budget не може коротко дозволити candidate до resync.

## 3. Output API і lifetime

Roles LegacyDisabled/FiniteBench04/Operational05 відокремлені від LEDC readiness. GPIO0 primedLOW не означає timer/channel ready. Той самий адаптер на startup та після одноразового handoff; другого writer немає. Реальний DIR–GND не керується firmware.

Залишити LEDC low-speed timer0/channel0/APB80 МГц/13bit/1000 Гц/hpoint0/noinvert/nofade,1..99% count=(p×8192+50)/100,0/100% checkedstopIdle0/1, duty8192 заборонений. Invalidpercent/protocolgeneration не clamp. Один і той самий generation/epoch idempotent; samegeneration ізіншим duty або epochoverflow — protocol-fatal.

На outputAPI failure: зафіксувати перший status/op/time-validity, inhibit nonzero/нова epoch, checkedstop0. Якщо channel unavailable або stopfailed — GPIO preload0→directionOUTPUT, обидва checked. Успішний шлях підтверджує API-levelLOW, не voltage. Failure кожного API зберігається в history; LowUnconfirmed тільки якщо жоден повний LOW path неуспішний. LowUnconfirmed забороняє reinitnonzero, no feed→watchdogreset; до reset нічого не маскується як успішне cutoff.

Recoverable після confirmedLOW: до 3 додаткових reinit спроб, нераніше 100000 мкс від завершення невдалої операції, що потребує наступного retry. Кожна спроба — неблокуючий staged FSM, не sleep/catch-up. Cleanup за knownstages: GPIOLOW/reroute, timerpause→deconfigure якщостворений; помилка нескидає ownership. Потім timerconfig, channelconfig з duty0, окремі checkedset0/update/stop0 й newrouteLEDCLow. Не claim destroy/recreateLEDCallocations. Кожна SDK операція обмежена 1 мс design budget; owner обслуговує ingress/control до/після неї. Attempt completion≤20 мс від старту без послаблення 03rearm/processing. Перевищення/зависання — failure/або watchdog, не pass. Після успіху fence end>recovered_at, жодного replay.

**Епізод retry (D75)**: починається з першої recoverable output failure; початкова невдала операція не є reinit-спробою. `retry_count` збільшується на старті кожної reinit-спроби, незалежно від її результату, і не перевищує3. Успішні reinit0, fallbackLOW або сам commit нової valid не поновлюють бюджет. Після reinit0 стан AwaitPostRecoveryValid/LOW зберігає витрачений ліміт; епізод закінчується лише після checked успішного застосування нової trustworthy post-fence valid-команди (`end>recovered_at`, актуальні capture/output epoch і generation), включно з valid0%. Тоді `retry_count=0` для майбутнього незалежного епізоду; sticky history зберігається. TimeoutLOW/службовий stop0 без такої команди епізоду не закриває. До цього моменту будь-яка нова output failure належить тому самому епізоду: після checkedLOW наступна спроба можлива лише із залишку бюджету та через≥100 мс від завершення цієї невдалої операції. Якщо три спроби вже витрачено, така failure дає ExhaustedLow до reboot, без appautorestart. Успішна третя reinit-спроба сама не exhaustion: вона ще може завершитися успішним post-fence apply. Для кожної нової owner-visible failure заново діє її aggregate fault→LOW≤1 мс; fallback не переносить початок цієї межі. ConfirmedLOW/healthy supervisory cycle feed дозволений під час очікування/exhaustion; input-loss recovery D64/D65 не витрачає цей output-reinit бюджет.

RMT/GPIO failures/rearm errors/overflow/stuck/malformed означають input unavailable: retry ingress≥100 мс з boundedresources, Testdeadline незмінний, Flyhold. Monitor й owner після handoff не cleanup разом із RMT/GPIO. Справжня sharedclock failure — ResetPending без time-based retry. Неправильний runtime percent/requestepoch — protocolfatal після доведеної armed subscription: best-effortLOW/no feed/reset. Watchdog init/add failure до handoff — SetupBlockedLOW без operational permission і без обіцянки watchdog reset: таймер без підписаної task може не працювати. Не handoff і не запускати nonzero; тільки новий окремо дозволений reboot або виправлення setup. Profile/clock failure до supervision лишають foundation blocked без GPIO для непогодженого профілю. Якщо esp_task_wdt_reset повернув error після handoff, best-effortLOW та явний emergency ROM reset, а не припущення, що втрачена subscription сама дасть reset. Жодна з цих помилок не лікується нескінченним output retry.

## 4. Числові межі M05

| Метрика | Обрана вимога до майбутньої перевірки |
|---|---|
| Testtimeout |500000 мкс, age≥T;499999hold,500000/500001LOW |
| Test загальний physical cutoff | L_measured+U_latency≤1000000 мкс від відповідного останнього accepted physical falling |
| Owner healthy service |notify1 мс; cutoff firstservice lateness≤1 мс; checked LOW≤2 мс від t_v+T |
| Normal valid→output |≤19000 мкс до checkedAPI result у MCU clock; direct physical falling→новий state≤20000 мкс |
| Technical fault→LOW API |≤1000 мкс від першої owner-visibletechnicalfailure для повного aggregate stop0 або обох GPIO fallback calls; per-call≤1 мс не доводить aggregate≤1 мс; fallback не rebase, failedLOW лишається failed |
| Retry |3 додаткові спроби на епізод D75, кожний reinit start витрачає одну; reset count лише після checked new post-fence valid apply, включно з0%;spacing≥100000 мкс від failurecompletion;1SDK call≤1 мс/attempt≤20 мс із interleaving |
| Watchdog |runtime TWDT100 мс, tasksubscription onlyowner; IWDT500 мс; reset hook перед default panic |
| Reset path allocation |last qualified checkpoint/feed→reset≤350 мс; reset→сталий physical LOW≤100 мс; критерії потребують independentresetanchor |
| Long Fly loss hold |≥600 фізичних секунд у 0/80/100%,active/slow/absent reader,далі new high/new low recovery |
| Physical latency method |U_latency≤1000 мкс;maximum bound pass за L+U≤limit;не змінює directE/U D54/M04 |

Алгебра stalled Test: checkpoint за nonzero тільки при age<T; на/після T feed тількипісля checked LOW. Тому найпізніший nonzero feed≤t_v+500 мс;350+100 мс до physical LOW дають≤950 мс,50 мс запасдо 1 с. Це розподіл прийняття, не гарантія вже вимірянихшляхів. Нормальний≤502 мс API та прямий≤1 с physical перевіряютьсяокремо. Не можна скласти MCUtimestamps з analyzertime через невідомий offset. Resetphysical/responsibilitygate не закриваєтьсяалгеброю.

## 5. Watchdog, час і startup order

Operational config: ESP_TASK_WDT_EN=y, ESP_TASK_WDT_INIT=n; runtime esp_task_wdt_init(timeout_ms100,idle_core_mask0,trigger_panictrue). Checked esp_task_wdt_add(rc_control_task), лишеця task checkedreset. Один entry, без idle чи diagnostic subscription. Сильний Csymbol esp_task_wdt_isr_user_handler у IRAM робить тільки ROM system reset; normal panic немає в declaredwatchdog path. LOGmax/default0 має прибрати SDKprehooklogs; це довести effectivecompile/ELF, не просто Kconfig. Якщо ROM повернувся — no feedloop, без поверненнядо panic. SDK/ROM calls та IRAMclosure audit — обов'язкові. Окремий tiny control_watchdog_reset.cpp містить сильний extern C hook із esp_rom/esp_attr headers без esp_task_wdt.h weak prototype; supervision init/add/reset API лишаються в control_watchdog.cpp. Включення weak declaration перед definition могло б зберегти weak binding, тому nm/map/disassembly мають довести саме strong retained symbol і ROM target.

На cleanstartup спершу profile/primedLOW/foundationsetup (5 спроб,1 смінімум D15 без дозволуроботи), потім runtimewatchdoginit й owner/notifytimer, registration і одноразовий handoff. До готовності супервізії —LOW. Owner починає outputinit/captureingress, сам обслуговує RMT/GPIOretry; перша trustworthyvalid після readiness транслюється одразу. startup_had_failure історичне для diagnostics, не bootlong lock operational післяуспіху; finite04 зберігаєстарий gate. Якщо owner/monitor нестворено — setupfailureLOW, нечастковий handoff. Після handoff app більше не poll-ить startup/output.

Checkpoint після bounded ingress/error handling, currentclock/ordering check, повного processing validation, modeevaluate, потрібного checkedapply/або перевірки вже правильного outputstate. Не feed із ISR/timercallback/dequeue/diagnostics. Cycle з missinginputFly або TesttimeoutLOW є progress. Clockfreeze:feed тільки при trustedraw>last_feed_raw. LongSDKstall/припинення notify не живлятьсяіншою task; TWDTreset. Atomiccriticalsections короткі, без I/O/SDK/formatting; MWDT0 не перепрограмовується напряму.

Sleep/DFS/cachewrite у steady state не додаються. Checked signed64raw до cast; епізод clockregression/overflowinvalid→LOW/no feed/reset; oldmeasurementordererror→capture resync, не fatalclock, якщо sharedraw справний. Статистична saturation не вимикає Fly; modulo32 seq/epoch з boundedspan, uint64 requestcounter із overflowguard. Післябудь-якого reset жодного retainedduty.

TWDT stage0ISR спершу feed, stock panic з 10 с RTCwatchdog непридатний як proof1 с. IWDT500 мс не повинен раніше TWDT nominal300 мс stage1 зайтиу panic. За arbitrarypanic/WDT/oscillator/reset/padfailure немає universal fault tolerance. Фізичні declaredstallcases перевіряються без debugger, через freshboot після OpenOCD; unsupportedhandlerfailure не зараховується як pass.

## 6. Diagnostics rc05 v1

Production record: newline + `rc05 1` + групи нижче у fixed order + newline. Hexlowercase без 0x, один ASCIIspace між tokens; груповийпрефікс додається лише першомуполю. Наприклад q=sequence build boot. Відсутні/невідомі поля/enum/mask/версія дають malformed;немає guesseddefaults. Numbers без leadingpadding,не більше наведеноїрозрядності.

| Група | Поля послідовно | Максимальні hex widths |
|---|---|---|
|q|sequence,buildfingerprint,bootidentity|16,16,16|
|i|width,start,end,period,capturestatus,capturephase,currentcapture,historycapture|8,16,16,8,2,2,4,4|
|o|mode,controlphase,calculated,effective,permission,applied,last validend,age,validflags,currenttechnical,historytechnical,retry|2,2,2,2,1,2,16,16,2,4,4,2|
|c|candidates,valid,invalid,capturedrop,capturefault,duplicate|8,8,8,8,8,8|
|d|overwritten,busy,writedrop,attempts,bytes|8,8,8,8,16|
|w|progresssequence,rawresetreason,saturated,lastsdkstatus,lastsdkoperation|16,2,1,8,2|

Консервативна межа≤347B разом із framing,buffer385B із NUL,stresspadding до 384B перед newline. Timingmaxima й повний startupdiagnosticcontext лишаються в RAMsnapshot/evidence; не додаютьсябезперевірки payload. validflagsbits:0hasvalid,1nowvalid,2agevalid,3workpermissionvalid; решта 0. При hasvalidfalse calculated/last valid/age чисельно 0, validityflags показуютьвідсутність. Mode0Test/1Fly; phasecodes0..6 відповідаютьпорядкумоделі; applied0Unknown/1Low/2Pwm/3High. Capturestatus/phase legacyordinals0..6/0..3 стабілізувати як wirecodes fixtures. Technicalmasks:1OutputApi,2Cleanup,4OperationLate,8Clock,16Protocol,32LowUnconfirmed,64WatchdogSetupOrFeed,128RetryExhausted;інші bitsreserved0. SDKstatus у 32bit two'scomplement;opcodes reuse stable PumpOperation, невідомийкод malformed. Rawresetreason0..255 від SDK, не гарантує watchdoglabel після ROMreset; saturation1bit.

Зберегти latest-only/spacing≥100 мс/≤10 записів/с/TX512/RX128/zero-waitdrain gate/onecommitted record/versionack/drop accounting03. Вже committed записможе завершитисяпісля reconnect;наступний latest,без pendinghistory/burst. Writer не touchcore/output/watchdog. Production немає archives/commandinput. Explicitbench export використовуєтойсамий writer/slot замість latest, із currentfaultflags, беззатримки owner;повні H/R/E cycles зі sourceidentity/count/digest, неповні/mixed/overflowcycles не evidence. Grammar rc04/rc03-trace не змінюється.
