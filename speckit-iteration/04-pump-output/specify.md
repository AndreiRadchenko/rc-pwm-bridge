# Specify prompt — 04-pump-output

Read and apply `speckit-iteration/current-baseline.md`, including every referenced current source, inherited contract and evidence boundary. It is required input for this prompt. The ratified constitution, current user instructions and decisions govern; do not reselect settled parameters or infer physical acceptance from completed software. Generate documentation in Ukrainian.

Target iteration `04-pump-output`. Requirement traceability: R01,R04,R06,R08,R12. Prerequisites: 01-foundation,02-control-core,03-rc-capture. Questions to resolve or explicitly scope: remaining Q02 power/bias/startup evidence and Q06 MD20A revision/interface/jumper; inherit selected Q04 policies and pending physical capture evidence. Q05 belongs to 05 and must not be bypassed.
Set `SPECIFY_FEATURE_DIRECTORY=specs/04-pump-output` explicitly for every workflow script in this phase; confirm returned paths belong to this iteration. The prompt directory is input, not generated feature artifacts. Preserve existing artifacts and user changes. During Specify, create/switch to `04-pump-output` under AGENTS.md/D10 while preserving user work; record the explicit exception to the older constitutional branch prohibition. Do not commit or push. Run only this requested phase; do not invoke other phases, access/flash hardware or operate loads.

## Intended result

Generate and measure the MD20A control waveform with an isolated no-pump test setup before operational arming exists.

Reuse the selected SuperMini profile: PWM=GPIO0, RC=GPIO3, permanent DIR–GND jumper at MD20A, no MCU DIR pin/control. Verify the driver revision, electrical interface, grounding and jumper without the pump (D20/D21); motor direction is a 06 test. Retain the 02/03 mapping, validation, fault latch and bounded diagnostics. A calculated RC duty is not output permission.

Define a bounded bench-only waveform-test mode with explicit entry/exit, fault-to-LOW behavior and no pump attached. Normal startup remains LOW/disarmed; neither RC nor USB diagnostics may select this test mode or authorize production output. The test path must make six nominal duty points and endpoints observable without adding a runtime command source.

At intermediate duty generate 1000 Hz PWM. Zero is continuous LOW and full duty is the supported continuous HIGH, with fixed direction. Startup, disarmed and faults select LOW. MD20A LOW means brake, not coast or power isolation. Do not enable automatic RC-to-pump operation before 05.

## Acceptance and boundaries

Define user stories, scope/non-goals, observable acceptance criteria, failure/recovery scenarios, dependencies and requirement-ID traceability. Resolve product choices with the user; do not convert historical suggestions into approved defaults. Unrelated future questions must not block this iteration.

Cover these acceptance scenarios: Meaningful adapter/core tests for changed behavior and a pinned target build; measure frequency and all six nominal duty points with a logic analyzer; verify static endpoints, the permanent MD20A DIR–GND jumper (not motor direction) and startup/reset/bootloader/upload traces; assign brownout/watchdog coverage to the relevant 05/06 gates. Document physical tolerances before testing and distinguish disconnected-driver vs powered-driver observations.

Write `specs/04-pump-output/spec.md` using the installed Spec Kit workflow. Report blockers and pending physical evidence honestly. Stop after specification; Plan is a separate explicit invocation.
