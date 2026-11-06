# Checklist якості специфікації: прозоре керування, Test/Fly та відновлення

**Мета**: перевірити повноту й якість специфікації перед окремою фазою Plan.\
**Створено**: 2026-11-06\
**Feature**: [spec.md](../spec.md)\
**Фаза**: quality validation усередині Specify; окрема фаза Checklist не запускалась.

## Якість змісту

- [x] Нову поведінку визначено як результати для оператора; не обрано нові API, структуру реалізації чи watchdog/retry механізм.
- [x] Описано цінність прозорої трансляції, Test cutoff, Fly hold і автоматичного recovery.
- [x] Документ українською, сценарії пояснюють спостережувані результати; терміни й фізичні межі розрізнені.
- [x] Усі обов'язкові розділи активного spec-template заповнено зі збереженням їхнього порядку.

## Повнота вимог

- [x] У spec немає невирішених маркерів NEEDS CLARIFICATION: відповіді C05-01–C05-03 записані D66–D68.
- [x] Вимоги однозначні в межах Specify; невибрані технічні параметри мають явне делегування Plan і строк до реалізації, без defaults.
- [x] Критерії вимірювані: age≥T, загальна Test межа1 с, точні mapping/validation/diagnostic/inherited physical limits; T та інші technical limits належать окремому Plan.
- [x] Успіх описаний через поведінку/сигнал/час і доказ, без прив'язки нового результату до конкретного API.
- [x] Acceptance охоплює обидві збірки, defaultTest, no-prior-valid, input loss, recovery-high, reset, technical failures/watchdog і diagnostics.
- [x] Граничні width/period/time/epoch/resync/overflow/old-sample,0/100% Fly, clock і stale-output випадки перелічені.
- [x] Один канал05 чітко відокремлено від двигуна, силового прийняття06 і відкладених07–09.
- [x] Залежності01–04, source contracts, Q02/Q03/Q06 та formal governance gate мають власників і строки, без припущення physical acceptance.

## Готовність feature

- [x] Усі17 FR і запитані11 R-ID мають US/SC, метод та evidence/status у traceability matrix.
- [x] П'ять історій охоплюють головні потоки, відмови й recovery; кожна має незалежний спосіб перевірки в межах своїх залежностей.
- [x] SC-001–SC-008 визначають перевірюваний майбутній результат; host/build/physical розділені, жодного непроведеного pass.
- [x] Spec не виконує технічний дизайн: успадковані stack/profile/timing constraints наведені як чинні обмеження, а нові selections залишені явно запитаному майбутньому Plan.

## Примітки перевірки

- Після відповідей користувача: Test physicalLOW≤1 с від accepted valid end; Plan обирає T/запас, точна межаage≥T (D66). Technical output/clock classes/reaction/automatic retry делеговані Plan (D67). Watchdog automatic reset stalled критичного циклу погоджений; coverage/числа/метод — Plan (D68).
- Невирішених продуктових уточнень Specify немає. Числа T/retry/watchdog/new-valid latency та протокол фізичних перевірок ще не вибрані й не оголошені затвердженими; це обов'язкові результати окремого Plan до залежної реалізації.
- Оновлення governance2026-11-06: D74 формально ухвалив конституціюv2.0.0 та синхронізував шаблони/настанови; G05 закрито. Старі D64/D65/D10 overrides лишаються історією; тепер це правила принципівII/V. Technical design D69–D73 не змінено, physical прийняття не додано.16/16 quality пунктів зберігають силу; це не виконання firmware tests.
- Physical Q02/H02/H03, Q03/Q06,03 T043–T047/T051 та04 T044–T050 не прийняті. HJ-only не закриває actual receiver/radio-loss gate. Зелений quality checklist не приймає firmware, стенд або ітерацію.
- Активний шаблон розв'язано штатним create-new-feature.sh через installed template resolution stack; використано .specify/templates/spec-template.md. Повернені FEATURE_DIR/SPEC_FILE належать05, .specify/feature.json вказує на05.
- Pre/post Specify hooks: .specify/extensions.yml відсутній, dispatch не потрібний. Інші фази не викликалися.
- Підсумок:16/16 пунктів якості виконано. Готово до окремо запитаного Plan з явними technical/governance/physical gates; реалізація наразі не дозволена цією фазою.
