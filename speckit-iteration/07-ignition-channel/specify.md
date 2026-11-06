# Specify prompt — 07-ignition-channel

Read and apply `speckit-iteration/current-baseline.md`, including every referenced current source, inherited contract and evidence boundary. It is required input for this prompt. The ratified constitution, current user instructions and decisions govern; do not reselect settled parameters or infer physical acceptance from completed software. Generate documentation in Ukrainian.

Target iteration `07-ignition-channel`. Requirement traceability: R03,R08,R09,R12,R13. Prerequisites: 06-pump-validation. Questions to resolve or explicitly scope: Q07; revisit Q02,Q04,Q05,Q06 for two channels.
Set `SPECIFY_FEATURE_DIRECTORY=specs/07-ignition-channel` explicitly for every workflow script in this phase; confirm returned paths belong to this iteration. The prompt directory is input, not generated feature artifacts. Preserve existing artifacts and user changes. During Specify, create/switch to `07-ignition-channel` under AGENTS.md/D10 while preserving user work; record the explicit exception to the older constitutional branch prohibition. Do not commit or push. Run only this requested phase; do not invoke other phases, access/flash hardware or operate loads.
This is a deferred roadmap extension. Reading this prompt does not activate it; require an explicit task for this iteration and accepted prerequisites.

## Intended result

Deferred extension: add a second independent RC input for ignition/glow-plug ON/OFF, preserving the accepted pump channel.

Identify whether ignition is direct plug power, ignition-unit supply or a control input before selecting its output. Agree thresholds/hysteresis, power/interface ratings and isolation needs, per-channel freshness and cross-channel fault/recovery rules. Startup in the hysteresis band stays OFF; a fault must not retain ON. Historical 1400/1600 us and shared cutoff are proposals only. Do not introduce automatic ignition sequencing.

Read the accepted 04–06 specs/contracts and acceptance evidence at invocation time. Preserve pump width/period validation, estimated-edge freshness, mapping/rounding, diagnostic fault history, accepted D64/D65 operational Test/Fly and automatic recovery policy, and bounded diagnostics. Do not turn historical pump signal-loss latches into permanent output inhibition or copy ignition OFF-on-fault into Fly pump behavior. Use one monotonic domain for both channels; extend independent status/counters without turning diagnostics into per-edge history. Check simultaneous capture/timing/loss under maximum diagnostics; add pins/resources only through the updated exact-board profile. Do not assign ignition the pump's validity/arming thresholds by assumption. Pump DIR remains a hardware GND jumper.

## Acceptance and boundaries

Define user stories, scope/non-goals, observable acceptance criteria, failure/recovery scenarios, dependencies and requirement-ID traceability. Resolve product choices with the user; do not convert historical suggestions into approved defaults. Unrelated future questions must not block this iteration.

Cover these acceptance scenarios: Regression suite for pump; ignition boundaries and startup band; both inputs active, either/both lost, independent freshness and chosen cross-channel policy; boot/reset/recovery of both physical commands. Separate interface-only tests from actual ignition-load acceptance.

Write `specs/07-ignition-channel/spec.md` using the installed Spec Kit workflow. Report blockers and pending physical evidence honestly. Stop after specification; Plan is a separate explicit invocation.
