# Specify prompt — 07-ignition-channel

Read `.specify/memory/constitution.md`, `docs/decisions.md`, `docs/requirements.md`, `docs/architecture.md`, `docs/roadmap.md`, `docs/open-questions.md` and `hardware/README.md`. Confirm the constitution has been filled and adopted; a stock placeholder is not an approved constitution. Treat `PROJECT_FOUNDATION.md` as historical context; current decisions supersede its WROOM/two-channel-start proposals.

Target iteration `07-ignition-channel`. Requirement traceability: R03,R08,R09,R12,R13. Prerequisites: 06-pump-validation. Questions to resolve or explicitly scope: Q07; revisit Q02,Q04,Q05,Q06 for two channels.
Set `SPECIFY_FEATURE_DIRECTORY=specs/07-ignition-channel` explicitly for all workflow scripts, including setup-plan; confirm returned paths belong to this iteration. The prompt directory is input, not generated feature artifacts. Preserve existing artifacts and user changes. Do not create/switch Git branches or commit automatically. Do not run Tasks or Implement, flash hardware or operate loads.
This is a deferred roadmap extension. Reading this prompt does not activate it; require an explicit task for this iteration and accepted prerequisites.

## Intended result

Deferred extension: add a second independent RC input for ignition/glow-plug ON/OFF, preserving the accepted pump channel.

Identify whether ignition is direct plug power, ignition-unit supply or a control input before selecting its output. Agree thresholds/hysteresis, power/interface ratings and isolation needs, per-channel freshness and cross-channel fault/recovery rules. Startup in the hysteresis band stays OFF; a fault must not retain ON. Historical 1400/1600 us and shared cutoff are proposals only. Do not introduce automatic ignition sequencing.

## Acceptance and boundaries

Define user stories, scope/non-goals, observable acceptance criteria, failure/recovery scenarios, dependencies and requirement-ID traceability. Resolve product choices with the user; do not convert historical suggestions into approved defaults. Unrelated future questions must not block this iteration.

Cover these acceptance scenarios: Regression suite for pump; ignition boundaries and startup band; both inputs active, either/both lost, independent freshness and chosen cross-channel policy; boot/reset/recovery of both physical commands. Separate interface-only tests from actual ignition-load acceptance.

Write `specs/07-ignition-channel/spec.md` using the installed Spec Kit workflow. Report blockers and pending physical evidence honestly. Stop after specification; Plan is a separate explicit invocation.
