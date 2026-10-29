# scripts

`build.py` збирає host або target у новому каталозі, `environment/check.py` перевіряє зафіксоване середовище, `verify/config.py` перевіряє target artifacts. Фізичних дій ці команди не виконують; подробиці — [quickstart](../specs/01-foundation/quickstart.md).
Скрипти Spec Kit розміщено в `.specify/scripts/bash/`.

## Підготовка feature-гілки

З `Lora_clone` перенесено `create-new-feature.sh` та потрібний йому `common.sh`.
`check-prerequisites.sh`, `setup-plan.sh` і `setup-tasks.sh` побайтово збігаються
з джерелом, тому їх не замінювали. `common.sh` скопійовано без змін; у
`create-new-feature.sh` замінено базу `rebuild/espnow-v2` на
`SPECIFY_BASE_BRANCH`, типово `main`.

Скрипт підтримує окремі імена каталогу й Git-гілки, зберігає їхній зв'язок у
`.specify/feature.json`, не перезаписує наявний `spec.md` та відмовляє в перемиканні
гілки за незбережених змін. Це локальний helper, а не встановлений Git extension
чи автоматичний hook Specify. Викликати його лише за відповідним завданням користувача.

Потрібна наявна локальна базова гілка з комітом. Якщо Git ще не має початкового
коміту, скрипт не створює його самостійно. Підготовку й коміт наявної роботи
потрібно виконувати за окремим завданням; не застосовувати stash/reset/clean
для обходу перевірки робочого дерева.

Для вже створеної специфікації 01 спочатку виконати попередню перевірку:

```sh
SPECIFY_BASE_BRANCH=main SPECIFY_FEATURE_DIRECTORY=specs/01-foundation \
  bash .specify/scripts/bash/create-new-feature.sh \
  --json --dry-run --allow-existing-branch '01 foundation'
```

За явним завданням на створення/перемикання гілки, після виконання передумов:

```sh
SPECIFY_BASE_BRANCH=main SPECIFY_FEATURE_DIRECTORY=specs/01-foundation \
  bash .specify/scripts/bash/create-new-feature.sh \
  --json --allow-existing-branch '01 foundation'
```

Очікувані гілка й каталог: `01-foundation` та `specs/01-foundation`.
Параметр `--branch-name` дозволяє інше ім'я гілки без зміни каталогу;
`--feature-directory` явно перевизначає каталог. Перевіряти шляхи у JSON.
Скрипт не виконує commit/push, Plan/Tasks/Implement, прошивання чи керування навантаженням.

Синтаксис п'яти bash-скриптів і поведінку перенесеного helper перевірено в тимчасових
Git-репозиторіях: dry-run без змін, створення гілки, відновлення без перезапису spec,
явний вибір каталогу/гілки, захист від брудного дерева та виходу за `specs/`, інша
базова гілка й відсутня база. Ці перевірки не є тестами firmware.

Див. [roadmap](../docs/roadmap.md).


## Перевірки03

`build.py target --rc03-bench diagnostics|accuracy` додає відповідний defaults fragment до baseline, не змінюючи його. Normal запускається без flag; на host цей flag відхиляється. Потрібен новий build directory, наявні результати не видаляються. `verify/config.py` перевіряє effective IRAM/cache-safe/PM/radio settings, actual capture sources та trace config/object match, strict C++11 усіх core objects. Manual ISR call graph/data placement review — окремо; build не є latency proof.

`verify/rc_capture_trace.py --trace <archive.txt> --analyzer <edges.csv> --u-width-us <U> --u-interval-us <U> --report <report.json>` — offline без serial/плати. CSV: `pulse_start_ns,pulse_end_ns`, integer ns. Exit0=pass,1=fail,2=blocked. Зберігає hashes, всі per-sample/per-interval errors без округлення/average/clock correction; U≤1мкс окремо та direct E≤5/10мкс. Формат bounded archive/діагностики й три build commands — [quickstart03](../specs/03-rc-capture/quickstart.md). Програмні докази — [verification03](../specs/03-rc-capture/verification/acceptance.md), physical not_run.
