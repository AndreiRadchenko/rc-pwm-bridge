# Plan prompt — 06-pump-validation

Read `.specify/memory/constitution.md`, `docs/decisions.md`, `docs/requirements.md`, `docs/architecture.md`, `docs/roadmap.md`, `docs/open-questions.md` and `hardware/README.md`. Confirm the constitution has been filled and adopted; a stock placeholder is not an approved constitution. Treat `PROJECT_FOUNDATION.md` as historical context; current decisions supersede its WROOM/two-channel-start proposals.

Target iteration `06-pump-validation`. Requirement traceability: R01,R04,R06,R07,R08,R10,R11,R12. Prerequisites: 01-foundation,02-control-core,03-rc-capture,04-pump-output,05-failsafe-arming. Questions to resolve or explicitly scope: Q03,Q06 and all unresolved v1 acceptance parameters.
Set `SPECIFY_FEATURE_DIRECTORY=specs/06-pump-validation` explicitly for all workflow scripts, including setup-plan; confirm returned paths belong to this iteration. The prompt directory is input, not generated feature artifacts. Preserve existing artifacts and user changes. Do not create/switch Git branches or commit automatically. Do not run Tasks or Implement, flash hardware or operate loads.

Read the accepted `specs/06-pump-validation/spec.md` and accepted prerequisite artifacts before planning. If the spec does not exist or critical behavior is unresolved, report the missing input rather than generating a plan for a different active feature.

## Implementation direction

Validate the accepted single-channel firmware, actual MD20A and pump in a staged controlled bench process, with honest separation of software, build and physical evidence.

Produce an operator procedure and evidence format recording firmware revision, board/driver identity, wiring, instruments, limits, ambient conditions, durations and observed results. Establish current/temperature/timing limits before tests using exact hardware documentation. Verify independent emergency power cutoff and reset biases; no uncontrolled stall testing, automatic flashing or unattended hazardous load activation.

Preserve the accepted behavior: First verify no-load signals and power/protection, then progress through a suitable dummy load and controlled pump tests. Check actual receiver failsafe including valid-looking pulses after radio loss. Document supply sag, startup, transients, noise, current and thermal behavior at representative 40/80 percent and expected normal load.

## Validation and deliverables

Repeat host suite and target build, then staged physical scenarios: startup/reset/upload/brownout, loss/invalid/recovery, emergency cutoff, pump direction, endpoint action, representative thermal endurance and receiver/EMI behavior. Do not infer motor current from duty or treat brake as proven zero fuel flow. Mark unavailable scenarios pending.

Produce `specs/06-pump-validation/plan.md` and necessary research/interface/data-model/quickstart artifacts through the installed workflow. Keep contracts minimal and relevant to embedded boundaries; do not invent web APIs or network schemas. State versions/dependencies, module boundaries and data flow, startup/failure order, compatibility effects and constitution compliance. Separate host tests, target builds and operator hardware steps. Resolve blocking technical questions and list remaining physical acceptance inputs without claiming tests have passed. Stop after planning; do not generate tasks or implement automatically.
