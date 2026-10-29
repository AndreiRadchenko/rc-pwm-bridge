# Повідомлення оператора: HJ Tester, установлення1701мкс

2026-10-29. Джерело — повідомлення користувача з потоком rc03. Користувач повідомив, що тестер підключений і видає1701мкс. Це значення налаштування/показу тестера, не окреме вимірювання логічним аналізатором. Агент serial port/плату не відкривав, не прошивав і не керував навантаженням. Реальні HIGH/LOW, схема підключення цього запуску, firmware hash та історія режимів/reset ще не підтверджені.

Нижче лише вибрані записи з повідомлення, не повний raw per-pulse archive:

```text
rc03 q=1bb4f id=f1da070bd29de94e sid=f1e w=1702 s=5e74e8b e=5e75531 p=19992 v=1 c=0 d=70 o=0 a=0 f=0 n=1359,ef9,460,0,0,0 k=1b77c,0,0,3df,2a04d m=10,b9,0 ft=0,0 b=3,1,0,1 sat=0
rc03 q=1c365 id=f1da070bd29de94e sid=f1e w=1704 s=602c3aa e=602ca51 p=19994 v=1 c=0 d=70 o=0 a=0 f=0 n=13b3,f53,460,0,0,0 k=1bf80,0,0,3f1,2acd1 m=10,b9,0 ft=0,0 b=3,1,0,1 sat=0
rc03 q=1c681 id=f1da070bd29de94e sid=f1e w=1703 s=60d7147 e=60d77ee p=19994 v=1 c=0 d=70 o=0 a=0 f=0 n=13d6,f76,460,0,0,0 k=1c295,0,0,3f8,2b1af m=10,b9,0 ft=0,0 b=3,1,0,1 sat=0
```

## Спостереження

- У всіх наданих snapshots current statusValid (`v=1`), captureWaitingRise (`c=0`), capture faults0. RMT width1702–1704мкс, estimated rising period19990–19997мкс у всьому повідомленні; nominal50Гц і вікно18000–22000мкс.
- Calculated duty70%: для1701..1704мкс карта дає70,1..70,4%, погоджене integer rounding дає70. Effective command0/permissionfalse за03; це не фізичне вимірювання PWM GPIO0.
- Різниця width із показом1701 становить+1..+3мкс. Без придатного analyzer/U це не E_width для D54 і не pass точності≤5мкс. Не доведено, яка частина різниці походить від джерела, conditioning чи capture.
- `s/e` — hex estimated GPIO ISR timestamps, `w` — незалежна hardware duration RMT. У вибраному record w1704, але e−s1703; ці джерела вимірювання незалежні за D51, не потрібно примусово прирівнювати.
- First `n=1359,ef9,460,0,0,0` →4953 candidates,3833 valid,1120 invalid,0 discarded,0 fault events,0 duplicate edges. Last `n=13d6,f76,460,0,0,0` →5078 candidates,3958 valid,1120 invalid. За фрагмент candidates/valid збільшились на125, invalid не збільшився.
- First rising timestamp99045003мкс, last101544263мкс: interval2,499260с для125 additional valid samples. Узгоджується з приблизно50Гц, але не є analyzer count/zero-loss proof.
- 1120 invalid накопичилися до першого наведеного snapshot. Primary причини й їхню історію ці записи не зберігають; один startup unknown не пояснює весь count. У користувача уточнюється, чи були інші widths/period modes.
- `m=10,b9,0` hex →max rearm gap16мкс після estimated falling+5000мкс; max processing185мкс після callback completion; max fault lateness0 за відсутніх capture faults. Це cumulative software instrumentation, не зовнішнє вимірювання accuracy/deadlines.
- `k` overwrite зростає як latest-only snapshot policy, busy/write-drop0 у наданому фрагменті. Це не RC loss count.
- Попереднє повідомлення мало f0x20; поточне f0. Capture fault flags/core latch не очищуються від valid за production API. Уточнюється reset/перепрошивання між фрагментами. profile ID і sid=f1e не є унікальним boot ID та не доводять однаковий запуск.

Друкується лише latest snapshot≤10/с при приблизно50 імпульсах/с. Цей текст не містить кожен pulse, не закриває D55 accuracy windows,600с loss tests або T043–T047/T051. Q03/H02/H03 і всі невиконані gates збережено. Точний висновок щодо історичних invalid/reset очікує відповіді користувача; code/номінали/допуски за цим повідомленням не змінювались.

## Уточнення користувача

Користувач підтвердив reset між попереднім f0x20 та цим f0; це пояснює скидання sticky state без автоматичного clear від valid. Також підтвердив, що налаштування тестера весь час були1701мкс/50Гц, інших ширин/частот не задавав. Попередня гіпотеза про зміну режимів не підтверджена. Ще уточнюється, чи сигнал був фізично присутній до reset, чи тестер підключили/увімкнули пізніше.

Перевірка policy source: просте очікування без фронтів не збільшує candidates/invalid; invalid збільшується лише для incomplete candidate або завершеного candidate зі статусом InvalidWidth/PeriodUnknown/InvalidPeriod. Без loss/fault/recovery перший unknown period очікується один раз, а не1120. Тому1120 не можна пояснити самим часом очікування чи вважати нормальним startup count. Можлива активність на непідключеному/неузгодженому вході — лише гіпотеза, не виміряний висновок. Якщо коректний постійний сигнал був присутній від reset, ці відхилення потребують окремої діагностики причин/ранніх logs.

У видимому фрагменті invalid не зростає, однак причина історичного count залишається відкритою. Firmware/валідація/номінали за уточненням не змінювалися, нова прошивка не потрібна для самого пояснення/reset-висновку.
