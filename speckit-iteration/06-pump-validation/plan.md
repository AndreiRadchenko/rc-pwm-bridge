# Plan prompt — 06-pump-validation

Read and apply `speckit-iteration/current-baseline.md`, including every referenced current source, inherited contract and evidence boundary. It is required input for this prompt. The ratified constitution, current user instructions and decisions govern; do not reselect settled parameters or infer physical acceptance from completed software. Generate documentation in Ukrainian.

Target iteration `06-pump-validation`. Requirement traceability: R01,R04,R05,R06,R07,R08,R09,R10,R11,R12. Prerequisites: 01-foundation,02-control-core,03-rc-capture,04-pump-output,05-failsafe-arming. Questions to resolve or explicitly scope: Q03,Q06 and all unresolved v1 acceptance parameters.
Set `SPECIFY_FEATURE_DIRECTORY=specs/06-pump-validation` explicitly for every workflow script in this phase; confirm returned paths belong to this iteration. The prompt directory is input, not generated feature artifacts. Preserve existing artifacts and user changes. Do not create/switch Git branches during Plan, commit or push. Run only this requested phase; do not invoke other phases, access/flash hardware or operate loads.

Read the accepted `specs/06-pump-validation/spec.md` and accepted prerequisite artifacts before planning. If the spec does not exist or critical behavior is unresolved, report the missing input rather than generating a plan for a different active feature.

## Implementation direction

Validate the accepted single-channel firmware, actual MD20A and pump in a staged controlled bench process, with honest separation of software, build and physical evidence.

Produce an operator procedure and evidence format recording firmware revision, board/driver identity, wiring, instruments, limits, ambient conditions, durations and observed results. Establish current/temperature/timing limits before tests using exact hardware documentation. Verify independent emergency power cutoff and reset biases; no uncontrolled stall testing, automatic flashing or unattended hazardous load activation.

Build an acceptance matrix from current 01–05 evidence, identifying each pending gate and its owner before any load step. Reuse the Q05 decisions and 04 output tolerances; preserve D53–D55 capture accuracy/loss methodology for affected physical regressions. Verify the permanent DIR–GND jumper and actual motor polarity rather than configuring a DIR GPIO. Resolve the real 3.3 V supply, USB/external-power mode, PWM bias, MD20A revision/interface and actual receiver conditioning before the corresponding connection. The planned HJ divider and operator serial sweep are not substitutes for electrical/analyzer acceptance. Plan hazardous power-cycle/upload/brownout cases with explicit load state and independent cutoff; a procedure does not authorize its execution.

Preserve the accepted behavior: First verify no-load signals and power/protection, then progress through a suitable dummy load and controlled pump tests. Check actual receiver failsafe including valid-looking pulses after radio loss. Document supply sag, startup, transients, noise, current and thermal behavior at representative 40/80 percent and expected normal load.

## Validation and deliverables

Repeat host suite and target build, then staged physical scenarios: startup/reset/upload/brownout, loss/invalid/recovery, emergency cutoff, pump direction, endpoint action, representative thermal endurance and receiver/EMI behavior. Do not infer motor current from duty or treat brake as proven zero fuel flow. Mark unavailable scenarios pending.

Produce `specs/06-pump-validation/plan.md` and necessary research/interface/data-model/quickstart artifacts through the installed workflow. Keep contracts minimal and relevant to embedded boundaries; do not invent web APIs or network schemas. State versions/dependencies, module boundaries and data flow, startup/failure order, compatibility effects and constitution compliance. Separate host tests, target builds and operator hardware steps. Resolve blocking technical questions and list remaining physical acceptance inputs without claiming tests have passed. Stop after planning; do not generate tasks or implement automatically.


Inherit D63-D65 and the accepted 05 implementation: explicit build-selected Test/Fly, default Test, transparent current-boot valid commands and automatic reconnect-high recovery without a zero handshake. Test cutoff and Fly long input-loss hold need distinct acceptance records, including no-prior-valid/reset and held endpoint cases. Do not let historical capture latches or generic fail-off assumptions silently replace Fly. Confirm formal constitution/template synchronization before implementing changed behavior; this prompt does not authorize that phase. ArduPilot/Matek output behavior and levels require actual source characterization; receiver/radio-link tests apply before that receiver is used, while converter PWM freshness alone does not establish radio-link health.
