# Specify prompt — 09-arduino-nano

Read `.specify/memory/constitution.md`, `docs/decisions.md`, `docs/requirements.md`, `docs/architecture.md`, `docs/roadmap.md`, `docs/open-questions.md` and `hardware/README.md`. Confirm the constitution has been filled and adopted; a stock placeholder is not an approved constitution. Treat `PROJECT_FOUNDATION.md` as historical context; current decisions supersede its WROOM/two-channel-start proposals.

Target iteration `09-arduino-nano`. Requirement traceability: R03,R08,R12,R15. Prerequisites: 08-esp32-portability. Questions to resolve or explicitly scope: Q09; platform-specific Q02,Q03,Q06.
Set `SPECIFY_FEATURE_DIRECTORY=specs/09-arduino-nano` explicitly for all workflow scripts, including setup-plan; confirm returned paths belong to this iteration. The prompt directory is input, not generated feature artifacts. Preserve existing artifacts and user changes. Do not create/switch Git branches or commit automatically. Do not run Tasks or Implement, flash hardware or operate loads.
This is a deferred roadmap extension. Reading this prompt does not activate it; require an explicit task for this iteration and accepted prerequisites.

## Intended result

Deferred extension: port the accepted behavior to an explicitly identified Arduino Nano model using a separate build and hardware adapters.

Do not assume Nano means ATmega328P; identify the exact model first. Preserve the two-channel behavior selected in 07, including cutoff and recovery, while retaining native ESP-IDF for ESP32. If hardware constraints prevent equivalence, expose and resolve the limitation rather than silently degrading behavior.

## Acceptance and boundaries

Define user stories, scope/non-goals, observable acceptance criteria, failure/recovery scenarios, dependencies and requirement-ID traceability. Resolve product choices with the user; do not convert historical suggestions into approved defaults. Unrelated future questions must not block this iteration.

Cover these acceptance scenarios: Shared core fixtures on host and relevant target; build all supported boards; document RAM/flash budget; measure capture/duty/frequency/endpoints, two-channel load, timer wrap, cutoff, diagnostics pressure, watchdog/reset and electrical startup behavior on Nano.

Write `specs/09-arduino-nano/spec.md` using the installed Spec Kit workflow. Report blockers and pending physical evidence honestly. Stop after specification; Plan is a separate explicit invocation.
