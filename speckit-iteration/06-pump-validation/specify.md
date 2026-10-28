# Specify prompt — 06-pump-validation

Read `.specify/memory/constitution.md`, `docs/decisions.md`, `docs/requirements.md`, `docs/architecture.md`, `docs/roadmap.md`, `docs/open-questions.md` and `hardware/README.md`. Confirm the constitution has been filled and adopted; a stock placeholder is not an approved constitution. Treat `PROJECT_FOUNDATION.md` as historical context; current decisions supersede its WROOM/two-channel-start proposals.

Target iteration `06-pump-validation`. Requirement traceability: R01,R04,R06,R07,R08,R10,R11,R12. Prerequisites: 01-foundation,02-control-core,03-rc-capture,04-pump-output,05-failsafe-arming. Questions to resolve or explicitly scope: Q03,Q06 and all unresolved v1 acceptance parameters.
Set `SPECIFY_FEATURE_DIRECTORY=specs/06-pump-validation` explicitly for all workflow scripts, including setup-plan; confirm returned paths belong to this iteration. The prompt directory is input, not generated feature artifacts. Preserve existing artifacts and user changes. Do not create/switch Git branches or commit automatically. Do not run Tasks or Implement, flash hardware or operate loads.

## Intended result

Validate the accepted single-channel firmware, actual MD20A and pump in a staged controlled bench process, with honest separation of software, build and physical evidence.

First verify no-load signals and power/protection, then progress through a suitable dummy load and controlled pump tests. Check actual receiver failsafe including valid-looking pulses after radio loss. Document supply sag, startup, transients, noise, current and thermal behavior at representative 40/80 percent and expected normal load.

## Acceptance and boundaries

Define user stories, scope/non-goals, observable acceptance criteria, failure/recovery scenarios, dependencies and requirement-ID traceability. Resolve product choices with the user; do not convert historical suggestions into approved defaults. Unrelated future questions must not block this iteration.

Cover these acceptance scenarios: Repeat host suite and target build, then staged physical scenarios: startup/reset/upload/brownout, loss/invalid/recovery, emergency cutoff, pump direction, endpoint action, representative thermal endurance and receiver/EMI behavior. Do not infer motor current from duty or treat brake as proven zero fuel flow. Mark unavailable scenarios pending.

Write `specs/06-pump-validation/spec.md` using the installed Spec Kit workflow. Report blockers and pending physical evidence honestly. Stop after specification; Plan is a separate explicit invocation.
