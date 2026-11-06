# Plan prompt — 04-pump-output

Read and apply `speckit-iteration/current-baseline.md`, including every referenced current source, inherited contract and evidence boundary. It is required input for this prompt. The ratified constitution, current user instructions and decisions govern; do not reselect settled parameters or infer physical acceptance from completed software. Generate documentation in Ukrainian.

Target iteration `04-pump-output`. Requirement traceability: R01,R04,R06,R08,R12. Prerequisites: 01-foundation,02-control-core,03-rc-capture. Questions to resolve or explicitly scope: remaining Q02 power/bias/startup evidence and Q06 MD20A revision/interface/jumper; inherit selected Q04 policies and pending physical capture evidence. Q05 belongs to 05 and must not be bypassed.
Set `SPECIFY_FEATURE_DIRECTORY=specs/04-pump-output` explicitly for every workflow script in this phase; confirm returned paths belong to this iteration. The prompt directory is input, not generated feature artifacts. Preserve existing artifacts and user changes. Do not create/switch Git branches during Plan, commit or push. Run only this requested phase; do not invoke other phases, access/flash hardware or operate loads.

Read the accepted `specs/04-pump-output/spec.md` and accepted prerequisite artifacts before planning. If the spec does not exist or critical behavior is unresolved, report the missing input rather than generating a plan for a different active feature.

## Implementation direction

Generate and measure the MD20A control waveform with an isolated no-pump test setup before operational arming exists.

Implement the ESP32-C3 LEDC adapter with a documented resolution, clock and SDK-compatible full-scale handling. Keep DIR permanently jumpered to GND on MD20A; allocate no MCU DIR pin and implement no direction control. Verify the jumper without the pump in 04; actual pump direction belongs to physical acceptance in 06. Do not add reversal. Define a bounded, explicitly isolated waveform-test path for acceptance with no pump attached; it must not become a runtime command override or default startup mode.

Use GPIO0 from the existing profile and pinned ESP-IDF v5.5.6. Define GPIO-to-LEDC ownership, endpoint transitions, checked API failures, cleanup/retry and fault-to-LOW order so stale duty cannot reappear after failure. Retain the capture/core owner and D56 GPIO3 configuration; do not retune capture to accommodate output. Bench test selection is separate from production permission and has no USB/diagnostic command input. Define bench entry/exit and test build/config identity explicitly; normal startup stays LOW/disarmed.

Preserve the accepted behavior: At intermediate duty generate 1000 Hz PWM. Zero is continuous LOW and full duty is the supported continuous HIGH, with fixed direction. Startup, disarmed and faults select LOW. MD20A LOW means brake, not coast or power isolation. Do not enable automatic RC-to-pump operation before 05.

## Validation and deliverables

Meaningful adapter/core tests for changed behavior and a pinned target build; measure frequency and all six nominal duty points with a logic analyzer; verify static endpoints, the permanent MD20A DIR–GND jumper (not motor direction) and startup/reset/bootloader/upload traces; assign brownout/watchdog coverage to the relevant 05/06 gates. Document physical tolerances before testing and distinguish disconnected-driver vs powered-driver observations.

Produce `specs/04-pump-output/plan.md` and necessary research/interface/data-model/quickstart artifacts through the installed workflow. Keep contracts minimal and relevant to embedded boundaries; do not invent web APIs or network schemas. State versions/dependencies, module boundaries and data flow, startup/failure order, compatibility effects and constitution compliance. Separate host tests, target builds and operator hardware steps. Resolve blocking technical questions and list remaining physical acceptance inputs without claiming tests have passed. Stop after planning; do not generate tasks or implement automatically.
