# Plan prompt — 04-pump-output

Read `.specify/memory/constitution.md`, `docs/decisions.md`, `docs/requirements.md`, `docs/architecture.md`, `docs/roadmap.md`, `docs/open-questions.md` and `hardware/README.md`. Confirm the constitution has been filled and adopted; a stock placeholder is not an approved constitution. Treat `PROJECT_FOUNDATION.md` as historical context; current decisions supersede its WROOM/two-channel-start proposals.

Target iteration `04-pump-output`. Requirement traceability: R01,R04,R06,R08,R12. Prerequisites: 01-foundation,02-control-core,03-rc-capture. Questions to resolve or explicitly scope: Q02,Q04,Q06; Q05 must not be bypassed.
Set `SPECIFY_FEATURE_DIRECTORY=specs/04-pump-output` explicitly for all workflow scripts, including setup-plan; confirm returned paths belong to this iteration. The prompt directory is input, not generated feature artifacts. Preserve existing artifacts and user changes. Do not create/switch Git branches or commit automatically. Do not run Tasks or Implement, flash hardware or operate loads.

Read the accepted `specs/04-pump-output/spec.md` and accepted prerequisite artifacts before planning. If the spec does not exist or critical behavior is unresolved, report the missing input rather than generating a plan for a different active feature.

## Implementation direction

Generate and measure the MD20A control waveform with an isolated no-pump test setup before operational arming exists.

Implement the ESP32-C3 LEDC adapter with a documented resolution, clock and SDK-compatible full-scale handling. Keep DIR permanently jumpered to GND on MD20A; allocate no MCU DIR pin and implement no direction control. Verify the jumper without the pump in 04; actual pump direction belongs to physical acceptance in 06. Do not add reversal. Define a bounded, explicitly isolated waveform-test path for acceptance with no pump attached; it must not become a runtime command override or default startup mode.

Preserve the accepted behavior: At intermediate duty generate 1000 Hz PWM. Zero is continuous LOW and full duty is the supported continuous HIGH, with fixed direction. Startup, disarmed and faults select LOW. MD20A LOW means brake, not coast or power isolation. Do not enable automatic RC-to-pump operation before 05.

## Validation and deliverables

Build plus adapter/core tests where meaningful; measure frequency and all six nominal duty points with a logic analyzer; verify static endpoints, direction and boot/reset/upload traces. Document physical tolerances before testing and distinguish disconnected-driver vs powered-driver observations.

Produce `specs/04-pump-output/plan.md` and necessary research/interface/data-model/quickstart artifacts through the installed workflow. Keep contracts minimal and relevant to embedded boundaries; do not invent web APIs or network schemas. State versions/dependencies, module boundaries and data flow, startup/failure order, compatibility effects and constitution compliance. Separate host tests, target builds and operator hardware steps. Resolve blocking technical questions and list remaining physical acceptance inputs without claiming tests have passed. Stop after planning; do not generate tasks or implement automatically.
