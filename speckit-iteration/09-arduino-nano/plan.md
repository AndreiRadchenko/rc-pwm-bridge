# Plan prompt — 09-arduino-nano

Read `.specify/memory/constitution.md`, `docs/decisions.md`, `docs/requirements.md`, `docs/architecture.md`, `docs/roadmap.md`, `docs/open-questions.md` and `hardware/README.md`. Confirm the constitution has been filled and adopted; a stock placeholder is not an approved constitution. Treat `PROJECT_FOUNDATION.md` as historical context; current decisions supersede its WROOM/two-channel-start proposals.

Target iteration `09-arduino-nano`. Requirement traceability: R03,R08,R12,R15. Prerequisites: 08-esp32-portability. Questions to resolve or explicitly scope: Q09; platform-specific Q02,Q03,Q06.
Set `SPECIFY_FEATURE_DIRECTORY=specs/09-arduino-nano` explicitly for all workflow scripts, including setup-plan; confirm returned paths belong to this iteration. The prompt directory is input, not generated feature artifacts. Preserve existing artifacts and user changes. Do not create/switch Git branches or commit automatically. Do not run Tasks or Implement, flash hardware or operate loads.
This is a deferred roadmap extension. Reading this prompt does not activate it; require an explicit task for this iteration and accepted prerequisites.

Read the accepted `specs/09-arduino-nano/spec.md` and accepted prerequisite artifacts before planning. If the spec does not exist or critical behavior is unresolved, report the missing input rather than generating a plan for a different active feature.

## Implementation direction

Deferred extension: port the accepted behavior to an explicitly identified Arduino Nano model using a separate build and hardware adapters.

Select and pin the Nano build system/toolchain for the chosen board. Reuse the standard-C++ core without ESP-IDF dependencies. Allocate capture interrupts/timers and 1000 Hz PWM, account for shared timer conflicts, monotonic time/wrap, ISR latency, atomic access, RAM/flash and logic levels. Do not weaken fail-off or replace hardware PWM with blocking pulse measurement.

Preserve the accepted behavior: Do not assume Nano means ATmega328P; identify the exact model first. Preserve the two-channel behavior selected in 07, including cutoff and recovery, while retaining native ESP-IDF for ESP32. If hardware constraints prevent equivalence, expose and resolve the limitation rather than silently degrading behavior.

## Validation and deliverables

Shared core fixtures on host and relevant target; build all supported boards; document RAM/flash budget; measure capture/duty/frequency/endpoints, two-channel load, timer wrap, cutoff, diagnostics pressure, watchdog/reset and electrical startup behavior on Nano.

Produce `specs/09-arduino-nano/plan.md` and necessary research/interface/data-model/quickstart artifacts through the installed workflow. Keep contracts minimal and relevant to embedded boundaries; do not invent web APIs or network schemas. State versions/dependencies, module boundaries and data flow, startup/failure order, compatibility effects and constitution compliance. Separate host tests, target builds and operator hardware steps. Resolve blocking technical questions and list remaining physical acceptance inputs without claiming tests have passed. Stop after planning; do not generate tasks or implement automatically.
