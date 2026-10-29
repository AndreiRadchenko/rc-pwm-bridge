# Перевірка якості специфікації: 03-rc-capture

**Мета**: повнота вимог перед завершенням Plan.  
**Створено**: 2026-10-28 | **Оновлено**: 2026-10-29, D51/D52  
**Специфікація**: [spec.md](../spec.md)

## Якість і межі

- [x] Сценарії, обсяг, non-goals, залежності 01/02 і R05/R09/R11/R12 визначено.
- [x] Поточні вимоги відокремлено від історичних уточнень; D51 явно замінює hardware timestamp і 3 мс HIGH.
- [x] Q05 operational cutoff/arming/recovery не отримали припущених defaults; фактичний вихід 0%.

## Повнота вимог

- [x] Width/period межі, invalid, last-valid, startup, fault latch і resync мають спостережувані критерії.
- [x] Спільний поріг 100 мс визначено; D52 задає додатковий бюджет fault ≤10 мс; текст не визначає deadline core.
- [x] Діагностика D43, bounded snapshot, drop/counters і reconnect визначені.
- [x] Похибки 5/10 мкс і 0 втрат/10 хв лишаються тестовими критеріями; програмні timestamps не позначено доведеними.
- [x] Host fixtures, target build та physical evidence розділені; Q03/H02/H03 і receiver gate збережено.

## Готовність

- [x] Вимоги дозволяють продовжити Plan за напрямком RMT + GPIO ISR без пошуку hardware latch.
- [x] Plan завершено: RX/rearm, асоціація, черги, fault budget визначені; research/data-model/contract/quickstart створено, конституційні gates переглянуто.
- [ ] Фізичне прийняття: Q03, прилад, вимірювання timing/loss і gates 01 залишаються відкритими.

Цей checklist перевіряє документацію. Жодних нових host/target/physical тестів при зміні вимог не виконано; Tasks/Implement не запускались.


### Доповнення analyze-guided C1-A / D55

Вимірюваність SC-003 конкретизовано: усі pulses/intervals у коротких bounded RAM windows порівнюються offline після freeze/stop/export; окремі600с runs перевіряють втрати та часові бюджети. Production diagnostics latest-only не є per-pulse evidence. D54/B незмінне: directE≤5/10мкс, U≤1мкс окремо. Tasks T052–T054 додані зі стабільними IDs; фізичних результатів немає. Це доповнення наявних CHK025/026, не перевірка реалізації чи новий hardware pass.
