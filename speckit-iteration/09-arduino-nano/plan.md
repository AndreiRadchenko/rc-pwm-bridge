# Plan prompt — 09-arduino-nano

Read and apply `speckit-iteration/current-baseline.md`, including every referenced current source, inherited contract and evidence boundary. It is required input for this prompt. The ratified constitution, current user instructions and decisions govern; do not reselect settled parameters or infer physical acceptance from completed software. Generate documentation in Ukrainian.

Target iteration `09-arduino-nano`. Requirement traceability: R03,R08,R12,R15. Prerequisites: 08-esp32-portability. Questions to resolve or explicitly scope: Q09; platform-specific Q02,Q03,Q06.
Set `SPECIFY_FEATURE_DIRECTORY=specs/09-arduino-nano` explicitly for every workflow script in this phase; confirm returned paths belong to this iteration. The prompt directory is input, not generated feature artifacts. Preserve existing artifacts and user changes. Do not create/switch Git branches during Plan, commit or push. Run only this requested phase; do not invoke other phases, access/flash hardware or operate loads.
This is a deferred roadmap extension. Reading this prompt does not activate it; require an explicit task for this iteration and accepted prerequisites.

Read the accepted `specs/09-arduino-nano/spec.md` and accepted prerequisite artifacts before planning. If the spec does not exist or critical behavior is unresolved, report the missing input rather than generating a plan for a different active feature.

## Implementation direction

Deferred extension: port the accepted behavior to an explicitly identified Arduino Nano model using a separate build and hardware adapters.

Select and pin the Nano build system/toolchain for the chosen board. Reuse the standard-C++ core without ESP-IDF dependencies. Allocate capture interrupts/timers and 1000 Hz PWM, account for shared timer conflicts, monotonic time/wrap, ISR latency, atomic access, RAM/flash and logic levels. Preserve the accepted per-channel Test/Fly loss and recovery policy rather than imposing universal fail-off; do not replace hardware PWM with blocking pulse measurement.

Preserve the accepted behavior: Do not assume Nano means ATmega328P; identify the exact model first. Preserve the two-channel behavior selected in 07, including cutoff and recovery, while retaining native ESP-IDF for ESP32. If hardware constraints prevent equivalence, expose and resolve the limitation rather than silently degrading behavior.

Read the accepted 04–08 contracts/evidence at invocation time. Reuse the strict C++11 portable core; keep IDF/FreeRTOS, RMT/LEDC and GPIO details in ESP32 adapters. Translate accepted monotonic microsecond, per-channel validation/fault/cutoff and bounded diagnostic contracts to Nano-specific adapters without assuming C3 pins or clock width. Preserve 1000 Hz output with static LOW/HIGH endpoints and the MD20A hardware DIR–GND jumper. If the chosen Nano cannot meet the accepted behavior/timing/resource budget, record a blocker or an explicit approved scope change before implementation.

## Validation and deliverables

Shared core fixtures on host and relevant target; build all supported boards; document RAM/flash budget; measure capture/duty/frequency/endpoints, two-channel load, timer wrap, cutoff, diagnostics pressure, watchdog/reset and electrical startup behavior on Nano.

Produce `specs/09-arduino-nano/plan.md` and necessary research/interface/data-model/quickstart artifacts through the installed workflow. Keep contracts minimal and relevant to embedded boundaries; do not invent web APIs or network schemas. State versions/dependencies, module boundaries and data flow, startup/failure order, compatibility effects and constitution compliance. Separate host tests, target builds and operator hardware steps. Resolve blocking technical questions and list remaining physical acceptance inputs without claiming tests have passed. Stop after planning; do not generate tasks or implement automatically.
