# Shared input context for iterations 04–09

Read this file with the requested iteration prompt. It is an English workflow input,
not a generated feature artifact or a new decision. Re-read current source documents
at invocation time; this summary does not freeze their future state.

## Sources and workflow

Read `docs/decisions.md`, `docs/requirements.md`, `docs/architecture.md`,
`docs/roadmap.md` and `docs/open-questions.md` first, then
`.specify/memory/constitution.md`, `AGENTS.md` and `hardware/README.md`.
The constitution is ratified v1.0.0, not a template. `PROJECT_FOUNDATION.md` is
historical context. Preserve installed skills and templates; invoke only the
requested phase. Write generated documentation and operator procedures in Ukrainian.

Use current decisions and prerequisite specifications/contracts for behavior, and
`specs/01-foundation/verification/acceptance.md`,
`specs/02-control-core/verification/acceptance.md` and
`specs/03-rc-capture/verification/acceptance.md` for evidence. Some architecture/roadmap
status prose predates implementation: do not restart completed software or infer
physical acceptance from that prose. Software exists for 01–03; 02 meets its own
software criteria under D38, while physical acceptance of 01/03 remains open.
Partial operator logs are observations, not replacements for the acceptance suite.
Recheck actual evidence and affected gates before dependent work; document pending
physical checks without inventing permission to bypass them. Specification/planning
may identify those gates without claiming that implementation or hardware use is ready.

During Specify, create or switch to the iteration's exact feature branch as required
by current `AGENTS.md` and D10, preserving existing files and user changes. D10 is
the explicit user-authorized exception to the older blanket branch prohibition in
constitution principle V; it does not authorize commit/push or hardware actions.
Record this reconciliation in generated workflow/compliance notes; do not amend the
constitution as a side effect. During other phases do not switch branches automatically.
Always set `SPECIFY_FEATURE_DIRECTORY=specs/<iteration>` explicitly, even when the
branch matches. If safe branch preparation is blocked, report it; do not discard,
stash, commit or overwrite user work to force progress. No prompt authorizes flashing,
device access, load operation, or another Spec Kit phase.

## Inherited design and contracts

- V1 is one local pump input on ESP32-C3 SuperMini and Cytron MD20A, using native
  ESP-IDF with radios/network services disabled. No Arduino framework or ignition in
  04–06. 07–09 remain separately requested extensions.
- Reuse `tools/environment/environment.lock.json` and the foundation build/test
  workflow (D14/D28 and closed Q01). ESP-IDF is pinned to v5.5.6; the portable core
  uses strict C++11 and app/platform C++17. Validate the recorded environment before
  reuse; do not reselect dependencies or add a second build system for v1.
- Reuse profile `c3-supermini-pinout-01` and unit `c3-supermini-188b0e93032c`:
  RC=GPIO3, PWM=GPIO0, external nominal 3.3 V at 3V3/GND. DIR is permanently
  jumpered to GND at MD20A, with no MCU DIR pin/control (D18–D20). Unknown factory
  revision is handled by D13's identified-variant policy, not guessed data.
  Power quality, PWM bias/startup and simultaneous USB/external-power suitability
  are not established by the pin assignment. Verify MD20A revision/interface/jumper
  in 04 without the pump; verify actual pump direction and power path in 06.
- Read `specs/02-control-core/contracts/control-core.md`. Width validity is
  900–2100 us inclusive. Validate before mapping/clamping: 1000/1250/1400/1500/1800/2000 us
  maps to 0/25/40/50/80/100 percent, nearest whole percent with half up, no filtering
  or calibration (D36). Invalid width retains the previous valid calculation and
  last-valid time. If none exists, no valid calculation is invented. Disarmed/fault
  overrides retain effective zero; 02/03 never grant operational permission.
- Read `specs/03-rc-capture/contracts/rc-capture.md` and its spec/plan. Only 50 Hz is
  supported in 03; period validity is 18000–22000 us inclusive between consecutive
  rising edges. Unknown/invalid period is not valid, does not refresh last-valid
  and alone does not latch a capture fault. Trustworthy invalid-period edges remain
  the next period origin (D41/D45/D49).
- RMT measures width independently; GPIO ISR supplies estimated rising/falling times
  in one monotonic microsecond domain. Last-valid uses the estimated falling time
  of an accepted valid pulse, never callback/worker/print time (D51). Do not restore
  superseded hardware-latch timestamps or the old 3 ms stuck-HIGH requirement.
  D56 disables internal GPIO3 pulls after each RMT channel creation, with checked
  cleanup/fault/retry on failure. Planned HJ conditioning is not measured Q03 acceptance.
- Explicit capture faults latch and force effective zero. Valid under latch may
  update calculation/last-valid, but cannot clear the latch or arm (D36/D37).
  Malformed/overflow data with unreliable boundaries is discarded; acquisition
  resynchronizes using two new trustworthy rising edges without clearing the fault.
  Preserve epoch/fault barriers, time-order handling and loss accounting from 03.
  Neither capture resync nor the model-only `clear_fault()` is production recovery.
- Capture stuck HIGH/LOW uses a 100 ms unchanged-level threshold from the last
  observed transition or observation start, plus a 10 ms fault-delivery budget.
  D52 also defines rearm/processing and active-RX guard budgets. These are capture
  requirements, not the operational freshness timeout or cutoff budget of Q05.
  Preserve them unless an explicit decision changes the relevant contract.
- Preserve D43/D52 diagnostics: one latest snapshot, at most one launch per 100 ms,
  bounded zero-wait/drop transport, counters/sticky faults and no control input.
  One already committed USB record may finish after reconnect; subsequent output
  is current, without pending history/burst. Do not revert to D22's foundation-only
  event policy. Diagnostic drops must not delay capture, control or cutoff.
  D55's bounded RAM trace/frozen export is bench-only, not production history.

## Decisions and evidence boundaries

Q04's width/rounding/period/capture policies are selected; physical compliance is
still pending. Q05 remains open for 05: operational timeout and inclusive boundary,
last-valid-to-LOW budget, valid-frame count, safe-low arming, recovery, watchdog and
any affected diagnostic integration. The existing 100 ms capture threshold and the
historical 100 ms operational proposal do not decide Q05. Preserve D36/D45 invalid
hold behavior; invalid activity never renews freshness. Any requested policy change
requires an explicit decision and synchronized affected artifacts before implementation.

Q02/H02/H03, Q03 and Q06 remain physical gates at their documented boundaries.
D38 permits software completion of 02 without physical acceptance of 01; D48 permits
completion of 03 with accepted HJ Tester capture, deferring a concrete receiver until
before its use. Neither decision accepts missing HJ/electrical/timing measurements.
Do not make receiver-specific tests a prerequisite for isolated HJ-only verification;
do not treat valid-looking receiver PWM as proof of radio-link health.

Before acceptance measurements, specify tolerances and instrument suitability.
Preserve 03's D53–D55 evidence scope: short bounded RAM accuracy windows with export
after capture, direct per-pulse E_width <=5 us / per-interval E_interval <=10 us,
separately documented U <=1 us, and separate 600 s zero-loss/timing tests under maximum
diagnostic load. No averaging or extra E+U margin; exact boundary/malformed fixtures
are host tests. Inherit these checks for affected regressions without inventing a
requirement to repeat unrelated hardware tests for document-only work.

PWM LOW at MD20A means brake, not coast, power isolation or proven zero flow. Hardware
bias, software LOW, watchdog and independent emergency power cutoff are distinct.
Do not copy a low-side MOSFET flyback arrangement to this H-bridge. Before accepted 05,
output tests are isolated without the pump; 06 requires accepted 05 and a verified
power bench. No missing Q02–Q06 evidence may be reported as hardware suitability.

Require meaningful tests for changed behavior, including boundaries and applicable
fault/recovery paths, without imposing test-first. Record requirement-to-acceptance
traceability and separate host tests, target builds, physical results and unperformed
checks under the owning feature. Every Plan must evaluate all six Constitution Check
gates before research and after design as pass/blocked/not applicable with references.
Do not fill blocking choices with generic defaults or a clarification-limit shortcut.
