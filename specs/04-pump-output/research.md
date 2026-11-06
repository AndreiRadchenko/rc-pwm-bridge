# Дослідження 04: вихід PWM та запуск стендової перевірки

Дата: 2026-10-30. Фаза: Plan / Phase 0. Це аналіз джерел, не реалізація, target build або фізичний тест. Дослідження проводиться на pinned ESP-IDF v5.5.6, commit `880a203ae603d08a94d06aa01e0d6761958e5e9e`.

## RCH-01 — Перевірка виконуваності D58

**Статус**: вирішено явною відповіддю A користувача, D59. D58 збережено як історію, його power-cycle-only вимогу замінено для окремого bench image. Phase0 закрито; фактичних hardware результатів немає.

**Історична вимога D58 до D59**: лише фактична нова подача живлення може почати стендову послідовність; будь-який інший reset, зокрема brownout/watchdog/кнопка Reset, залишає LOW. Нових GPIO чи командного входу немає.

**Доказ у зафіксованому SDK**:

- `build/scopa01-idf-sdk/components/soc/esp32c3/include/soc/reset_reasons.h`, рядки 30–32: `RESET_REASON_CHIP_POWER_ON`, `RESET_REASON_CHIP_BROWN_OUT`, `RESET_REASON_CHIP_SUPER_WDT` мають однакове значення `0x01`.
- `build/scopa01-idf-sdk/components/esp_system/port/soc/esp32c3/reset_reason.c`, рядки 17–24: цей код повертає `ESP_RST_POWERON`. Гілка не відрізняє перелічені причини. Окремі system-level brownout/WDT коди існують, але не усувають alias chip-level коду.
- Commit локального SDK перевірено read-only через Git; він збігається з environment lock. Пристрій не підключався.

Первинні вихідні тексти: [reset_reasons.h](https://github.com/espressif/esp-idf/blob/880a203ae603d08a94d06aa01e0d6761958e5e9e/components/soc/esp32c3/include/soc/reset_reasons.h), [reset_reason.c](https://github.com/espressif/esp-idf/blob/880a203ae603d08a94d06aa01e0d6761958e5e9e/components/esp_system/port/soc/esp32c3/reset_reason.c).

Офіційний [ESP32-C3 TRM v1.4, §6.1, Table 6.1-1](https://www.espressif.com/sites/default/files/documentation/esp32-c3_technical_reference_manual_en.pdf#page=193) також описує спільний chip-reset код; §6.1.1 не гарантує збереження внутрішньої пам'яті після chip reset. Тому RTC/noinit magic не доводить відмінність power-off від усіх заборонених chip reset. [Datasheet §2.5.3](https://documentation.espressif.com/esp32-c3_datasheet_en.html) описує CHIP_EN як спільний power-up/reset інтерфейс. Зв'язок конкретної кнопки SuperMini з EN тут фізично не вимірювався; його не видаємо за перевірений факт. Уже самої alias-неоднозначності brownout/super-WDT достатньо, щоб блокувати D58.

Нативні USB raw resets 0x15/0x16 у pinned mapping дають ESP_RST_USB, але це не покриває всі способи upload/reset. [Офіційна esptool документація](https://docs.espressif.com/projects/esptool/en/latest/esp32c3/advanced-topics/boot-mode-selection.html#automatic-bootloader) окремо описує EN-based шлях.

**Висновок із коду**: умова `esp_reset_reason()==ESP_RST_POWERON` недостатня для D58. Вона може дозволити запуск після причини, яку D58 забороняє. Відхилення всього класу `0x01` залишить LOW і після справжнього power-on, тому не дасть потрібної тестової послідовності. Не можна назвати це повною реалізацією 04.

**Рішення D59**: тільки окремий no-pump bench image може одноразово виконати послідовність після кожного нового app boot незалежно від reset reason. Initial LOW/готовність/fault gates зберігаються, в одному boot повтору немає. Звичайний образ завжди LOW/disarmed; автоматичний application reset не додається. Не намагаємося довести фізичний power cycle через reset enum.

**Розглянуті альтернативи**:

1. Змінити політику тільки окремої no-pump стендової збірки: один скінченний тест після кожного нового boot, з початковим LOW та всіма чинними fault-умовами; усередині boot повтору немає. Це змінює D58 і acceptance reset/upload для стендової збірки; звичайна збірка незмінна.
2. Зберегти точний D58 та дослідити додаткове незалежне підтвердження нового живлення/дозвіл запуску. Це нова апаратна або командна межа, не затверджені GPIO/схема; до підтвердженого рішення автоматичний стендовий запуск заблокований.

Користувач явно ухвалив першу альтернативу; друга не активована. RTC-прапорець без доведеної persistence/reset-domain поведінки та NVS-запис без незалежного свідчення power-off не закривають вимогу розрізнення фізичних подій.

## RCH-02 — Успадковане середовище та межі

**Рішення**: зберегти `tools/environment/environment.lock.json`, native ESP-IDF v5.5.6, strict C++11 ядро та C++17 app/platform, RC GPIO3, PWM GPIO0, DIR–GND на MD20A. Не додавати SDK/build system або GPIO.

**Обґрунтування**: D14/D18–D20/D28, чинні контракти 02/03. Q01 закрито для записаного environment; локальні шляхи перевіряються перед майбутньою збіркою. Програмні докази 01/02/03 відокремлені від відкритих H02/H03/Q03/Q06. У цьому Plan host/target/physical тести не виконувалися.

**Альтернативи**: Arduino, нова плата чи повернення до застарілих hardware timestamps/3 мс stuck HIGH не відповідають поточному обсягу.

## RCH-03 — Інтеграційна межа виходу

**Незалежний результат аналізу**: наявний `firmware/main/app_main.cpp` викликає startup supervisor, а `platform::RcCapture` має власний serialized owner із capture policy/core. Новий вихідний controller не повинен створити другого конкурентного writer станів/LEDC.

`EspSafeOutput::apply_low()` зараз керує звичайним GPIO0. Після передачі контакту LEDC цю операцію треба замінити єдиною adapter-owned семантикою LOW; простий `gpio_set_level` під активним peripheral routing не доводить LOW на контакті. Наявний supervisor після ReadyDisabled не повинен періодично перехоплювати контакт під активним тестом. Cleanup має зберігати LOW, а не відновлювати попередній ненульовий запит.

Capture/core, D56 floating GPIO3, epoch/fault barriers та bounded diagnostics зберігаються. Розрахований duty/core permission не підміняються bench duty; adapter applied status не є фізичним вимірюванням. Точний контракт Phase1 після D59 — [pump-output.md](contracts/pump-output.md).

## RCH-04 — Дослідження LEDC

**Статус**: дослідження завершено, технічний вибір прийнятий у Plan за D60 після D59. Це не реалізація або доведений waveform pass.

**Рішення**: LEDC low-speed, timer0/channel0, GPIO0, APB80 МГц, 1000 Гц, 13 bits, hpoint0, без інверсії/fade, sleep disabled. APB/(1000×8192)=9,765625; divider з 8 fractional bits має точне ціле представлення2500. Арифметична похибка divider нульова; реальну точність clock перевіряють фізично. 14 bits тут не потрібні.

Для 1–99% `count=(percent*8192+50)/100`. Точки:25→2048,40→3277,50→4096,80→6554. Це відповідно25%,40,00244140625%,50%,80,0048828125%. Для0% — checked `ledc_stop(...,0)`; для100% — checked `ledc_stop(...,1)`, без програмування overflow/full-scale count8192. Повернення до PWM — checked `ledc_set_duty_with_hpoint` → checked `ledc_update_duty`; update знову вмикає output. API-return не доводить фізичний рівень чи наступний PWM-cycle.

**Єдиний owner та відмови**: fade service й інші LEDC clients для зарезервованого timer/channel заборонені. `ledc_set_duty_and_update` не підходить мінімальному шляху: pinned implementation створює fade resources і використовує semaphore wait. Окремі set/update не мають цього очікування за відсутнього fade state; це потребує config/source review. Жодні API не викликаються з ISR.

Перед пріоритетним LOW — зняти bench permission/зафіксувати terminal fault, відкинути старі запити; checked stop0. Якщо потрібний fallback: спочатку `gpio_set_level(GPIO0,0)`, потім checked `gpio_set_direction(...OUTPUT)` для від'єднання peripheral routing у simple GPIO LOW. Після fallback заборонені stale LEDC update/retry ненульового виходу в тому самому boot. `gpio_reset_pin()` не є LOW cleanup: він вмикає pull-up і вимикає output. Звичайний startup writer не викликає direction/config після LEDC takeover. Cleanup timer лише після LOW і pause; простіше утримувати один timer/channel протягом boot без автоматичного nonzero retry.

**Точні джерела в pinned SDK**:

- `components/soc/esp32c3/include/soc/soc_caps.h:228` — resources/clock/14-bit maximum.
- `components/hal/esp32c3/include/hal/ledc_ll.h:28` —8 fractional bits.
- `components/esp_driver_ledc/src/ledc.c:459,778,831,977,996,1035` — divider, config, update, stop, set; `:201,1307,1596` — fade/semaphores; `:741` — paused timer deconfigure.
- `components/esp_driver_gpio/src/gpio.c:222,315,462` — peripheral reroute/direction/reset_pin.
- `components/esp_driver_ledc/test_apps/ledc/main/test_ledc.c:142` — upstream GPIO-observed idle-level test; його наявність не є результатом нашого стенда.

Шляхи вище відносні до `build/scopa01-idf-sdk/`. [Офіційний LEDC guide C3 v5.5](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32c3/api-reference/peripherals/ledc.html) дає API-контекст; exact patch поведінку перевірено за локальним commit, не припущено зі stable docs.

**Альтернативи**: full-scale duty8192 відкинуто на користь явного idle HIGH; fade/ramping і semaphore-based update не потрібні. Другий output-owner або довільний GPIO reset створює конфлікт routing. Програмний snapshot/readback не замінює вимірювання.

## RCH-05 — Числові M04 межі до реалізації та вимірювань

За явним Plan-завданням технічний вибір закрито D60; повні нормативні параметри й методика — [контракт](contracts/pump-output.md). Нижче rationale, не вимірювання. Waveform:5 с initial LOW +6×10 с =65 с. Stress output:5 с+6×100 с=605 с; чинне evidence window за D62 —601 MCU-секунда після initial LOW, окремо підтверджується≥600 фізичних секунд. По одному run для active/slow/absent reader. Це одна скінченна послідовність, без loop. Accuracy instrumentation завершується раніше за чинним D55 і має окремий scope evidence.

- Частота: абсолютна E_f≤1 Гц від1000 Гц; U_f≤0,1 Гц окремо.
- Duty: E_d≤0,1 відсоткового пункту від nominal, U_d≤0,02 п.п. окремо. Максимальна 13-bit quantization≤0,006104п.п., тобто всередині запропонованого бюджету. Не усереднювати погані цикли для pass.
- Time sampling≤0,1 мкс (≥10MS/s), із документованою timebase accuracy та придатним voltage-capable методом для LOW/HIGH/startup. Сам цифровий аналізатор не доводить аналогові рівні.
- Кожне stable plateau≥1с із per-cycle перевіркою; повні endpoint plateaus без протилежних імпульсів у задекларованій detection resolution. Перехідні вікна окремо, без їх приховування середнім.
- MCU-domain budget: owner fault→checked LOW API≤1мс, scheduled deadline→checked API≤2мс. Окремо фізичні plateau intervals: E≤0,1% тривалості+2мс, U≤0,01% тривалості+0,1мс та analyzer GPIO3-stuck→GPIO0-LOW≤112мс. Абсолютну owner→physical2мс вимогу з попередніх кандидатів відкинуто: cross-domain offset не можна незалежно довести тими ж output transitions. Це виправлення методу до реалізації, не послаблення погодженого користувачем числового критерію (користувач доручив M04 Plan).
- Host fixtures мають охопити endpoints/API selection,1..99% округлення,>100 rejection, simultaneous fault/stale request, partial API failures, HIGH→PWM→faultLOW, fade/routing conflicts. Фізична перевірка цих результатів окрема.

## RCH-06 — Інструментування, межі доказів та alternatives

**Рішення**: M04 static voltage LOW≤0,8В і HIGH2,4–3,6В з U_V≤0,05В та guard-banded voltage comparisons; detection≥1мкс. Обраний HIGH має запас відносно1,5В порогу локального MD20A rev1.0, не припускає придатність невідомої ревізії. ADC/voltage-capable прилад і реальний power/bias/ground перевіряються до physical gate; конкретна модель приладу не вигадується. Digital timing acquisition та analog voltage/transient method — різні докази.

**Рішення**: bounded64×40B OutputEvidence після freeze, sole USB export, без new marker GPIO. API/fault/deadline timestamps дають тільки MCU-domain budget. Output transitions не можуть самі калібрувати unknown offset для доведення абсолютної physical latency: спільний delay може сховатися. Тому приймаються незалежні direct analyzer112мс/plateau метрики; cross-domain pass без незалежного anchor заблокований і не вимагається як заявлена властивість04. Прямий analyzer stuck→LOW112мс лишається незалежним physical check. Serial print timestamps не використовуються як physical fault time. Для reset не намагаємося визначати граничну мить за журналом USB.

**Рішення**: не дублювати capture owner, не міняти02 ControlCore API, D56GPIO3 або capture budgets/thresholds/queues. Output service вставляється у той самий serialized owner, між обмеженими batch/fault/recovery операціями. APB/no fade/без per-cycle update дають мінімальний steady-state overhead; actual schedule/bounds потребують перевірки.

**Рішення**: новий компактний rc04 snapshot grammar з явними bench/core fields зберігає transport384B; all fields hexadecimal з допустимими enum/mask bounds. Форматна арифметика дає383B worst case до padding; це не тест майбутнього formatter. Boundary tests та actual map потрібні при реалізації.

**Відхилено**: друк на кожному PWM-edge, довільний USB command, непогоджений GPIO-marker, багатозадачний LEDC access, усереднення для physical pass, повтори тесту в одному boot, фіктивні hardware passes через host/build. Жодне з рішень не встановлює operational timeout/Q05 або не приймає Q02/Q03/Q06.

Review перед завершенням Phase1: уточнено sticky-fault/API linearization та post-call recheck без shared lock навколоSDK. OutputEvidence розширено до raw sdk_status кожної операції: native≤40B/canonical36B,64records+header≤2688B; разом із03trace≤9088B. Форматна межаrc04 лишилась383B. Самокалібрування clock offset через припущений physical delay відкинуто, software/physical критерії розділено.

Для фізичних plateau intervals враховано0,1% clock tolerance узгоджено з PWM1000±1Гц плюс2мс scheduler allowance; E_T≤0,001×T_nom+2000мкс, U_T≤0,0001×T_nom+100мкс окремо. Абсолютні MCU-domain API/deadline межі1/2мс лишаються незмінними. Це уникає прихованого20ppm clock requirement на100с plateau.

Checklist-guided уточнив вимоги без нових hardware фактів: Fault видимий і в normal; sticky ordering охоплює staticHIGH; recorder overflow не може перешкодити LOW або рекурсивно логувати себе; freeze потребує checked LOW, а LowUnconfirmed не є успішним stop. Нормативна схема містить stable wire codes/per-operation statuses/initial physical states.112мс вимірюється до сталого LOW, не короткої фази PWM;10с terminal observation узгоджене з наявним P2/P5 no-repeat guard. Технічний M04 закритий як дизайн, його фізична перевірка відкрита.

## RCH-07 — Докази stress-вікна за відсутнього reader

Користувач обрав A / D61 під час checklist-guided 2026-10-30: bounded RAM summary, capture після вікна продовжується. Відхилена альтернатива B — зупинка capture для stress export. Latest-only snapshots самі не зберігають точні bounds/counts600с; їх друк після reconnect не є достатнім доказом. Окремий CaptureWindowEvidence≤512B зберігає належність за estimated falling, boundary provenance, window counts і maxima без per-pulse history. Для здорового valid потоку closure guard17мс складається з чинних stop5мс/callback-rearm1мс/processing10мс та owner poll1мс; підтвердження ingress barrier обов’язкове, самого спливу часу недостатньо. Capture budgets03 не змінюються. Це рішення й технічний дизайн до реалізації, без нових фізичних результатів.


## RCH-08 — Аналіз U1, рішення A / D62

2026-10-30 користувач обрав A: зберегти output schedule D60, подовжити лише bounded RAM window до 601 MCU-секунди [t0+5 с,t0+606 с). Це замінює первісні 600 MCU-секундного bounds у D61/RCH-07. Наприклад, гіпотетичний MCU-clock на 0,1% швидший дає близько 599,4 фізичних секунд за 600 MCU-секунд; це арифметичний приклад, не виміряна похибка плати. Запас 601 сам не гарантує прийняття: консервативний інтервал між first/last matched falling мінус невизначеність має бути≥600 фізичних секунд.

На 605 с output Complete/LOW, collector продовжує збір та max diagnostics до 606 с, closure≤17 мс від нової межі; export чекає freeze. RAM≤512B/transport/one writer/clock domain не змінюються. Відхилено B — звузити 04 до номінальних 600 MCU-секунд; C — відкласти рішення. Фізичні результати відсутні, Q02/Q03/Q06 відкриті.
