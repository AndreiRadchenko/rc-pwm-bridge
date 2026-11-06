# Specify prompt — 06-pump-validation

Read and apply `speckit-iteration/current-baseline.md`, including every referenced current source, inherited contract and evidence boundary. It is required input for this prompt. The ratified constitution, current user instructions and decisions govern; do not reselect settled parameters or infer physical acceptance from completed software. Generate documentation in Ukrainian.

Target iteration `06-pump-validation`. Requirement traceability: R01,R04,R05,R06,R07,R08,R09,R10,R11,R12. Prerequisites: 01-foundation,02-control-core,03-rc-capture,04-pump-output,05-failsafe-arming. Questions to resolve or explicitly scope: Q03,Q06 and all unresolved v1 acceptance parameters.
Set `SPECIFY_FEATURE_DIRECTORY=specs/06-pump-validation` explicitly for every workflow script in this phase; confirm returned paths belong to this iteration. The prompt directory is input, not generated feature artifacts. Preserve existing artifacts and user changes. During Specify, create/switch to `06-pump-validation` under AGENTS.md/D10 while preserving user work; record D10 as the Specify-only branch rule now codified in constitution v2.0.0 principle V (D74). Do not commit or push. Run only this requested phase; do not invoke other phases, access/flash hardware or operate loads.

## Intended result

Validate the accepted single-channel firmware, actual MD20A and pump in a staged controlled bench process, with honest separation of software, build and physical evidence.

Use the accepted contracts and evidence from 01–05, including the Q05 policy selected in 05; this iteration must not choose new arming/cutoff defaults. Identify and close all applicable remaining H02/H03/Q03/Q06 physical gates before load operation. D48 permits HJ-based capture acceptance, but the actual receiver must be characterized and accepted before its use, including valid-looking PWM after radio loss. Keep the permanent MD20A DIR–GND jumper; verify actual motor polarity/direction here without adding firmware reversal. Manufacturer ratings and reported normal current do not establish startup/stall or thermal limits.

First verify no-load signals and power/protection, then progress through a suitable dummy load and controlled pump tests. Check actual receiver failsafe including valid-looking pulses after radio loss. Document supply sag, startup, transients, noise, current and thermal behavior at representative 40/80 percent and expected normal load.

## Acceptance and boundaries

Define user stories, scope/non-goals, observable acceptance criteria, failure/recovery scenarios, dependencies and requirement-ID traceability. Resolve product choices with the user; do not convert historical suggestions into approved defaults. Unrelated future questions must not block this iteration.

Cover these acceptance scenarios: Repeat host suite and target build, then staged physical scenarios: startup/reset/upload/brownout, loss/invalid/recovery, emergency cutoff, pump direction, endpoint action, representative thermal endurance and receiver/EMI behavior. Do not infer motor current from duty or treat brake as proven zero fuel flow. Mark unavailable scenarios pending.

Write `specs/06-pump-validation/spec.md` using the installed Spec Kit workflow. Report blockers and pending physical evidence honestly. Stop after specification; Plan is a separate explicit invocation.


Inherit D63-D65 and the accepted 05 implementation: explicit build-selected Test/Fly, default Test, transparent current-boot valid commands and automatic reconnect-high recovery without a zero handshake. Test cutoff and Fly long input-loss hold need distinct acceptance records, including no-prior-valid/reset and held endpoint cases. Do not let historical capture latches or generic fail-off assumptions silently replace Fly. Constitution v2.0.0/D74 already formalizes D63-D65 and closes G05; verify current compliance and do not rerun that amendment or treat it as implementation/physical acceptance. ArduPilot/Matek output behavior and levels require actual source characterization; receiver/radio-link tests apply before that receiver is used, while converter PWM freshness alone does not establish radio-link health.
