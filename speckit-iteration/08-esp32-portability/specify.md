# Specify prompt — 08-esp32-portability

Read and apply `speckit-iteration/current-baseline.md`, including every referenced current source, inherited contract and evidence boundary. It is required input for this prompt. The ratified constitution, current user instructions and decisions govern; do not reselect settled parameters or infer physical acceptance from completed software. Generate documentation in Ukrainian.

Target iteration `08-esp32-portability`. Requirement traceability: R02,R03,R08,R12,R14. Prerequisites: 07-ignition-channel. Questions to resolve or explicitly scope: Q08; platform-specific Q02,Q03,Q06.
Set `SPECIFY_FEATURE_DIRECTORY=specs/08-esp32-portability` explicitly for every workflow script in this phase; confirm returned paths belong to this iteration. The prompt directory is input, not generated feature artifacts. Preserve existing artifacts and user changes. During Specify, create/switch to `08-esp32-portability` under AGENTS.md/D10 while preserving user work; record the explicit exception to the older constitutional branch prohibition. Do not commit or push. Run only this requested phase; do not invoke other phases, access/flash hardware or operate loads.
This is a deferred roadmap extension. Reading this prompt does not activate it; require an explicit task for this iteration and accepted prerequisites.

## Intended result

Deferred extension: support one explicitly selected additional ESP32 board without changing accepted core behavior.

Name the exact SoC, board and revision, supported channel count and electrical interface. Preserve mapping, arming, faults and diagnostic semantics. Recheck peripheral availability instead of assuming C3 and other ESP32 chips are identical.

Read the accepted 04–07 contracts/evidence at invocation time. Keep the SuperMini profile and pinned C3 build as regression baselines; do not copy GPIO0/3, GPIO pull configuration or C3 resource counts onto the new board. Preserve the accepted per-channel time/validation/fault/cutoff and diagnostic contracts through explicit adapters. Verify both-channel resource and timing budgets selected in 07. Any SDK/toolchain change must be justified, pinned and checked against the existing C3 environment; a new platform does not relax core behavior.

## Acceptance and boundaries

Define user stories, scope/non-goals, observable acceptance criteria, failure/recovery scenarios, dependencies and requirement-ID traceability. Resolve product choices with the user; do not convert historical suggestions into approved defaults. Unrelated future questions must not block this iteration.

Cover these acceptance scenarios: Shared behavior fixtures and host suite; reproducible builds for C3 and the chosen target; physical timing/endpoints/capture/loss/recovery/boot/EMI acceptance for the new board. Compile success alone is not supported-hardware acceptance.

Write `specs/08-esp32-portability/spec.md` using the installed Spec Kit workflow. Report blockers and pending physical evidence honestly. Stop after specification; Plan is a separate explicit invocation.
