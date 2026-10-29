# Фізичне прийняття 03 — not_run

2026-10-29. Користувач дозволив лише програмну реалізацію, host-тести й target builds із відкритими фізичними задачами. У цьому виконанні плату не прошивали, serial monitor не відкривали, сигнал і навантаження не підключали.

| Gate / задача | Стан | Відсутній доказ |
|---|---|---|
| T043 / H02,H03,Q03 | blocked | Зовнішній bias/живлення/startup evidence01; перевірене conditioning HJ до3,3В, ground/protection; analyzer model/timebase та U_width/U_interval≤1мкс |
| T044 | not_run | Окреме доручення на flash/підключення після gates |
| T045 | partial; formal acceptance pending | Raw accuracy archives/LA edges/E кожного pulse/interval; actual stuck/rearm/processing/fault timings |
| T046 | not_run | Три окремі600с runs active/slow/absent reader; analyzer/accepted counts,0loss,actual load/timing |
| T047 | partial; full stress/reconnect pending | Реальний USB reconnect/partial/fault під max diagnostics, GPIO0 waveform |
| T051 | blocked | Повне прийняття03 потребує T043–T047 |
| Receiver gate D48 | pending до використання | Конкретний RC-приймач, електричні параметри і radio-loss/failsafe поведінка |

Очікувані межі визначено заздалегідь: width error≤5мкс кожна, rising-interval error≤10мкс кожен, U≤1мкс окремо; rearm≤1000мкс після falling+5000, completion→core≤10000мкс; HIGH/LOW threshold100000, fault delivery≤110000мкс. Виміряні значення: **відсутні**. Жодна межа не оголошена досягнутою.

Процедура — [hardware-procedure.md](hardware-procedure.md). Q05 лишається05, Q06 —04/06. LOW MD20A означає brake; насос/MD20A мають бути від’єднані для03.

Користувач повідомив про читання ROM boot log через PlatformIO monitor (USB reset, SPI boot, entry) без rc03 у наведеному фрагменті. Повної процедури/вимірювань не надано. Знайдено програмну помилку USB RX64, виправлено128; перевірка виправленої збірки на платі pending. Це не закриває фізичних tasks.

Оновлення за наступним повідомленням користувача: rc03 application records вже присутні. GPIO3 спостерігається HIGH без завершених pulses; єStuckHigh0x20 і software lateness161мкс. Джерело/схема GPIO3 та bin hash цього запуску ще не підтверджені. Розшифрований витяг — [operator-usb-log-2026-10-29.md](operator-usb-log-2026-10-29.md); це часткове спостереження, не завершення T043–T047/T051.

Наступний фрагмент користувача з підключеним HJ/показом1701мкс: latest widths1702–1704,period19990–19997,statusValid,duty70/effective0, fault0. Історичні invalid1120 сталі протягом фрагмента; історія режимів/reset уточнюється. Розбір — [operator-hj-1701-log-2026-10-29.md](operator-hj-1701-log-2026-10-29.md). Це partial operator observation, не analyzer accuracy/600с acceptance.

Уточнення користувача до HJ1701: reset між f20 і f0 був; тестер весь час налаштований1701мкс/50Гц. Зміна режимів не пояснює1120invalid. Відсутність фронтів сама не збільшує цей count; порядок фізичного підключення відносно reset ще уточнюється, причина попередніх відхилень відкрита.

За новим явним дорученням користувача агент виконав60с read-only USB monitoring поточного sweep800–2200. 600 current snapshots +1раніший committed record; за continuous interval candidates+2996,valid+2396,invalid+600,detected discarded/fault/duplicates0. Latest width802–2203,status відхилення на краях відповідає900–2100. Жодного TX/flash/навмисного reset; порт закрито. Raw/межі — [serial-sweep-20261029T155006Z-report.md](serial-sweep-20261029T155006Z-report.md). Формальне фізичне прийняття/600с/analyzer не виконано.
