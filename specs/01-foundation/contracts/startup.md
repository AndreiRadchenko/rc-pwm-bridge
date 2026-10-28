# Контракт startup та повторних спроб

Версія 4, 2026-10-28. FR-007,FR-008,FR-010,FR-011; D12,D20,D22,D23.

## Межі

Core приймає лише час і результат спроби, повертає рішення «спроба потрібна» або
«нічого не робити» та snapshot. GPIO/SDK типів у його public headers немає.
Дозвіл роботи завжди false. Сигнал RC не є входом цього автомата.

Параметри нижче вибрано саме в Plan як технічні правила init retries. Вони **не є**
RC timeout, cutoff budget або arming/recovery політикою Q05.

| Параметр | Значення |
|---|---|
| Максимум спроб | 5 загалом, включно з першою. |
| Перша спроба | Одразу після успішної перевірки профілю; не чекати console host. |
| Пауза після невдачі | 1 000 000 мкс від моменту отримання результату невдалої спроби. |
| Опитування supervisor у target | Номінально раз на 10 мс після старту scheduler; без busy-wait. Верхню межу фактичного запізнення в 01 не задано (D23). |
| Успіх | `ReadyDisabled`, PWM LOW, дозвіл false; більше init-спроб у цій сесії немає. |
| П'ята невдача | `Exhausted`, утримання заборони; без автоматичного reset чи нових init-спроб. |
| Новий reset | Нова сесія й новий лічильник; не відновлення дозволу. |

Пауза — мінімальна: наступна спроба не раніше completion_us + 1 000 000 мкс.
За D23 target видає її на першому фактично доступному poll на/після deadline;
номінальний період 10 мс не є гарантованою верхньою межею запізнення.
У 01 не вводиться окремий target-timing режим або критерій lateness ≤10 мс.
Host-тести перевіряють earliest deadline та одну спробу на запізнілому tick,
target config/source review — номінальні tick/poll settings; це різні докази.
Тривалість синхронної спроби не входить у паузу. Це init retry, не RC timeout
або operational cutoff budget Q05.

## Переходи

| Початковий стан / подія | Результат |
|---|---|
| `AwaitingAttempt`, перше опитування | Видати одну спробу, count=1, `Attempting`. |
| `Attempting`, успіх | `ReadyDisabled`; deadline відсутній. |
| `Attempting`, невдача, count<5 | `RetryWait`, deadline=completion_us+1 000 000. |
| `RetryWait`, now<deadline | Без дій. |
| `RetryWait`, now>=deadline | Одна нова спроба, count+1, `Attempting`. |
| `Attempting`, п'ята невдача | `Exhausted`; deadline відсутній. |
| Термінальний стан, подальші ticks | Без init-спроб, дозвіл false. |
| Регресія часу / overflow deadline / порушення протоколу результатів | `ClockFault` або явний terminal protocol fault, без init-спроб і дозволу. |

Для protocol fault використовується `Exhausted` з причиною `ProtocolFault`; окремий
phase не додається. Дубльований result не збільшує count і не запускає спроби.

## Порядок однієї платформної спроби

1. Перевірити погодження/відповідність compiled profile. Неповний профіль блокує
   target configure; runtime невідповідність не дозволяє GPIO-access.
2. Ідемпотентно застосувати PWM LOW; спочатку preload LOW latch, далі потрібний
   режим виходу за перевіреною послідовністю SDK. `gpio_reset_pin` не є assert LOW.
3. Якщо assert LOW повернув помилку, зупинити решту кроків, зафіксувати failure
   і планувати bounded повторення тільки дозволеної послідовності. Не стверджувати
   фізичний LOW за невдалою API-операцією; зовнішній bias — окрема передумова.
4. Перевірити обов'язкові локальні служби/адаптери, зокрема ідемпотентне встановлення
   USB transport та наявність diagnostic worker без очікування host. Помилка цього
   локального setup є невдалою init-спробою; наступна не встановлює вже готовий
   driver вдруге. У 01 немає RMT RX, LEDC output,
   Wi-Fi/Bluetooth, NVS, network stack чи load-enable кроків.
5. Передати `InitOutcome` supervisor; опублікувати snapshot без очікування.

DIR до startup-послідовності не входить: на MD20A передбачено постійну перемичку
DIR–GND за D20. GPIO1 не конфігурується для DIR, програмного перемикання напрямку немає.

Монотонний clock має бути доступний до планування спроб: адаптер використовує
`esp_timer_get_time()`, перевіряє невід'ємність і передає час у core. Помилка clock
не запускає init без визначеного часу. Від’ємний signed raw або помилка clock-порту
перевіряються до cast у MonotonicUs; спільна SDK-незалежна політика переводить
сесію в terminal ClockFault з permission=false без штучного timestamp і без
init/GPIO/setup викликів. Checked conversion має бути доступна fake-clock тестам,
а SDK leaf лише отримує raw. Deep/light sleep і dynamic sleep у 01 не потрібні.
У snapshot now_valid=true означає валідний час поточної події. Для відсутнього,
від'ємного або регресивного sample now_valid=false; now_us зберігає останній
прийнятий час або 0 до першого валідного sample. Таке значення не є поточним
виміряним часом; його не передають у core як заміну невдалої clock-операції.
До першого запиту clock відсутність sample у початковому snapshot сама не є fault;
перехід у ClockFault потребує невдалої/невалідної clock-операції.

Помилка профілю не лікується підстановкою pins. У runtime дозволено лише повторити
валідацію без GPIO; помилка залишається причиною blocked, а не дозволом роботи.

## Діагностика

Startup не виконує stdout/ESP_LOG/USB writes. SDK application logs у design вимкнено
через `CONFIG_LOG_DEFAULT_LEVEL_NONE=y` і `CONFIG_LOG_MAXIMUM_EQUALS_DEFAULT=y`;
перевіряється фактичне maximum level=0 та відсутність обходів `LOG_LOCAL_LEVEL` у
викликаних init-компонентах. Локальний snapshot передається через статичний mailbox
із заміною попереднього запису, без очікування в producer.

За D22 producer публікує новий sequence для початкового snapshot і подій, що
змінюють phase/attempt/deadline/error/output-result. Зміна лише timestamp під час
опитування не є новою подією. Окремий consumer нижчого пріоритету перевіряє mailbox
раз на 100 мс і лише для нового sequence робить одну спробу передати запис до
256 байтів. Він використовує `usb_serial_jtag_write_bytes` з timeout 0,
TX buffer 512 байтів і RX buffer 64 байти; не викликає wait_tx_done/flush/stdout.
Після спроби sequence позначається обробленим незалежно від кількості переданих
байтів; нульова/часткова передача — dropped, без повтору запису чи його залишку.
Поточний snapshot може зберігатися внутрішньо, але повторно не передається.
USB reconnect не створює подію, не скидає маркер consumer й не спричиняє replay.
Втрачений термінальний стан може не з'явитися до наступної події або окремо
дозволеного reset. Немає heartbeat, command parser чи automatic reset для виводу.
Console backpressure не створює mutex/очікування в startup.
USB Serial/JTAG — обраний транспорт за умови підтвердження USB-розводки Q02;
UART fallback не призначається непомітно. Console worker створюється після assert LOW
та працює за готовності transport; його непрацездатність є init failure й незавершеним
прийняттям локальної діагностики, а не приводом дозволити роботу.
Стан/причина зберігаються у snapshot незалежно від доступності виводу. Відсутність
або нечитання host не є setup failure; доступність виводу тоді не гарантується.

## Майбутні host-перевірки

- Початок, waiting/attempting/ready/exhausted/clock fault: permission завжди false.
- Невдача у t=0; tick 999999 не запускає спробу, 1000000 запускає рівно одну.
- Перший tick із запізненням після deadline запускає рівно одну спробу без
  catch-up циклу; це не доказ верхньої межі target lateness (D23).
- Повторні ticks у той самий момент не дублюють спробу; новий deadline рахується
  від completion наступної невдачі, не від старту попередньої спроби.
- Успіх третьої спроби завершує retries; рівно п'ять невдалих ведуть в Exhausted.
- Після Exhausted немає автоматичних resets/спроб; fresh reset очищає volatile count.
- Mock output failure не допускає решту init; DIR GPIO-викликів немає в жодному сценарії. Непогоджений профіль не допускає
  жодного GPIO-access. Перевіряється порядок викликів, а не лише кінцевий bool.
- Невалідний/від’ємний raw clock або clock-port failure до cast дає terminal
  ClockFault без init/GPIO/setup; permission=false, штучний timestamp не передається.
  Snapshot має now_valid=false та попередній валідний now_us або 0, без unsigned wrap.
- Регресія часу/overflow/дубльований result не породжують команд чи unsigned wrap.
- Штучно неправильне очікування в окремому negative probe дає ненульовий exit.
- Нульова/часткова передача нового sequence дає одну спробу й drop; наступні
  consumer polls та USB reconnect не повторюють його. Новий sequence допускає
  рівно одну нову спробу. Термінальний стан без нової події не породжує heartbeat.
