# rc-pwm-bridge

Локальний міст RC-PWM для системи Scopa. Перша версія: **один вхід RC-PWM → бензонасос через Cytron MD20A, ESP32-C3, нативний ESP-IDF**. Майбутні розширення: другий вхід для запалювання ON/OFF, інші ESP32, Arduino Nano.

Стан: реалізовано startup-каркас 01; host-тести, чиста ESP32-C3 збірка й програмний config review виконані. Результати — [індекс перевірок](specs/01-foundation/verification/acceptance.md). Фізичні живлення/bias і startup waveform не прийняті, тому ітерація 01 залишається відкритою. Плата після окремо дозволеної SFDP-діагностики залишена в ROM download mode; нову прошивку не записували. [Конституцію v1.0.0](.specify/memory/constitution.md) ухвалено 2026-10-28.

## Початок роботи

1. Прочитати [архітектуру](docs/architecture.md), [roadmap](docs/roadmap.md), [вимоги](docs/requirements.md) і [відкриті питання](docs/open-questions.md).
2. Відкрити Codex у корені репозиторію. Spec Kit CLI **0.12.17**, інтеграція Codex зі skills, уже ініціалізований. Нові skills можуть потребувати нового сеансу Codex.
3. Прочитати ухвалену [конституцію](.specify/memory/constitution.md). [constitution-prompt.md](speckit-iteration/constitution-prompt.md) збережено як вхід первинного ухвалення; повторний `$speckit-constitution` потрібен лише за окремим завданням на зміну конституції.
4. За окремим запитом виконувати лише потрібну фазу за [інструкцією ітерацій](speckit-iteration/README.md), починаючи з `01-foundation`; явно задавати `SPECIFY_FEATURE_DIRECTORY`.

Встановлений upstream: [github/spec-kit](https://github.com/github/spec-kit). Ініціалізація: `specify init --here --integration codex --script sh --force`. Повторно її запускати для звичайної роботи не потрібно. Версія записана також у `.specify/integration.json`; оновлення CLI/шаблонів робити свідомо й перевіряти diff.

## Каталоги

| Каталог | Призначення |
|---|---|
| `docs/` | Архітектура, вимоги, roadmap, рішення й питання |
| `hardware/` | Локальні технічні документи та їх походження |
| `firmware/` | ESP-IDF startup-каркас і переносиме ядро |
| `tests/` | Host-тести ядра, політики й tooling |
| `scripts/` | Відтворювані перевірки та build tooling |
| `speckit-iteration/` | Вхідні промпти, не згенеровані специфікації |
| `specs/` | Результати команд Spec Kit по ітераціях |
| `.specify/` | Штатні скрипти й workflow Spec Kit, ухвалена конституція та синхронізовані з нею шаблони |
| `.agents/skills/` | Штатні команди Spec Kit для Codex |

[PROJECT_FOUNDATION.md](PROJECT_FOUNDATION.md) збережений без змін як історичне джерело. Актуальні уточнення мають пріоритет за [реєстром рішень](docs/decisions.md).
